using Avalonia.Controls;
using Chronomarker.Services;

namespace Chronomarker.Views;

public partial class MainView : UserControl
{
    public MainView()
    {
        InitializeComponent();

        AppServiceProvider.GetRequiredService<IWatchService>
    }
}
