using System;
using System.Threading;
using System.Threading.Tasks;
using Chronomarker.ViewModels;
using NetPebble.Transports;

namespace Chronomarker.Services;

internal sealed class PebbleDevConnectionService(
    LogService logService, IGameService gameService, StatusModel status, Uri address
    ) : PebbleService(logService, gameService, status)
{
    protected override ITransport CreateTransport() =>
        new WebsocketTransport(address);
}

internal sealed class PebbleBLClassicService(
    LogService logService, IGameService gameService, StatusModel status
    ) : PebbleService(logService, gameService, status)
{
    protected override ITransport CreateTransport() =>
        new BluetoothClassicTransport();
}

internal abstract class PebbleService : IWatchService
{
    private class Connection : IDisposable, InfrequentPacketScheduler.IAdapter
    {
        private static readonly Guid AppGuid = Guid.Parse("4a022ac1-39ae-4903-b538-ea5c035a0e81");

        private byte transactionId;
        private bool disposedValue;
        public bool CanBeUsed => !disposedValue && !Cancellation.IsCancellationRequested;
        public readonly CancellationTokenSource Cancellation = new();
        public readonly TaskCompletionSource Completion = new();
        public required NetPebble.Transports.ITransport Transport { get; init; }
        public DateTime LastHeartbeat { get; private set; } = DateTime.UtcNow;
        public TimeSpan TimeSinceHeartbeat => DateTime.UtcNow - LastHeartbeat;
        public DateTime MyLastHeartbeat { get; private set; } = DateTime.UtcNow;
        public TimeSpan TimeSinceMyHeartbeat => DateTime.UtcNow - MyLastHeartbeat;
        public static readonly TimeSpan MyHeartbeat = TimeSpan.FromSeconds(10);

        public TinyProtocol Protocol { get; }
        public InfrequentPacketScheduler Scheduler { get; }
        public bool HasPendingMessages => Protocol.HasPendingMessages;

        public Connection(LogService logService, StatusModel statusModel)
        {
            Scheduler = new(this, logService);
            Protocol = new(logService, Scheduler.QueuePacket, statusModel);
            Protocol.HandleWatchMessage([1, 0]);
            Task.Run(HeartbeatLoop);
        }

        private async Task HeartbeatLoop()
        {
            while (!Cancellation.IsCancellationRequested)
            {
                await Task.Delay(MyHeartbeat, Cancellation.Token);
                MyLastHeartbeat = DateTime.UtcNow;
                if (!Cancellation.IsCancellationRequested)
                {
                    if (!Protocol.FlushPendingMessages())
                        Protocol.Heartbeat();
                }
            }
        }

        public void MarkHeartbeat() => LastHeartbeat = DateTime.UtcNow;

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    Scheduler.Dispose();
                    Transport.Dispose();
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

        public void FlushPendingMessages() => Protocol.FlushPendingMessages();
        public void QueueEverything() => Protocol.QueueEverything();

        public Task SendPacket(byte[] packet, CancellationToken ct)
        {
            if (CanBeUsed)
            {
                var pebbleMessage = new NetPebble.Protocol.AppMessagePush(
                    unchecked(transactionId++),
                    AppGuid,
                    [new NetPebble.Protocol.AppMessageByteArray(16, packet)]);
                return Transport.Send(pebbleMessage, ct);
            }
            return Task.CompletedTask;
        }
    }

    private readonly LogService logService;
    private readonly StatusModel statusModel;
    private readonly Action<string> logMessage;
    private readonly object runLock = new();
    private TinyProtocol? prevProtocol = null; // to pass state across connections
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

    public PebbleService(LogService logService, IGameService gameService, StatusModel statusModel)
    {
        this.logService = logService;
        this.statusModel = statusModel;
        logMessage = logService.Log;
        gameService.OnMessage += HandleMessage;
        prevProtocol = new TinyProtocol(logService, _ => { }, statusModel);
    }

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
                currentConnection?.Dispose();
            }
            catch (Exception) { }
            currentConnection = null;
            cancellation = null;
            runTask = null;
            Status = WatchStatus.Initial;
        }
    }

    private void HandleMessage(IGameMessage message)
    {
        lock(runLock)
        {
            if (currentConnection?.CanBeUsed is true)
                currentConnection.Protocol.HandleGameMessage(message);
        }
    }

    private class ConnectionException : Exception
    {
        public ConnectionException() { }
        public ConnectionException(string? message) : base(message) { }
        public ConnectionException(string? message, Exception? innerException) : base(message, innerException) { }
    }

    protected abstract NetPebble.Transports.ITransport CreateTransport();

    private async Task<Connection> Connect()
    {
        try
        {
            var transport = CreateTransport();
            //var transport = new NetPebble.Transports.WebsocketTransport("ws://127.0.0.1:33327");
            //var transport = new NetPebble.Transports.WebsocketTransport("ws://192.168.178.89:9000");
            //var transport = new NetPebble.Transports.BluetoothClassicTransport();
            await transport.ConnectAsync(default);

            // Connection
            var connection = new Connection(logService, statusModel)
            {
                Transport = transport
            };
            prevProtocol = connection.Protocol;
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
        logMessage($"Disconnect {connection}: {reason}");
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

    private void HandleReceive(Connection connection, byte[] bytes)
    {
        if (!connection.CanBeUsed)
            return;
        connection.MarkHeartbeat();
        connection.Protocol.HandleWatchMessage(bytes);
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
                    //HandleDisconnect(connection, "Timeout");
                    //return;
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
                Status = WatchStatus.Connecting;
                logMessage("Connecting...");
                Connection? connection;
                try { connection = currentConnection = await Connect(); }
                catch (ConnectionException ex)
                {
                    logService.Log("Connection failed: " + ex.Message);
                    continue;
                }
                Status = WatchStatus.Connected;
                logMessage("Connected");
                token.Register(connection.Cancellation.Cancel);
                //_ = Task.Run(() => TaskTimeout(connection));
                await connection.Completion.Task.WaitAsync(token);
                HandleDisconnect(connection, "Unknown disconnection event");
            }
        }
        catch (OperationCanceledException) { }
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
