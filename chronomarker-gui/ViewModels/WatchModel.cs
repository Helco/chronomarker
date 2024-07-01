using System;
using System.Collections.Generic;
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

    public WatchType WatchType { get; private set; }
    public WatchStatus WatchStatus { get; private set; }
    public string StartStopText => proxyWatchService?.IsRunning is true ? "Stop" : "Start";
    //public ReactiveCommand<Unit, Unit>? StartStop { get; }

    private WatchModel() => proxyWatchService = null!;
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
    }

    public void StartStop()
    {
        if (proxyWatchService.IsRunning)
            proxyWatchService.Stop();
        else
            proxyWatchService.Start();
        this.RaisePropertyChanged(nameof(StartStopText));
    }
}
