using System;
using System.IO;

namespace NetPebble;

internal static class BinaryWriterExtensions
{
    public static void WriteBE(this BinaryWriter writer, ushort value) => writer.Write(Swap(value));

    public static void Write(this BinaryWriter writer, Guid guid)
    {
        Span<byte> bytes = stackalloc byte[16];
        if (!guid.TryWriteBytes(bytes, bigEndian: true, out _))
            throw new InvalidOperationException("Something is very wrong");
        writer.Write(bytes);
    }

    public static ushort Swap(ushort value) =>
        (ushort)((value >> 8) | (value << 8));
}
