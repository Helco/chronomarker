using System;
using System.IO;
using System.Net.WebSockets;
using System.Threading;
using System.Threading.Tasks;
using NetPebble.Protocol;

namespace NetPebble.Transports;

public class WebsocketTransport(Uri address) : ITransport
{
    private readonly ClientWebSocket webSocket = new();
    private readonly Uri address = address;
    private bool disposedValue;

    public WebsocketTransport(string address) : this(new Uri(address)) { }
    
    public Task ConnectAsync(CancellationToken ct)
    {
        return webSocket.ConnectAsync(address, ct);
    }

    public Task Send(IPebblePacket packet, CancellationToken ct)
    {
        using var memoryStream = new MemoryStream(128);
        using var writer = new BinaryWriter(memoryStream);
        writer.Write((byte)0x01); // Relay to watch
        packet.SerializeWithFrame(writer);
        writer.Flush();
        var message = memoryStream.ToArray();

        return webSocket.SendAsync(message, WebSocketMessageType.Binary, endOfMessage: true, ct);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!disposedValue)
        {
            if (disposing)
            {
                webSocket.Dispose();
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
