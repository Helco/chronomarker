namespace Chronomarker.ViewModels;

internal class MainViewModel : ViewModelBase
{
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
