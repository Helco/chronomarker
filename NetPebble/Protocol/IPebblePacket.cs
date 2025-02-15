using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace NetPebble.Protocol;

public interface IPebblePacket
{
    ushort Endpoint { get; }

    void Serialize(BinaryWriter writer);

    void SerializeWithFrame(BinaryWriter writer)
    {
        Debug.Assert(writer.BaseStream.CanSeek);
        var startPos = writer.BaseStream.Position;
        writer.WriteBE((ushort)0);
        writer.WriteBE(Endpoint);
        Serialize(writer);
        var endPos = writer.BaseStream.Position;
        writer.BaseStream.Position = startPos;
        writer.WriteBE((ushort)(endPos - startPos - 4));
        writer.BaseStream.Position = endPos;
    }
}
