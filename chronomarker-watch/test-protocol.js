const fs = require("fs");

/** @type Buffer[] */
const packets = fs
.readFileSync("chronomarker-gui.Desktop/bin/Debug/net8.0-windows10.0.19041.0/tiny_messages.log", "utf-8")
.split("\n")
.map(l => l.trim())
.filter(l => l !== "")
.map(l => Buffer.from(l, "hex"));

class BitStream
{
    /**
     * @param {ArrayBuffer} buffer
     */
    constructor(buffer) {
        this.buffer = buffer;
        this.values = new Uint16Array(buffer.buffer, buffer.byteOffset, buffer.byteLength / 2);
        this.nextValue = 0;
        this.bits = 0;
        this.bitCount = 0;
    }

    /**
     * @param {number} bitCount 
     * @returns bits
     */
    read(bitCount) {
        if (bitCount < 0 || bitCount > 16)
            throw new Error("Invalid bit count");
        if (this.bitCount < bitCount) {
            if (this.nextValue >= this.values.length)
                throw new Error("end of stream");
            this.bits |= (this.values[this.nextValue++] << this.bitCount);
            this.bitCount += 16;
        }
        this.bitCount -= bitCount;
        const value = this.bits & ((1 << bitCount) - 1);
        this.bits >>= bitCount;
        return value;
    }

    readf(bitCount) {
        const i = this.read(bitCount);
        return i;// / ((1 << bitCount) - 1);
    }

    reads(lengthBitCount) {
        const l = this.read(lengthBitCount);
        let s = "";
        for (let i = 0; i < l; i++)
            s += String.fromCharCode(32 + this.read(6));
        return s;
    }

    bitsRemaining() {
        return (this.values.length - this.nextValue) * 16 + this.bitCount;
    }

    hasAtLeast(n) {
        return this.bitsRemaining() >= n;
    }
}

for (let packet of packets)
{
    console.log(packet.toString("hex"));
    try
    {
        let stream = new BitStream(packet);
        while (stream.hasAtLeast(5))
        {
            let type = stream.read(5);
            switch(type)
            {
                case 0: console.log("  - EndOfPacket"); break;
                case 1: console.log("  - Reset"); break;
                case 2: console.log(`  - O2: ${stream.readf(6)}`); break;
                case 3: console.log(`  - CO2: ${stream.readf(6)}`); break;
                case 4: console.log(`  - Heading: ${stream.readf(7)}`); break;
                case 5:
                    const flags = stream.read(3);
                    console.log(`  - Flags: ${flags & 1 ? 'InSpaceShip' : ''}, ${flags & 2 ? 'Landed' : '' }, ${flags & 4 ? 'Scanning' : ''}`);
                    break;
                case 8: console.log(`  - LocalTime: ${stream.readf(6)}`); break;
                case 6: console.log(`  - Body: ${stream.reads(4)}`); break;
                case 9: console.log(`  - Location: ${stream.reads(5)}`); break;
                case 7:
                    console.log(`  - Gravity: ${stream.readf(8) / 100}`);
                    console.log(`  - Temperature: ${stream.read(11) - 294}`);
                    console.log(`  - Oxygen: ${stream.read(7)}%`);
                    break;
                case 10: {
                    const c = stream.read(3);
                    for (let i = 0; i < c; i++)
                        console.log(`  - Personal: ${['Cardio', 'Skeletal', 'Nervous', 'Digestive', 'Misc'][stream.read(3)]}`);
                    break;
                }
                case 11: {
                    const c = stream.read(3);
                    for (let i = 0; i < c; i++)
                        console.log(`  - Environment: ${['Radiation', 'Thermal', 'Airborne', 'Corrosive'][stream.read(3)]}`);
                    break;
                }
                case 12:
                case 13:
                    console.log(`  - ${['Positive', 'Negative'][type-12]}: ${['None', 'Radiation', 'Thermal', 'Airborne', 'Corrosive', 'Cardio', 'Skeletal', 'Nervous', 'Digestive', 'Misc', 'Restore'][stream.read(4)]} "${stream.reads(5)}" "${stream.reads(5)}"`);
                    break;
                default:
                    console.log(`  - Invalid packet type (${type})`);
                    type = 0; // we cannot trust any more data in this packet
                    break;
            }
            if (type == 0)
                break;
        }
    }
    catch(e)
    {
        console.log("  - Invalid packet: " + e);
    }
    console.log();
}
