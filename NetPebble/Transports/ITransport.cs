using System;
using System.Threading;
using System.Threading.Tasks;
using NetPebble.Protocol;

namespace NetPebble.Transports;

public interface ITransport : IDisposable
{
    Task ConnectAsync(CancellationToken ct);
    Task Send(IPebblePacket packet, CancellationToken ct);
}
