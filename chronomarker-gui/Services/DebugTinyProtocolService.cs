using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chronomarker.Services;

internal class DebugTinyProtocolService : IWatchService, InfrequentPacketScheduler.IAdapter
{
    private readonly LogService logService;
    private readonly IGameService gameService;
    private readonly Stopwatch stopwatch = new();
    private TimeSpan lastSecond = TimeSpan.Zero;
    private int packetCount = 0;
    private TinyProtocol? protocol;
    private InfrequentPacketScheduler? scheduler;
    private StreamWriter? writer;
    private WatchStatus status;
    private bool disposedValue;

    public bool IsRunning => Status is WatchStatus.Connected;
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

    bool InfrequentPacketScheduler.IAdapter.HasPendingMessages => protocol?.HasPendingMessages ?? false;

    public event Action<WatchStatus>? OnStatusChanged;

    public DebugTinyProtocolService(LogService logService, IGameService gameService)
    {
        this.logService = logService;
        this.gameService = gameService;
        stopwatch.Start();
    }

    public void Start()
    {
        Stop();
        writer = new StreamWriter("tiny_messages.log");
        Status = WatchStatus.Connected;
        scheduler = new(this, logService);
        protocol = new(logService, scheduler.QueuePacket);
        protocol.HandleWatchMessage([1, 0]);
        gameService.OnMessage += protocol.HandleGameMessage;
    }

    public void Stop()
    {
        Status = WatchStatus.Disconnected;
        if (scheduler != null)
        {
            scheduler.Dispose();
            scheduler = null;
        }
        if (protocol != null)
        {
            gameService.OnMessage -= protocol.HandleGameMessage;
            protocol = null;
        }
        writer?.Dispose();
        writer = null;
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

    void InfrequentPacketScheduler.IAdapter.QueueEverything() => protocol?.QueueEverything();
    void InfrequentPacketScheduler.IAdapter.FlushPendingMessages() => protocol?.FlushPendingMessages();

    Task InfrequentPacketScheduler.IAdapter.SendPacket(byte[] packet)
    {
        packetCount++;
        var sinceLastSecond = stopwatch.Elapsed - lastSecond;
        if (sinceLastSecond > TimeSpan.FromSeconds(1))
        {
            logService.Log($"Packets: {packetCount / sinceLastSecond.TotalSeconds:0.##}/sec");
            packetCount = 0;
            lastSecond = stopwatch.Elapsed;
        }

        //logService.Log("Tiny: " + Convert.ToHexString(packet));
        writer?.WriteLine(Convert.ToHexString(packet));
        return writer?.FlushAsync() ?? Task.CompletedTask;
    }
}
