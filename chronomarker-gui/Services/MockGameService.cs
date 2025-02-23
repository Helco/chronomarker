using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chronomarker.Services;

internal class MockGameService : IGameService
{
    public GameConnection Status { get; private set; }

    public event Action<GameConnection>? OnStatusChanged;
    public event Action<IGameMessage>? OnMessage;

    public void SendMessage(IGameMessage message)
    {
        if (Status is not GameConnection.Connected)
            OnStatusChanged?.Invoke(Status = GameConnection.Connected);
        OnMessage?.Invoke(message);
    }

    public void Dispose() { OnStatusChanged?.Invoke(Status = GameConnection.Disconnected); }
}
