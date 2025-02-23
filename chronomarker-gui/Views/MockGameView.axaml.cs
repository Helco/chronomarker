using Avalonia.Controls;
using Chronomarker.Services;

namespace Chronomarker.Views;

public partial class MockGameView : UserControl
{
    public MockGameView()
    {
        InitializeComponent();
        if (Design.IsDesignMode)
            return;
    }
}
