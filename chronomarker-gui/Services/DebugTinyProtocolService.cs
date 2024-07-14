using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chronomarker.Services;

internal class DebugTinyProtocolService : IWatchService
{
    private readonly TinyProtocol protocol;
    private readonly LogService logService;
    private StreamWriter? writer;
    private WatchStatus status;
    private bool disposedValue;

    public bool IsRunning { get; private set; }
    public WatchStatus Status
    {
        get => status;
        set
        {
            if (status == value)
                return;
            status = value;
            OnStatusChanged?.Invoke(status);
        }
    }

    public event Action<WatchStatus>? OnStatusChanged;

    public DebugTinyProtocolService(LogService logService, IGameService gameService)
    {
        protocol = new(logService, QueuePacket);
        this.logService = logService;
        gameService.OnMessage += protocol.HandleGameMessage;
    }

    public void Start()
    {
        Stop();
        writer = new StreamWriter("tiny_messages.log");
        Status = WatchStatus.Connected;
        protocol.HandleWatchMessage(new byte[] { 1, 0 });
    }

    public void Stop()
    {
        Status = WatchStatus.Disconnected;
        writer?.Dispose();
        writer = null;
    }

    private void QueuePacket(ReadOnlySpan<byte> message)
    {
        var messageStr = Convert.ToHexString(message);
        writer?.WriteLine(messageStr);
        logService.Log("Tiny: " + messageStr);
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!disposedValue)
        {
            if (disposing)
            {
                Stop();
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
