using NetPebble.Protocol;
using NetPebble.Transports;

namespace NetPebble.DevTest;

internal class Program
{
    static async Task Main(string[] args)
    {
        Console.WriteLine("Hello, World!");
        //var transport = new WebsocketTransport("ws://127.0.0.1:38375");
        //var transport = new WebsocketTransport("ws://192.168.178.89:9000");
        var transport = new BluetoothClassicTransport();
        await transport.ConnectAsync(default);

        while (true)
        {
            var packet = new AppMessagePush(1,
                Guid.Parse("4a022ac1-39ae-4903-b538-ea5c035a0e81"),
                [new AppMessageInt<byte>(16, (byte)DateTime.Now.Second)]);
            await transport.Send(packet, default);
            await Task.Delay(3000);
        }
    }
}
