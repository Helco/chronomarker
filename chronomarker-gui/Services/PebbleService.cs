using System;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using System.Threading.Tasks;
using NetPebble.Protocol;

namespace Chronomarker.Services;

internal class PebbleService : IWatchService
{
    private class Connection : IDisposable, InfrequentPacketScheduler.IAdapter
    {
        private static readonly Guid AppGuid = Guid.Parse("4a022ac1-39ae-4903-b538-ea5c035a0e81");

        private byte transactionId;
        private bool disposedValue;
        public bool CanBeUsed => !disposedValue && !Cancellation.IsCancellationRequested;
        public readonly CancellationTokenSource Cancellation = new();
        public readonly TaskCompletionSource Completion = new();
        public required NetPebble.Transports.ITransport Transport { get; init; }
        public DateTime LastHeartbeat { get; private set; } = DateTime.UtcNow;
        public TimeSpan TimeSinceHeartbeat => DateTime.UtcNow - LastHeartbeat;

        public TinyProtocol Protocol { get; }
        public InfrequentPacketScheduler Scheduler { get; }
        public bool HasPendingMessages => Protocol.HasPendingMessages;

        public Connection(LogService logService)
        {
            Scheduler = new(this, logService);
            Protocol = new(logService, Scheduler.QueuePacket);
            Protocol.HandleWatchMessage([1, 0]);
        }

        public void MarkHeartbeat() => LastHeartbeat = DateTime.UtcNow;

        protected virtual void Dispose(bool disposing)
        {
            if (!disposedValue)
            {
                if (disposing)
                {
                    Scheduler.Dispose();
                    Transport.Dispose();
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

        public void FlushPendingMessages() => Protocol.FlushPendingMessages();
        public void QueueEverything() => Protocol.QueueEverything();

        public Task SendPacket(byte[] packet)
        {
            if (CanBeUsed)
            {
                var pebbleMessage = new NetPebble.Protocol.AppMessagePush(
                    unchecked(transactionId++),
                    AppGuid,
                    [new NetPebble.Protocol.AppMessageByteArray(16, packet)]);
                return Transport.Send(pebbleMessage, ct: default);
            }
            return Task.CompletedTask;
        }
    }

    private readonly LogService logService;
    private readonly Action<string> logMessage;
    private readonly object runLock = new();
    private Connection? currentConnection = null;
    private CancellationTokenSource? cancellation;
    private Task? runTask;
    private bool disposedValue;

    private WatchStatus _status;
    public WatchStatus Status
    {
        get => _status;
        set
        {
            if (_status == value)
                return;
            _status = value;
            OnStatusChanged?.Invoke(value);
        }
    }
    public bool IsRunning => cancellation != null;
    public event Action<WatchStatus>? OnStatusChanged;

    public bool PrintWatchLog { get; set; } // non-functional

    public PebbleService(LogService logService, IGameService gameService)
    {
        this.logService = logService;
        logMessage = logService.Log;
        gameService.OnMessage += HandleMessage;
    }

    public void Start()
    {
        lock (runLock)
        {
            Stop();
            Status = WatchStatus.Initial;
            cancellation = new();
            runTask = Task.Run(RunLoop);
        }
    }

    public void Stop()
    {
        lock (runLock)
        {
            try
            {
                cancellation?.Cancel();
                runTask?.Wait(TimeSpan.FromSeconds(3));
                cancellation?.Dispose();
            }
            catch (Exception) { }
            cancellation = null;
            runTask = null;
            Status = WatchStatus.Initial;
        }
    }

    private void HandleMessage(IGameMessage message)
    {
        lock(runLock)
        {
            if (currentConnection?.CanBeUsed is true)
                currentConnection.Protocol.HandleGameMessage(message);
        }
    }

    private class ConnectionException : Exception
    {
        public ConnectionException() { }
        public ConnectionException(string? message) : base(message) { }
        public ConnectionException(string? message, Exception? innerException) : base(message, innerException) { }
    }

    private async Task<Connection> Connect()
    {
        try
        {
            var transport = new NetPebble.Transports.WebsocketTransport("ws://127.0.0.1:38747");
            await transport.ConnectAsync(default);

            // Connection
            var connection = new Connection(logService)
            {
                Transport = transport
            };
            return connection;
        }
        catch (Exception ex) when (ex is not ConnectionException)
        {
            throw new ConnectionException(ex.Message, ex);
        }
    }

    private void HandleDisconnect(Connection connection, string reason)
    {
        if (!connection.CanBeUsed)
            return;
        connection.Cancellation.Cancel();
        logMessage($"Disconnect {connection}: {reason}");
        Status = WatchStatus.Disconnected;
        Task.Run(() =>
        {
            try
            {
                if (!connection.Completion.Task.Wait(TimeSpan.FromSeconds(3)))
                    throw new TimeoutException("Connection tasks did not finish in time");
            }
            catch (Exception ex)
            {
                logMessage($"{ex.GetType().Name} during disconnect: " + ex.Message);
            }
            connection.Completion.TrySetResult();
            try
            {
                connection.Dispose();
            }
            catch (Exception ex)
            {
                logMessage($"{ex.GetType().Name} during disconnect disposal: " + ex.Message);
            }
        });
    }

    private void HandleReceive(Connection connection, byte[] bytes)
    {
        if (!connection.CanBeUsed)
            return;
        connection.MarkHeartbeat();
        connection.Protocol.HandleWatchMessage(bytes);
    }

    private TimeSpan Timeout = TimeSpan.FromSeconds(5);
    private async Task TaskTimeout(Connection connection)
    {
        try
        {
            while (connection.CanBeUsed)
            {
                await Task.Delay(Timeout - connection.TimeSinceHeartbeat, connection.Cancellation.Token);
                if (connection.TimeSinceHeartbeat >= Timeout)
                {
                    //HandleDisconnect(connection, "Timeout");
                    //return;
                }
            }
        }
        catch (OperationCanceledException) { }
        finally
        {
            connection.Completion.TrySetResult();
        }
    }

    private async Task RunLoop()
    {
        try
        {
            var token = cancellation!.Token;
            while (!token.IsCancellationRequested)
            {
                Status = WatchStatus.Watching;
                Status = WatchStatus.Connecting;
                logMessage("Connecting...");
                var connection = currentConnection = await Connect();
                Status = WatchStatus.Connected;
                logMessage("Connected");
                token.Register(connection.Cancellation.Cancel);
                _ = Task.Run(() => TaskTimeout(connection));
                await connection.Completion.Task;
                HandleDisconnect(connection, "Unknown disconnection event");
            }
        }
        catch (OperationCanceledException) { }
    }

    protected virtual void Dispose(bool disposing)
    {
        if (disposedValue || !disposing)
            return;
        disposedValue = true;
    }

    public void Dispose()
    {
        Dispose(disposing: true);
        GC.SuppressFinalize(this);
    }
}
