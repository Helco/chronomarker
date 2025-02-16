﻿using System;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using NetPebble.Protocol;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Rfcomm;
using Windows.Devices.Enumeration;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;

namespace NetPebble.Transports;

public class BluetoothClassicTransport : ITransport
{
    private record class Connection(
        BluetoothDevice Device,
        StreamSocket Socket,
        DataWriter Writer);

    private bool disposedValue;
    private Connection? connection;

    protected virtual void Dispose(bool disposing)
    {
        if (!disposedValue)
        {
            if (disposing)
            {
                if (connection is not null)
                {
                    connection.Writer.Dispose();
                    connection.Socket.Dispose();
                    connection.Device.Dispose();
                    connection = null;
                }
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

    public async Task ConnectAsync(CancellationToken ct)
    {
        var tcs = new TaskCompletionSource<string>();
        var watcher = DeviceInformation.CreateWatcher(
            "(System.ItemNameDisplay:~<\"Pebble\")",
            [], DeviceInformationKind.DeviceInterface); // DeviceInterface is much faster to enumerate
        watcher.Added += (watcher, deviceInfo) => tcs.TrySetResult(deviceInfo.Id);
        watcher.EnumerationCompleted += (watcher, _) => tcs.TrySetException(new IOException("Could not find Pebble"));
        watcher.Start();
        var deviceId = await tcs.Task.WaitAsync(ct);
        watcher.Stop();

        var accessInfo = DeviceAccessInformation.CreateFromId(deviceId);
        if (accessInfo.CurrentStatus != DeviceAccessStatus.Allowed)
            throw new IOException("Application is not allowed to access watch: " + accessInfo.CurrentStatus);

        BluetoothDevice? device = null;
        StreamSocket? socket = null;
        try
        {
            device = await BluetoothDevice.FromIdAsync(deviceId).AsTask(ct);
            var services = await device.GetRfcommServicesForIdAsync(RfcommServiceId.SerialPort).AsTask(ct);
            if (services.Error != BluetoothError.Success || services.Services.Count == 0)
                throw new IOException("Could not get serial access: " + services.Error.ToString());
            var service = services.Services[0];

            socket = new StreamSocket();
            await socket.ConnectAsync(service.ConnectionHostName, service.ConnectionServiceName).AsTask(ct);
            connection = new(device, socket, new(socket.OutputStream));
        }
        catch
        {
            socket?.Dispose();
            device?.Dispose();
        }
    }

    public async Task Send(IPebblePacket packet, CancellationToken ct)
    {
        if (connection is null)
            return;

        using var memoryStream = new MemoryStream(128);
        using var writer = new BinaryWriter(memoryStream);
        packet.SerializeWithFrame(writer);
        writer.Flush();
        var message = memoryStream.ToArray();

        connection.Writer.WriteBytes(message);
        await connection.Writer.StoreAsync().AsTask(ct);
    }
}
