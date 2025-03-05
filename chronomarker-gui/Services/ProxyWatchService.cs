using System;
using Microsoft.Extensions.DependencyInjection;

namespace Chronomarker.Services;

internal class ProxyWatchService : IWatchService
{
    private readonly IServiceProvider services;

    private IWatchService impl = NullWatchService.Instance;
    private WatchType? watchType = null;
    private Uri address = new Uri("ws://127.0.0.1:9000"); // dummy

    public WatchStatus Status => impl.Status;
    public bool IsRunning => impl.IsRunning;
    public event Action<WatchStatus>? OnStatusChanged;
    public void Start() => impl.Start();
    public void Stop() => impl.Stop();

    public ProxyWatchService(IServiceProvider services)
    {
        this.services = services;
        SetWatchType(WatchType.PebbleBLClassic, address);
    }

    private void HandleStatusChanged(WatchStatus status) {
        lock (this) { OnStatusChanged?.Invoke(status); }
    }

    public void SetWatchType(WatchType newType, Uri address)
    {
        lock (this)
        {
            if (watchType == newType && this.address == address)
                return;
            this.address = address;
            impl.OnStatusChanged -= HandleStatusChanged;
            var prevStatus = impl.Status;
            impl.Dispose();
            switch(newType)
            {
                case WatchType.LPV6: impl = ActivatorUtilities.CreateInstance<LPV6Service>(services); break;
                case WatchType.PebbleDevConnection: impl = ActivatorUtilities.CreateInstance<PebbleDevConnectionService>(services, address); break;
                case WatchType.PebbleBLClassic: impl = ActivatorUtilities.CreateInstance<PebbleBLClassicService>(services); break;
                case WatchType.DebugTinyProtocol: impl = ActivatorUtilities.CreateInstance<DebugTinyProtocolService>(services); break;
                default: throw new NotImplementedException($"Did not implement watch instantiation for {newType}");
            }
            watchType = newType;
            if (impl.Status != prevStatus)
                HandleStatusChanged(impl.Status);
            impl.OnStatusChanged += HandleStatusChanged;
        }
    }

    public void ResetWatchService()
    {
        lock (this)
        {
            var prevStatus = impl.Status;
            impl.OnStatusChanged -= HandleStatusChanged;
            impl.Dispose();
            impl = NullWatchService.Instance;
            if (impl.Status != prevStatus)
                HandleStatusChanged(impl.Status);
        }
    }

    public void Dispose() => ResetWatchService();
}
