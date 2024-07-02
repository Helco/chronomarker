using System;
using System.Linq;
using NetMQ;
using NetMQ.Monitoring;
using NetMQ.Sockets;

namespace Chronomarker.Services;

internal enum GameConnection
{
    Disconnected,
    Connected
}

internal interface IGameService : IDisposable
{
    GameConnection Status { get; }
    event Action<GameConnection>? OnStatusChanged;
    event Action<IGameMessage>? OnMessage;
}

internal class GameService : IGameService
{
    private readonly SubscriberSocket socket;
    private readonly NetMQMonitor monitor;
    private readonly NetMQPoller poller;
    private bool disposedValue;
    private GameConnection _status;

    public event Action<GameConnection>? OnStatusChanged;
    public event Action<IGameMessage>? OnMessage;
    public GameConnection Status
    {
        get => _status;
        set
        {
            if (_status != value)
            {
                _status = value;
                OnStatusChanged?.Invoke(value);
            }
        }
    }

    public GameService()
    {
        socket = new();
        socket.Options.Linger = TimeSpan.Zero;
        socket.ReceiveReady += HandleGameMessage;
        monitor = new NetMQMonitor(socket, "inproc://monitor",
            SocketEvents.Connected | SocketEvents.Disconnected | SocketEvents.Closed);
        monitor.Closed += (_, _) => Status = GameConnection.Disconnected;
        monitor.Disconnected += (_, _) => Status = GameConnection.Disconnected;
        monitor.Connected += (_, _) => Status = GameConnection.Connected;
        poller = [ socket ];
        monitor.AttachToPoller(poller);

        socket.Connect("tcp://127.0.0.1:7201");
        socket.SubscribeToAnyTopic();
        poller.RunAsync();
    }

    private void HandleGameMessage(object? sender, NetMQSocketEventArgs e)
    {
        NetMQMessage? zmqMessage = null;
        while (socket.TryReceiveMultipartMessage(ref zmqMessage) && zmqMessage != null)
        {
            var fullFrame = zmqMessage.Aggregate((a, b) => new NetMQFrame(a.ToByteArray(true).Concat(b.ToByteArray(true)).ToArray()));
            if (IGameMessage.TryParse(fullFrame.ToByteArray(true), out var gameMessage))
                OnMessage?.Invoke(gameMessage);
        }
    }

    protected virtual void Dispose(bool disposing)
    {
        if (!disposedValue)
        {
            if (disposing)
            {
                poller.Stop();
                poller.Dispose();
                socket.Close();
                socket.Dispose();
            }
            disposedValue = true;
        }
    }

    public void Dispose()
    {
        // Do not change this code. Put cleanup code in 'Dispose(bool disposing)' method
        Dispose(disposing: true);
        GC.SuppressFinalize(this);
    }
}
