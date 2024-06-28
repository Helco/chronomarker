using System.Text.Json;
using NetMQ;
using NetMQ.Sockets;

namespace Chronomarker;

public partial class MainForm : Form
{
    private SubscriberSocket socket;
    private NetMQPoller poller;
    private AllGameMessages lastMessages;
    private LPV6 lpv6;

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

        lpv6 = new(PrintLine);
        lpv6.OnStatusChanged += status => PrintLine($"New status: {status}");
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
    }

    int i = 0;
    bool shouldSendData = true;

    private void Form1_Load(object sender, EventArgs e)
    {
        lpv6.Start();

        _ = Task.Run(async () =>
        {
            while (shouldSendData)
            {
                i++;
                await lpv6.SendMessage(Enumerable.Repeat((byte)(i % 64), i % 64).ToArray());
                PrintLine($"Sending {i%64} bytes");
                await Task.Delay(300);
            }
        });
    }

    private void PrintLine(string line)
    {
        BeginInvoke(() => textBox1.Text = line + "\r\n" + textBox1.Text);
    }

    private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
    {
        poller.Stop();
        poller.Dispose();
        socket.Close();
        socket.Dispose();
        shouldSendData = false;
        lpv6.Stop();
        lpv6.Dispose();
    }
}
