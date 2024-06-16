using System.Text.Json;
using NetMQ;
using NetMQ.Sockets;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;

namespace Chronomarker;

public partial class MainForm : Form
{
    private SubscriberSocket socket;
    private NetMQPoller poller;
    private AllGameMessages lastMessages;

    public MainForm()
    {
        InitializeComponent();

        socket = new SubscriberSocket();
        socket.Options.Linger = TimeSpan.Zero;
        socket.ReceiveReady += HandleGameMessage;
        socket.Connect("tcp://127.0.0.1:7201");
        socket.SubscribeToAnyTopic();

        poller = new() { socket };
        poller.RunAsync();

        label1.Text = JsonSerializer.Serialize(lastMessages, new JsonSerializerOptions() { WriteIndented = true, IncludeFields = true });
    }

    private void HandleGameMessage(object? sender, NetMQSocketEventArgs e)
    {
        NetMQMessage? zmqMessage = null;
        string? newText = null;
        while (socket.TryReceiveMultipartMessage(ref zmqMessage) && zmqMessage != null)
        {
            var fullFrame = zmqMessage.Aggregate((a, b) => new NetMQFrame(a.ToByteArray(true).Concat(b.ToByteArray(true)).ToArray()));
            if (IGameMessage.TryParse(fullFrame.ToByteArray(true), out var gameMessage))
            {
                lastMessages.Set(gameMessage);
                newText = JsonSerializer.Serialize(lastMessages, new JsonSerializerOptions() { WriteIndented = true, IncludeFields = true });
            }
        }
        if (newText != null)
            Invoke(() => label1.Text = newText);
    }

    private async void Form1_Load(object sender, EventArgs e)
    {
        var adapter = await BluetoothAdapter.GetDefaultAsync();
        var watcher = new BluetoothLEAdvertisementWatcher();
        watcher.Received += async (s, e) =>
        {
            var device = await BluetoothLEDevice.FromBluetoothAddressAsync(e.BluetoothAddress);
        };
    }

    private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
    {
        poller.Stop();
        poller.Dispose();
        socket.Close();
        socket.Dispose();
    }
}
