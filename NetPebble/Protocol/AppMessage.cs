using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace NetPebble.Protocol;

public interface IPebbleAppMessagePacket : IPebblePacket
{
    ushort IPebblePacket.Endpoint => 0x30;
    byte Command { get; }
    byte TransactionId { get; }

    void IPebblePacket.Serialize(BinaryWriter writer)
    {
        writer.Write(Command);
        writer.Write(TransactionId);
    }
}

public readonly record struct AppMessageACK(byte TransactionId) : IPebbleAppMessagePacket
{
    public byte Command => 0xff;
}

public readonly record struct AppMessageNACK(byte TransactionId) : IPebbleAppMessagePacket
{
    public byte Command => 0x7f;
}

public readonly record struct AppMessagePush(byte TransactionId, Guid Uuid, IAppMessageTuple[] Tuples) : IPebbleAppMessagePacket
{
    public byte Command => 0x01;

    public void Serialize(BinaryWriter writer)
    {
        writer.Write(Command);
        writer.Write(TransactionId);
        writer.Write(Uuid);
        writer.Write(checked((byte)Tuples.Length));
        foreach (var tuple in Tuples)
            tuple.Serialize(writer);
    }
}

public interface IAppMessageTuple
{
    uint Key { get; }
    byte Type { get; }
    ushort Length { get; }

    void Serialize(BinaryWriter writer);
    void SerializeHeader(BinaryWriter writer)
    {
        writer.Write(Key);
        writer.Write(Type);
        writer.Write(Length);
    }
}

public readonly record struct AppMessageInt<T>(uint Key, T Value) : IAppMessageTuple
    where T : unmanaged, IBinaryInteger<T>, IMinMaxValue<T>
{
    public byte Type => (byte)(T.MinValue < T.Zero ? 3 : 2);
    public ushort Length => LengthConst;
    private static readonly ushort LengthConst = (ushort)T.AllBitsSet.GetByteCount();

    public void Serialize(BinaryWriter writer)
    {
        (this as IAppMessageTuple).SerializeHeader(writer);
        Span<byte> buffer = stackalloc byte[8];
        var count = Value.WriteLittleEndian(buffer);
        writer.Write(buffer[0..count]);
    }
}

public readonly record struct AppMessageString(uint Key, string Value) : IAppMessageTuple
{
    public byte Type => 1;
    public ushort Length => (ushort)(Encoding.UTF8.GetByteCount(Value) + 1);
    
    public void Serialize(BinaryWriter writer)
    {
        (this as IAppMessageTuple).SerializeHeader(writer);
        Span<byte> buffer = stackalloc byte[Length + 1];
        var count = Encoding.UTF8.GetBytes(Value, buffer);
        buffer[count++] = 0;
        writer.Write(buffer[0..count]);
    }
}

public readonly record struct AppMessageByteArray(uint Key, byte[] Value) : IAppMessageTuple
{
    public byte Type => 0;
    public ushort Length => (ushort)Value.Length;

    public void Serialize(BinaryWriter writer)
    {
        (this as IAppMessageTuple).SerializeHeader(writer);
        writer.Write(Value);
    }
}
