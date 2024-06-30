using System;
using Microsoft.Extensions.DependencyInjection;

namespace Chronomarker.Services;

internal class ProxyWatchService : IWatchService
{
    private readonly IServiceProvider services;

    private IWatchService impl = NullWatchService.Instance;

    public WatchStatus Status => impl.Status;
    public bool IsRunning => impl.IsRunning;
    public event Action<WatchStatus>? OnStatusChanged;
    public void Start() => impl.Start();
    public void Stop() => impl.Stop();

    public ProxyWatchService(IServiceProvider services)
    {
        this.services = services;
        SetWatchType(WatchType.LPV6);
    }

    private void HandleStatusChanged(WatchStatus status) {
        lock (this) { OnStatusChanged?.Invoke(status); }
    }

    public void SetWatchType(WatchType newType)
    {
        lock (this)
        {
            var prevStatus = impl.Status;
            impl.Dispose();
            switch(newType)
            {
                case WatchType.LPV6: impl = ActivatorUtilities.CreateInstance<LPV6Service>(services); break;
                default: throw new NotImplementedException($"Did not implement watch instanciation for {newType}");
            }
            if (impl.Status != impl.Status)
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
