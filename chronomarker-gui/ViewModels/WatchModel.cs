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
    private readonly LogService logService;
    private WatchType watchType = WatchType.PebbleDevConnection;

    public WatchType WatchType
    {
        get => watchType;
        set
        {
            var prevNeedsAddress = NeedsAddress;
            watchType = value;
            this.RaisePropertyChanged(nameof(WatchType));
            this.RaisePropertyChanged(nameof(DefaultAddress));
            this.RaisePropertyChanged(nameof(NeedsAddress));
            if (prevNeedsAddress != NeedsAddress)
            {
                Address = "";
                this.RaisePropertyChanged(nameof(Address));
            }
        }
    }
    public WatchStatus WatchStatus { get; private set; }
    public string DefaultAddress => WatchType switch
    {
        WatchType.LPV6 => "<none> (also LPV6 is not supported, how did you get here?)",
        WatchType.PebbleDevConnection => "Pebble App->Settings->Developer Connection->Server IP",
        WatchType.PebbleBLClassic => "Disconnect the Pebble from the phone and pair with PC",
        WatchType.DebugTinyProtocol => "Nothing, this is just for development",
        _ => "How did you get here?"
    };
    public bool NeedsAddress => WatchType is WatchType.PebbleDevConnection && !IsRunning;
    public string Address { get; set; } = "";
    public string StartStopText => IsRunning ? "Stop" : "Start";
    public bool IsRunning => proxyWatchService?.IsRunning is true;
    public bool IsNotRunning => !IsRunning;
    //public ReactiveCommand<Unit, Unit>? StartStop { get; }

    [DesignOnly(true)] public WatchModel() => proxyWatchService = null!;
    public WatchModel(ProxyWatchService proxyWatchService, LogService logService)
    {
        this.proxyWatchService = proxyWatchService;
        this.logService = logService;
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
            if (CheckAddress() is not Uri uri)
                return;
            proxyWatchService.SetWatchType(WatchType, uri);
            proxyWatchService.Start();
        }
        RaiseIsRunningChange();
    }

    private void RaiseIsRunningChange()
    {
        this.RaisePropertyChanged(nameof(StartStopText));
        this.RaisePropertyChanged(nameof(IsRunning));
        this.RaisePropertyChanged(nameof(IsNotRunning));
        this.RaisePropertyChanged(nameof(NeedsAddress));
    }

    private Uri? CheckAddress()
    {
        if (WatchType is not WatchType.PebbleDevConnection)
            return new Uri("ws://127.0.0.1:9000");

        var address = Address.Trim();
        if (!address.StartsWith("ws://"))
            address = "ws://" + address;
        if (!Uri.TryCreate(address, UriKind.Absolute, out var uri))
        {
            logService.Log("Unable to parse address");
            return null;
        }
        if (uri.Scheme != "ws")
        {
            logService.Log("Unable to parse address - expected scheme to be \"ws\"");
            return null;
        }
        if (uri.PathAndQuery != "" && uri.PathAndQuery != "/")
        {
            logService.Log("Unable to parse address - unexpected path");
            return null;
        }
        if (uri.IsDefaultPort)
        {
            // I guess the user did not give a port so we take the default one
            var b = new UriBuilder(uri);
            b.Port = 9000;
            uri = b.Uri;
        }
        return uri;
    }
}
