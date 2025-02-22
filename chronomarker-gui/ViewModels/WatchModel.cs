using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Reactive;
using System.Text;
using System.Threading.Tasks;
using Avalonia.Media;
using Chronomarker.Services;
using ReactiveUI;

namespace Chronomarker.ViewModels;

internal class WatchModel : ViewModelBase
{
    private readonly ProxyWatchService proxyWatchService;

    public WatchType WatchType { get; set; }
    public WatchStatus WatchStatus { get; private set; }
    public string StartStopText => IsRunning ? "Stop" : "Start";
    public bool IsRunning => proxyWatchService?.IsRunning is true;
    public bool IsNotRunning => !IsRunning;
    //public ReactiveCommand<Unit, Unit>? StartStop { get; }

    [DesignOnly(true)] public WatchModel() => proxyWatchService = null!;
    public WatchModel(ProxyWatchService proxyWatchService)
    {
        this.proxyWatchService = proxyWatchService;
        proxyWatchService.OnStatusChanged += HandleStatusChanged;
        /*StartStop = ReactiveCommand.Create(() =>
        {
            if (proxyWatchService.IsRunning)
                proxyWatchService.Stop();
            else
                proxyWatchService.Start();
            this.RaisePropertyChanged(nameof(StartStopText));
        });*/
    }

    private void HandleStatusChanged(WatchStatus status)
    {
        if (WatchStatus == status)
            return;
        WatchStatus = status;
        this.RaisePropertyChanged(nameof(WatchStatus));
        RaiseIsRunningChange();
    }

    public void StartStop()
    {
        if (proxyWatchService.IsRunning)
            proxyWatchService.Stop();
        else
        {
            proxyWatchService.SetWatchType(WatchType);
            proxyWatchService.Start();
        }
        RaiseIsRunningChange();
    }

    private void RaiseIsRunningChange()
    {
        this.RaisePropertyChanged(nameof(StartStopText));
        this.RaisePropertyChanged(nameof(IsRunning));
        this.RaisePropertyChanged(nameof(IsNotRunning));
    }
}
