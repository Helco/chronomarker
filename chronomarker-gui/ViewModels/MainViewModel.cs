using System.ComponentModel;

namespace Chronomarker.ViewModels;

internal class MainViewModel : ViewModelBase
{
#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider adding the 'required' modifier or declaring as nullable.
    [DesignOnly(true)] public MainViewModel() { }
#pragma warning restore CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider adding the 'required' modifier or declaring as nullable.

    public MainViewModel(StatusModel status, LogModel log, WatchModel watch)
    {
        Status = status;
        Log = log;
        Watch = watch;
    }

    public string Greeting => "Welcome to Avalonia!";
    public StatusModel Status { get; }
    public LogModel Log { get; }
    public WatchModel Watch { get; }
}
