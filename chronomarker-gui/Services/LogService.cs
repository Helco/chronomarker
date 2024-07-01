using System;
using Avalonia.Threading;
using Chronomarker.ViewModels;

namespace Chronomarker.Services;

internal class LogService
{
    private readonly LogModel model;

    public LogService(LogModel model)
    {
        this.model = model;
    }

    public void Log(string message)
    {
        Dispatcher.UIThread.InvokeAsync(() => model.Lines.Insert(0, message));
    }
}
