using System;
using System.Diagnostics;
using Avalonia.Controls;

namespace Chronomarker.Views;

public partial class MockGameWindow : Window
{
    public MockGameWindow()
    {
        InitializeComponent();
    }

    protected override void OnClosed(EventArgs e)
    {
        base.OnClosed(e);
        Environment.Exit(0);
    }
}
