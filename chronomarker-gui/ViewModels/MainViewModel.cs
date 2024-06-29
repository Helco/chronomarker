namespace Chronomarker.ViewModels;

internal class MainViewModel : ViewModelBase
{
    public string Greeting => "Welcome to Avalonia!";
    public StatusModel Status { get; } = new();
}
