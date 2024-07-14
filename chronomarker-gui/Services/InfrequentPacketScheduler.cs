using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Reactive.Concurrency;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Chronomarker.Services;

internal class InfrequentPacketScheduler : IDisposable
{
    internal interface IAdapter
    {
        bool HasPendingMessages { get; }
        void FlushPendingMessages();
        void QueueEverything();
        Task SendPacket(byte[] packet);
    }

    private static readonly TimeSpan MinDelay = TimeSpan.FromSeconds(0.06);
    private static readonly TimeSpan MaxDelay = TimeSpan.FromSeconds(0.33);
    private const int ResetQueuedPackets = 5;
    private const int FastBurstPackets = 3;

    private readonly SemaphoreSlim semaphore = new(0, 1);
    private readonly CancellationTokenSource cancellation = new();
    private readonly Queue<byte[]> queuedPackets = new(ResetQueuedPackets);
    private readonly Stopwatch stopwatch = new();
    private readonly LogService logService;
    private readonly IAdapter adapter;
    private readonly Task sendLoopTask;
    private TimeSpan lastMessage = TimeSpan.Zero;
    private bool disposedValue;

    public InfrequentPacketScheduler(IAdapter adapter, LogService logService)
    {
        this.logService = logService;
        this.adapter = adapter;
        stopwatch.Start();
        sendLoopTask = Task.Run(SendLoop);
        semaphore.Release();
    }

    public void QueuePacket(ReadOnlySpan<byte> packet)
    {
        if (cancellation.IsCancellationRequested)
            return;

        var packetData = packet.ToArray(); // we need it as array in any case
        bool iDidAlreadyRelease = false;
        semaphore.Wait();
        try
        {
            if (queuedPackets.Count >= ResetQueuedPackets)
            {
                // Oh no... reset everything and LETS GO! (but exit-early as this gets recursive)
                logService.Log("Oh no, messages queued up. Resetting outbox...");
                queuedPackets.Clear();
                semaphore.Release();
                iDidAlreadyRelease = true;
                adapter.QueueEverything();
                return;
            }

            queuedPackets.Enqueue(packetData);
        }
        finally
        {
            if (!iDidAlreadyRelease)
                semaphore.Release();
        }
    }

    private async Task SendLoop()
    {
        bool isFastBurst = false;
        while (!cancellation.IsCancellationRequested)
        {
            var curDelay = stopwatch.Elapsed - lastMessage;
            if (!isFastBurst && curDelay < MinDelay)
                await Task.Delay(MinDelay - curDelay);
            if (curDelay > MaxDelay)
                adapter.FlushPendingMessages();
            await semaphore.WaitAsync();
            try
            {
                if (queuedPackets.Count >= FastBurstPackets)
                {
                    isFastBurst = true;
                    logService.Log("Bursting messages to avoid queuing up");
                }
                if (queuedPackets.Any())
                {
                    lastMessage = stopwatch.Elapsed;
                    await adapter.SendPacket(queuedPackets.Dequeue());
                }
                else if (isFastBurst)
                {
                    isFastBurst = false;
                    logService.Log("Burst is over");
                }
            }
            catch(Exception e)
            {
                logService.Log("Exception in SendLoop: " + e);
            }
            finally
            {
                semaphore.Release();
            }
        }
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!disposedValue)
        {
            if (disposing)
            {
                cancellation.Cancel();
                semaphore.Wait();
                sendLoopTask.Wait(TimeSpan.FromSeconds(2.0));
                semaphore.Release();
                semaphore.Dispose();
                cancellation.Dispose();
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
