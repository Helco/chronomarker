using System;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;

namespace Chronomarker.Services;

internal class LPV6Service : IWatchService
{
    private class Connection : IDisposable
    {
        private bool disposedValue;
        public bool CanBeUsed => !disposedValue && !Cancellation.IsCancellationRequested;
        public readonly CancellationTokenSource Cancellation = new();
        public readonly TaskCompletionSource Completion = new();
        public required ulong Address { get; init; }
        public required BluetoothLEDevice Device { get; init; }
        public required GattDeviceService Service { get; init; }
        public required GattCharacteristic Characteristic { get; init; }
        public DateTime LastHeartbeat { get; private set; } = DateTime.UtcNow;
        public TimeSpan TimeSinceHeartbeat => DateTime.UtcNow - LastHeartbeat;

        public void MarkHeartbeat() => LastHeartbeat = DateTime.UtcNow;

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    Service.Dispose();
                    Device.Dispose();
                }
                disposedValue = true;
            }
        }

        public void Dispose()
        {
            // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }

    private readonly Action<string> logMessage;
    private readonly object runLock = new();
    private Connection? currentConnection = null;
    private CancellationTokenSource? cancellation;
    private Task? runTask;
    private bool disposedValue;

    private WatchStatus _status;
    public WatchStatus Status
    {
        get => _status;
        set
        {
            if (_status == value)
                return;
            _status = value;
            OnStatusChanged?.Invoke(value);
        }
    }
    public bool IsRunning => cancellation != null;
    public event Action<WatchStatus>? OnStatusChanged;

    public bool PrintWatchLog { get; set; } // non-functional

    public LPV6Service(LogService logService) => logMessage = logService.Log;

    public void Start()
    {
        lock (runLock)
        {
            Stop();
            Status = WatchStatus.Initial;
            cancellation = new();
            runTask = Task.Run(RunLoop);
        }
    }

    public void Stop()
    {
        lock (runLock)
        {
            try
            {
                cancellation?.Cancel();
                runTask?.Wait(TimeSpan.FromSeconds(3));
                cancellation?.Dispose();
            }
            catch (Exception) { }
            cancellation = null;
            runTask = null;
            Status = WatchStatus.Initial;
        }
    }

    private async Task<ulong> WatchForDevice(CancellationToken cancel)
    {
        var adapter = await BluetoothAdapter.GetDefaultAsync();
        cancel.ThrowIfCancellationRequested();

        // Toggle radio to hopefully disconnect all devices and reset everything
        var radio = await adapter.GetRadioAsync();
        await radio.SetStateAsync(Windows.Devices.Radios.RadioState.Off);
        await Task.Delay(300);
        await radio.SetStateAsync(Windows.Devices.Radios.RadioState.On);
        cancel.ThrowIfCancellationRequested();

        var completion = new TaskCompletionSource<ulong>();
        var filterAdvertisement = new BluetoothLEAdvertisement();
        filterAdvertisement.ManufacturerData.Add(new(0x7201, new byte[0].AsBuffer()));
        var watcher = new BluetoothLEAdvertisementWatcher(new()
        {
            Advertisement = filterAdvertisement
        });
        watcher.Received += (_, adv) =>
        {
            var versionData = adv.Advertisement.ManufacturerData.FirstOrDefault(m => m.CompanyId == 0x7201);
            if (versionData == null || versionData.Data.Length < 1u)
                return;
            var version = versionData.Data.GetByte(0);
            logMessage($"Found {adv.BluetoothAddress:X8} with version {version}");
            if (version != 1)
                return;

            if (watcher.Status != BluetoothLEAdvertisementWatcherStatus.Started)
                return;
            watcher.Stop();
            completion.SetResult(adv.BluetoothAddress);
        };
        watcher.Start();
        return await completion.Task.WaitAsync(cancel);
    }

    private class ConnectionException : Exception
    {
        public ConnectionException() { }
        public ConnectionException(string? message) : base(message) { }
        public ConnectionException(string? message, Exception? innerException) : base(message, innerException) { }
    }

    private async Task<Connection> ConnectTo(ulong address)
    {
        try
        {
            // Device
            var device = await BluetoothLEDevice.FromBluetoothAddressAsync(address);
            if (device == null)
                throw new ConnectionException($"Unknown device {address:X8}");
            var access = await device.RequestAccessAsync();
            if (access != Windows.Devices.Enumeration.DeviceAccessStatus.Allowed)
                throw new ConnectionException($"RequestAccess failed with {access}");

            // Service
            var services = await device.GetGattServicesForUuidAsync(BluetoothUuidHelper.FromShortId(0x7201));
            if (services.Status != GattCommunicationStatus.Success)
                throw new ConnectionException($"GetServices failed with {services.Status} ({services.ProtocolError})");
            if (services.Services.Count < 1)
                throw new ConnectionException("Service could not be retrieved");
            var service = services.Services.First();
            var openStatus = await service.OpenAsync(GattSharingMode.SharedReadAndWrite);
            if (openStatus != GattOpenStatus.Success)
                throw new ConnectionException($"Service could not be opened: {openStatus}");

            // Characteristic
            var characteristics = await service.GetCharacteristicsForUuidAsync(BluetoothUuidHelper.FromShortId(0xC520));
            if (characteristics.Status != GattCommunicationStatus.Success)
                throw new ConnectionException($"GetCharacteristics failed with {characteristics.Status} ({characteristics.ProtocolError})");
            if (characteristics.Characteristics.Count < 1)
                throw new ConnectionException("Characteristic could not be retrieved");
            var characteristic = characteristics.Characteristics.First();
            var notifyStatus = await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                GattClientCharacteristicConfigurationDescriptorValue.Notify);
            if (notifyStatus != GattCommunicationStatus.Success)
                throw new ConnectionException("Characteristic notification could not be set");

            // Connection
            var connection = new Connection()
            {
                Address = address,
                Device = device,
                Service = service,
                Characteristic = characteristic
            };
            device.ConnectionStatusChanged += (_, _) => HandleDisconnect(connection, "Device was closed");
            service.Session.SessionStatusChanged += (_, _) => HandleDisconnect(connection, "Service was closed");
            characteristic.ValueChanged += (_, ev) => HandleReceive(connection, ev);
            return connection;
        }
        catch (Exception ex) when (ex is not ConnectionException)
        {
            throw new ConnectionException(ex.Message, ex);
        }
    }

    private void HandleDisconnect(Connection connection, string reason)
    {
        if (!connection.CanBeUsed)
            return;
        connection.Cancellation.Cancel();
        logMessage($"Disconnect {connection.Address:X8}: {reason}");
        Status = WatchStatus.Disconnected;
        Task.Run(() =>
        {
            try
            {
                if (!connection.Completion.Task.Wait(TimeSpan.FromSeconds(3)))
                    throw new TimeoutException("Connection tasks did not finish in time");
            }
            catch (Exception ex)
            {
                logMessage($"{ex.GetType().Name} during disconnect: " + ex.Message);
            }
            connection.Completion.TrySetResult();
            try
            {
                connection.Dispose();
            }
            catch (Exception ex)
            {
                logMessage($"{ex.GetType().Name} during disconnect disposal: " + ex.Message);
            }
        });
    }

    private void HandleReceive(Connection connection, GattValueChangedEventArgs ev)
    {
        if (!connection.CanBeUsed)
            return;
        connection.MarkHeartbeat();
    }

    private TimeSpan Timeout = TimeSpan.FromSeconds(5);
    private async Task TaskTimeout(Connection connection)
    {
        try
        {
            while (connection.CanBeUsed)
            {
                await Task.Delay(Timeout - connection.TimeSinceHeartbeat, connection.Cancellation.Token);
                if (connection.TimeSinceHeartbeat >= Timeout)
                {
                    HandleDisconnect(connection, "Timeout");
                    return;
                }
            }
        }
        catch (OperationCanceledException) { }
        finally
        {
            connection.Completion.TrySetResult();
        }
    }

    private async Task RunLoop()
    {
        try
        {
            var token = cancellation!.Token;
            while (!token.IsCancellationRequested)
            {
                Status = WatchStatus.Watching;
                logMessage("Start watching for LPV6");
                var address = await WatchForDevice(token);
                Status = WatchStatus.Connecting;
                logMessage("Found watch, connecting...");
                var connection = currentConnection = await ConnectTo(address);
                Status = WatchStatus.Connected;
                logMessage("Connected");
                token.Register(connection.Cancellation.Cancel);
                _ = Task.Run(() => TaskTimeout(connection));
                await connection.Completion.Task;
                HandleDisconnect(connection, "Unknown disconnection event");
            }
        }
        catch (OperationCanceledException) { }
    }

    public async Task SendMessage(byte[] message)
    {
        var connection = currentConnection;
        if (connection?.CanBeUsed is true)
            await connection.Characteristic.WriteValueAsync(message.AsBuffer(), GattWriteOption.WriteWithoutResponse);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (disposedValue || !disposing)
            return;
        disposedValue = true;
    }

    public void Dispose()
    {
        Dispose(disposing: true);
        GC.SuppressFinalize(this);
    }
}
