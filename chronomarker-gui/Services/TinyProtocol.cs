using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Chronomarker.Services;

internal class UShortBitStream
{
    private readonly List<ushort> data;
    private int bitsLeft = 0;

    public int ByteSize => data.Count * sizeof(ushort);
    public ReadOnlySpan<byte> Data => MemoryMarshal.AsBytes(CollectionsMarshal.AsSpan(data));

    public UShortBitStream(int capacity = 64) => data = new((capacity + 1) / sizeof(ushort));

    public void Clear()
    {
        bitsLeft = 0;
        data.Clear();
    }

    public void Write(int value, int bits)
    {
        if (bits < 1 || bits > 16)
            throw new ArgumentOutOfRangeException(nameof(bits));
        if (value < 0 || value >= (1 << bits))
            throw new ArgumentOutOfRangeException(nameof(value));

        if (bitsLeft < bits)
        {
            if (bitsLeft > 0)
                WriteInternal(ref value, ref bits);
            data.Add(0);
            bitsLeft = 16;
        }
        if (bitsLeft < bits)
            throw new InvalidProgramException();
        WriteInternal(ref value, ref bits);
        if (value != 0 || bits != 0)
            throw new InvalidProgramException();
    }

    private void WriteInternal(ref int value, ref int bits)
    {
        int wbits = Math.Min(bitsLeft, bits);
        data[^1] |= (ushort)((value & ((1 << wbits) - 1)) << (16 - bitsLeft));
        value >>= wbits;
        bitsLeft -= wbits;
        bits -= wbits;
    }

    public void Write(string str, int countBits = 4)
    {
        Write(str.Length, countBits);
        for (int i = 0; i < str.Length; i++)
        {
            int ch = char.ToUpperInvariant(str[i]) - 32;
            if (ch < 0 || ch > 63)
                ch = 0;
            Write(ch, 6);
        }
    }
}

internal class TinyProtocol
{
    private const int CurrentVersion = 0;
    private const int TemperatureZero = -294; // rounded down

    private const int ProviderMessageTypeBits = 5;
    private const int VersionBits = 3;
    private const int O2CO2Bits = 6;
    private const int HeadingBits = 7;
    private const int LocalTimeBits = 6;
    private const int GravityBits = 8; // divided by 100
    private const int OxygenBits = 7;
    private const int TemperatureBits = 11; // shifted by absolute zero point
    private const int PlayerFlagBits = 3;
    private const int PlanetNameCountBits = 4;
    private const int LocationNameCountBits = 5;
    private const int EffectBits = 3;
    private const int AlertIconBits = 4;
    private const int AlertTextBits = 5;

    private enum ProviderMessage
    {
        Nop = 0,
        Reset,
        O2,
        CO2,
        Heading,
        PlayerFlags,
        PlanetName,
        PlanetStats,
        LocalTime,
        LocationName,
        SetPersonalEffects,
        SetEnvEffects,
        PositiveAlert,
        NegativeAlert
    }

    private enum WatchMessage
    {
        StillAlive = 0,
        INeedVersion = 1,
        NeedEverything = 2,
    }

    public enum PersonalEffectType
    {
        Cardio,
        Skeletal,
        Nervous,
        Digestive,
        Misc
    }
    public static PersonalEffectType? PersonalEffectFromIconName(string iconName) => iconName switch
    {
        "PersonalEffect_CardioRespiratoryCirculatory" => PersonalEffectType.Cardio,
        "PersonalEffect_SkeletalMuscular" => PersonalEffectType.Skeletal,
        "PersonalEffect_NervousSystem" => PersonalEffectType.Nervous,
        "PersonalEffect_DigestiveImmune" => PersonalEffectType.Digestive,
        "PersonalEffect_Misc" => PersonalEffectType.Misc,
        _ => null
    };

    public enum EnvEffectType
    {
        Radiation = 0,
        Thermal,
        Airborne,
        Corrosive
    }

    public static EnvEffectType? EnvEffectFromIconName(string iconName) => iconName switch
    {
        "HazardEffect_Radiation" => EnvEffectType.Radiation,
        "HazardEffect_Thermal" => EnvEffectType.Thermal,
        "HazardEffect_Airborne" => EnvEffectType.Airborne,
        "HazardEffect_Corrosive" => EnvEffectType.Corrosive,
        _ => null
    };

    public enum AlertIcon
    {
        None,
        Radiation,
        Thermal,
        Airborne,
        Corrosive,
        Cardio,
        Skeletal,
        Nervous,
        Digestive,
        Misc,
        Restore
    }
    public static AlertIcon? AlertIconFromName(string iconName) => iconName.Replace("_Positive", "").Replace("_Negative", "") switch
    {
        "HazardEffect_Radiation" => AlertIcon.Radiation,
        "HazardEffect_Thermal" => AlertIcon.Thermal,
        "HazardEffect_Airborne" => AlertIcon.Airborne,
        "HazardEffect_Corrosive" => AlertIcon.Corrosive,
        "HazardEffect_RestoreSoak" => AlertIcon.Restore,
        "PersonalEffect_CardioRespiratoryCirculatory" => AlertIcon.Cardio,
        "PersonalEffect_SkeletalMuscular" => AlertIcon.Skeletal,
        "PersonalEffect_NervousSystem" => AlertIcon.Nervous,
        "PersonalEffect_DigestiveImmune" => AlertIcon.Digestive,
        "PersonalEffect_Misc" => AlertIcon.Misc,
        "no icon" => AlertIcon.None,
        _ => null
    };

    [Flags]
    private enum PlayerFlags
    {
        IsInSpaceship = (1 << 0),
        IsLanded = (1 << 1),
        IsScanning = (1 << 2),
    }

    public delegate void QueuePacketHandler(ReadOnlySpan<byte> message);

    private const int MaxMessageSize = 64;
    private readonly object mutex = new();
    private readonly LogService logService;
    private readonly QueuePacketHandler queuePacket;
    // to prevent messages of the same type in the packet we keep a list to remove and reconstruct the pending packet
    private readonly UShortBitStream bitStream = new(MaxMessageSize + sizeof(ushort));
    private readonly List<(ProviderMessage type, Action write)> queuedMessages = new(16);
    private int
        o2, co2,
        heading,
        localTime,
        planetGravity, planetTemp, planetOxygen,
        playerFlags;
    private string planetName = "", locationName = "";
    private readonly List<PersonalEffectType> personalEffects = new(5);
    private readonly List<EnvEffectType> envEffects = new(4);

    public bool HasPendingMessages => queuedMessages.Count > 0;

    public TinyProtocol(LogService logService, QueuePacketHandler queuePacket)
    {
        this.logService = logService;
        this.queuePacket = queuePacket;
    }

    public void HandleGameMessage(IGameMessage message)
    {
        lock(mutex)
        {
            switch(message)
            {
                case PlayerFrequentMessage msg:
                    QueueO2(msg.fOxygen / msg.fMaxO2CO2, true);
                    QueueCO2(msg.fCarbonDioxide / msg.fMaxO2CO2, true);
                    break;
                case LocalEnvironmentMessage msg:
                    QueuePlanetName(msg.sBodyName, true);
                    QueueLocationName(msg.sLocationName, true);
                    QueuePlanetStats(msg.fGravity, msg.fTemperature, msg.fOxygenPercent, true);
                    QueuePlayerFlags(default(PlayerFlags) |
                        (msg.bInSpaceship ? PlayerFlags.IsInSpaceship : default) |
                        (msg.bIsScanning ? PlayerFlags.IsScanning : default) |
                        (msg.bIsLanded ? PlayerFlags.IsLanded : default),
                        true);
                    break;
                case LocalEnvFrequentMessage msg:
                    QueueLocalTime(msg.fLocalPlanetTime, true);
                    break;
                case PersonalEffectsMessage msg:
                    QueuePersonalEffects(msg.aPersonalEffects
                        .Select(e => PersonalEffectFromIconName(e.sEffectIcon))
                        .Where(e => e != null)
                        .Select(e => e!.Value)
                        .ToArray(), true);
                    break;
                case EnvironmentEffectsMessage msg:
                    QueueEnvEffects(msg.aEnvironmentEffects
                        .Select(e => EnvEffectFromIconName(e.sEffectIcon))
                        .Where(e => e != null)
                        .Select(e => e!.Value)
                        .ToArray(), true);
                    break;
                case AlertsMessage msg:
                    foreach (var alert in msg.aAlerts)
                        QueueAlert(alert);
                    break;
            }
        }
    }

    public void HandleWatchMessage(ReadOnlySpan<byte> message)
    {
        lock(mutex)
        {
            if (message.Length < 1)
                return;
            switch((WatchMessage)message[0])
            {
                case WatchMessage.StillAlive: break;
                case WatchMessage.INeedVersion:
                    if (message.Length < 2)
                        logService.Log("To small INeedVersion message from watch");
                    else if (message[1] != CurrentVersion)
                        logService.Log("Watch wants other version of protocol. Too bad. It won't work.");
                    QueueEverything();
                    break;
                case WatchMessage.NeedEverything:
                    QueueEverything();
                    break;
                default:
                    logService.Log($"Unknown watch message type {message[0]:X2}");
                    break;
            }
        }
    }

    public void FlushPendingMessages()
    {
        lock (mutex)
        {
            if (bitStream.ByteSize == 0)
                return;
            queuePacket(bitStream.Data);
            bitStream.Clear();
            queuedMessages.Clear();
        }
    }

    public void QueueEverything()
    {
        lock (mutex)
        {
            bitStream.Clear();
            queuedMessages.Clear();
            QueueReset();
            QueueInteger(ProviderMessage.O2, o2, ref o2, O2CO2Bits, false);
            QueueInteger(ProviderMessage.CO2, co2, ref co2, O2CO2Bits, false);
            QueueInteger(ProviderMessage.Heading, heading, ref heading, HeadingBits, false);
            QueueInteger(ProviderMessage.Heading, playerFlags, ref playerFlags, PlayerFlagBits, false);
            QueueInteger(ProviderMessage.Heading, localTime, ref localTime, LocalTimeBits, false);
            QueuePlanetName(planetName, false);
            QueueLocationName(locationName, false);
            AddMessage(ProviderMessage.PlanetStats, () =>
            {
                bitStream.Write(planetGravity, GravityBits);
                bitStream.Write(planetTemp, TemperatureBits);
                bitStream.Write(planetOxygen, OxygenBits);
            });
            FlushPendingMessages();
        }
    }

    private void QueueInteger(ProviderMessage type, int newValue, ref int oldValue, int bits, bool doCheck)
    {
        if (doCheck && newValue == oldValue)
            return;
        newValue = Math.Clamp(newValue, 0, (1 << bits) - 1);
        oldValue = newValue;
        AddMessage(type, () => bitStream.Write(newValue, bits));
    }

    private void QueueInteger(ProviderMessage type, float newValue, ref int oldValue, int bits, bool doCheck)
    {
        int newIntValue = (int)(Math.Clamp(newValue, 0f, 1f) * ((1 << bits) - 1) + 0.5f);
        QueueInteger(type, newIntValue, ref oldValue, bits, doCheck);
    }

    private void QueueReset() => AddMessage(ProviderMessage.Reset, () => { });

    private void QueueO2(float o2, bool doCheck) =>
        QueueInteger(ProviderMessage.O2, o2, ref this.o2, O2CO2Bits, doCheck);

    private void QueueCO2(float co2, bool doCheck) =>
        QueueInteger(ProviderMessage.CO2, co2, ref this.co2, O2CO2Bits, doCheck);

    private void QueueHeading(float heading, bool doCheck) =>
        QueueInteger(ProviderMessage.Heading, heading, ref this.heading, HeadingBits, doCheck);

    private void QueuePlayerFlags(PlayerFlags flags, bool doCheck) =>
        QueueInteger(ProviderMessage.PlayerFlags, (int)flags, ref this.playerFlags, PlayerFlagBits, doCheck);

    private void QueueLocalTime(float localTime, bool doCheck) =>
        QueueInteger(ProviderMessage.LocalTime, localTime, ref this.localTime, LocalTimeBits, doCheck);

    private void QueueString(ProviderMessage type, string newValue, ref string oldValue, int lengthBits, bool doCheck)
    {
        if (doCheck && newValue == oldValue)
            return;
        oldValue = newValue;
        AddMessage(type, () => bitStream.Write(newValue, lengthBits));
    }

    private void QueuePlanetName(string planetName, bool doCheck) =>
        QueueString(ProviderMessage.PlanetName, planetName, ref this.planetName, PlanetNameCountBits, doCheck);

    private void QueueLocationName(string locationName, bool doCheck) =>
        QueueString(ProviderMessage.LocationName, locationName, ref this.locationName, LocationNameCountBits, doCheck);

    private int BitClamp(float value, int bits) =>
        Math.Clamp((int)(value + 0.5f), 0, (1 << bits) - 1);

    private void QueuePlanetStats(float gravity, float temperature, float oxygen, bool doCheck)
    {
        int gravityI = BitClamp(gravity * 100f, GravityBits);
        int temperatureI = BitClamp(temperature - TemperatureZero, TemperatureZero);
        int oxygenI = BitClamp(oxygen * 100f, OxygenBits);
        if (doCheck && planetGravity == gravityI && planetTemp == temperatureI && planetOxygen == oxygenI)
            return;
        planetGravity = gravityI;
        planetTemp = temperatureI;
        planetOxygen = oxygenI;
        AddMessage(ProviderMessage.PlanetStats, () =>
        {
            bitStream.Write(gravityI, GravityBits);
            bitStream.Write(temperatureI, TemperatureBits);
            bitStream.Write(oxygenI, OxygenBits);
        });
    }

    private void QueueEffects<TEnum>(ProviderMessage messageType, IReadOnlyList<TEnum> newValues, List<TEnum> oldValues, bool doCheck) where TEnum : Enum
    {
        if (doCheck && oldValues.SequenceEqual(newValues))
            return;
        if (!ReferenceEquals(oldValues, newValues))
        {
            oldValues.Clear();
            oldValues.AddRange(newValues);
        }
        AddMessage(messageType, () =>
        {
            int sendCount = Math.Min(newValues.Count, (1 << EffectBits) - 1);
            bitStream.Write(sendCount, EffectBits);
            foreach (var value in newValues.Take(sendCount))
                bitStream.Write(value.GetHashCode(), EffectBits);
        });
    }

    private void QueuePersonalEffects(IReadOnlyList<PersonalEffectType> effects, bool doCheck) =>
        QueueEffects(ProviderMessage.SetPersonalEffects, effects, personalEffects, doCheck);

    private void QueueEnvEffects(IReadOnlyList<EnvEffectType> effects, bool doCheck) =>
        QueueEffects(ProviderMessage.SetEnvEffects, effects, envEffects, doCheck);

    private void QueueAlert(Alert alert)
    {
        if (AlertIconFromName(alert.sEffectIcon) is not AlertIcon icon)
            return;
        AddMessage(alert.bIsPositive ? ProviderMessage.PositiveAlert : ProviderMessage.NegativeAlert, () =>
        {
            bitStream.Write((int)icon, AlertIconBits);
            bitStream.Write(alert.sAlertText, AlertTextBits);
            bitStream.Write(alert.sAlertSubText, AlertTextBits);
        });
    }

    private void AddMessage(ProviderMessage message, Action writeMessage)
    {
        if (message is not ProviderMessage.NegativeAlert or ProviderMessage.PositiveAlert)
        {
            int previousI = queuedMessages.FindIndex(t => t.type == message);
            if (previousI >= 0)
            {
                queuedMessages.RemoveAt(previousI);
                ReconstructPacket();
            }
        }

        WriteMessage(message, writeMessage);
        if (bitStream.ByteSize > MaxMessageSize)
        {
            ReconstructPacket();
            FlushPendingMessages();
            WriteMessage(message, writeMessage);
            if (bitStream.ByteSize > MaxMessageSize)
                throw new ArgumentException("Packet is too large");
        }
        if (bitStream.ByteSize == MaxMessageSize)
            FlushPendingMessages();
        else
            queuedMessages.Add((message, writeMessage));
    }

    private void ReconstructPacket()
    {
        bitStream.Clear();
        foreach (var tuple in queuedMessages)
            WriteMessage(tuple);
    }

    private void WriteMessage((ProviderMessage type, Action writeBody) t) =>
        WriteMessage(t.type, t.writeBody);

    private void WriteMessage(ProviderMessage type, Action writeBody)
    {
        bitStream.Write((int)type, ProviderMessageTypeBits);
        writeBody();
    }
}
