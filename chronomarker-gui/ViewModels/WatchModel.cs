using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Chronomarker.Services;
using ReactiveUI;

namespace Chronomarker.ViewModels;

internal class WatchModel : ViewModelBase
{
    private readonly ProxyWatchService proxyWatchService;

    public WatchModel(ProxyWatchService proxyWatchService)
    {
        this.proxyWatchService = proxyWatchService;
        proxyWatchService.OnStatusChanged += HandleStatusChanged;
    }

    public WatchType WatchType { get; private set; }
    public WatchStatus WatchStatus { get; private set; }

    private void HandleStatusChanged(WatchStatus status)
    {
        if (WatchStatus == status)
            return;
        WatchStatus = status;
        this.RaisePropertyChanged(nameof(WatchStatus));
    }
}
