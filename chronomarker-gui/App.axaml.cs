using System;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using Chronomarker.Services;
using Chronomarker.ViewModels;
using Chronomarker.Views;
using Microsoft.Extensions.DependencyInjection;

namespace Chronomarker;

public partial class App : Application
{
    public const bool MockGame = false;

    public override void Initialize()
    {
        AvaloniaXamlLoader.Load(this);
    }

    public override void OnFrameworkInitializationCompleted()
    {
        var collection = new ServiceCollection();
        collection.AddSingleton<LogModel>();
        collection.AddSingleton<StatusModel>();
        collection.AddSingleton<WatchModel>();
        collection.AddSingleton<MainViewModel>();

        collection.AddSingleton<LogService>();
        collection.AddSingleton<ProxyWatchService>();

        if (MockGame)
            collection.AddSingleton<IGameService, MockGameService>();
        else
            collection.AddSingleton<IGameService, GameService>();

        var services = collection.BuildServiceProvider();
        Resources.Add(typeof(IServiceProvider), services);
        var mv = services.GetRequiredService<MainViewModel>();
        if (ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop)
        {
            desktop.MainWindow = MockGame
                ? new MockGameWindow { DataContext = mv }
                : new MainWindow { DataContext = mv };
        }
        else if (!Design.IsDesignMode)
        {
            throw new PlatformNotSupportedException();
        }

        base.OnFrameworkInitializationCompleted();
    }
}

public static class AppServiceProvider
{
    public static IServiceProvider GetServiceProvider() =>
        (IServiceProvider)App.Current!.Resources[typeof(IServiceProvider)]!;

    public static T GetRequiredService<T>() where T : notnull => GetServiceProvider().GetRequiredService<T>();
}
