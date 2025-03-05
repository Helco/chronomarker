using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chronomarker.Services;

internal enum WatchStatus
{
    Initial,
    Watching,
    Connecting,
    Connected,
    Disconnected
}

internal enum WatchType
{
    LPV6,
    PebbleDevConnection,
    PebbleBLClassic,

#if DEBUG
    DebugTinyProtocol
#endif
}

internal interface IWatchService : IDisposable
{
    WatchStatus Status { get; }
    bool IsRunning { get; }
    event Action<WatchStatus>? OnStatusChanged;
    void Start();
    void Stop();
}

internal class NullWatchService : IWatchService
{
    public WatchStatus Status => WatchStatus.Disconnected;
    public bool IsRunning => false;
    event Action<WatchStatus>? IWatchService.OnStatusChanged { add { } remove { } }
    public void Dispose() { }
    public void Start() { }
    public void Stop() { }

    public static readonly NullWatchService Instance = new();
}
