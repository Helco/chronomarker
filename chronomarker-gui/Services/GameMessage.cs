using System;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using static Chronomarker.Services.IGameMessage;

namespace Chronomarker.Services;

enum GameMessageType : byte
{
    PlayerFrequent = 0,
    LocalEnvironment,
    LocalEnvFrequent,
    PersonalEffects,
    EnvironmentEffects,
    Alerts
}

internal interface IGameMessageValue<TValue>
{
    bool TryParse(ref ReadOnlySpan<byte> data);
}

internal struct AllGameMessages
{
    public PlayerFrequentMessage playerFrequent;
    public LocalEnvironmentMessage localEnvironment;
    public LocalEnvFrequentMessage localEnvFrequent;
    public PersonalEffectsMessage personalEffects;
    public EnvironmentEffectsMessage envEffects;
    public AlertsMessage alerts;

    public void Set(IGameMessage message)
    {
        switch (message)
        {
            case PlayerFrequentMessage m: playerFrequent = m; break;
            case LocalEnvironmentMessage m: localEnvironment = m; break;
            case LocalEnvFrequentMessage m: localEnvFrequent = m; break;
            case PersonalEffectsMessage m: personalEffects = m; break;
            case EnvironmentEffectsMessage m: envEffects = m; break;
            case AlertsMessage m: alerts = m with { aAlerts = alerts.aAlerts?.Concat(m.aAlerts).ToArray() ?? m.aAlerts }; break;
        }
    }
}

internal interface IGameMessage
{
    public const ulong MaxStringSize = 64;
    public const ulong MaxArraySize = 8;
    public GameMessageType Type { get; }

    bool TryParse(ReadOnlySpan<byte> data);

    public static bool TryParse(ReadOnlySpan<byte> data, out IGameMessage message)
    {
        message = null!;
        if (data.IsEmpty)
            return false;

        switch ((GameMessageType)data[0])
        {
            case GameMessageType.PlayerFrequent: return TryParse<PlayerFrequentMessage>(data, out message);
            case GameMessageType.LocalEnvironment: return TryParse<LocalEnvironmentMessage>(data, out message);
            case GameMessageType.LocalEnvFrequent: return TryParse<LocalEnvFrequentMessage>(data, out message);
            case GameMessageType.PersonalEffects: return TryParse<PersonalEffectsMessage>(data, out message);
            case GameMessageType.EnvironmentEffects: return TryParse<EnvironmentEffectsMessage>(data, out message);
            case GameMessageType.Alerts: return TryParse<AlertsMessage>(data, out message);
            default: return false;
        }
    }

    private static bool TryParse<TGameMessage>(ReadOnlySpan<byte> data, out IGameMessage message) where TGameMessage : struct, IGameMessage
    {
        message = null!;
        var value = new TGameMessage();
        if (!value.TryParse(data))
            return false;
        message = value;
        return true;
    }

    public unsafe static bool Read<TValue>(ref ReadOnlySpan<byte> data, out TValue value) where TValue : unmanaged
    {
        value = default;
        if (data.Length < sizeof(TValue))
            return false;
        value = MemoryMarshal.Cast<byte, TValue>(data)[0];
        data = data[sizeof(TValue)..];
        return true;
    }

    public unsafe static bool Read(ref ReadOnlySpan<byte> data, out string value)
    {
        value = "";
        if (!Read(ref data, out ulong size) || size > MaxStringSize)
            return false;
        if ((ulong)data.Length < size)
            return false;
        value = Encoding.UTF8.GetString(data[..(int)size]);
        data = data[(int)size..];
        return true;
    }

    public unsafe static bool Read<TValue>(ref ReadOnlySpan<byte> data, out TValue[] array) where TValue : struct, IGameMessageValue<TValue>
    {
        array = [];
        if (!Read(ref data, out ulong size) || size > MaxArraySize)
            return false;
        array = new TValue[(int)size];
        for (int i = 0; i < (int)size; i++)
        {
            if (!array[i].TryParse(ref data))
                return false;
        }
        return true;
    }

    public static bool ReadAndCheckType(ref ReadOnlySpan<byte> data, IGameMessage message) =>
        Read<GameMessageType>(ref data, out var type) && type == message.Type;
}

internal struct PlayerFrequentMessage : IGameMessage
{
    public GameMessageType Type => GameMessageType.PlayerFrequent;
    public float
        fHealth, fMaxHealth,
        fStarPower, fMaxStarPower,
        fHealthGainPct,
        fHealthBarDamage,
        fOxygen,
        fCarbonDioxide,
        fMaxO2CO2;
    public uint uDetectionLevel;

    public bool TryParse(ReadOnlySpan<byte> data)
    {
        return ReadAndCheckType(ref data, this) &&
            Read(ref data, out fHealth) &&
            Read(ref data, out fMaxHealth) &&
            Read(ref data, out fStarPower) &&
            Read(ref data, out fMaxStarPower) &&
            Read(ref data, out fHealthGainPct) &&
            Read(ref data, out fHealthBarDamage) &&
            Read(ref data, out fOxygen) &&
            Read(ref data, out fCarbonDioxide) &&
            Read(ref data, out fMaxO2CO2) &&
            Read(ref data, out uDetectionLevel);
    }
}

internal struct LocalEnvironmentMessage : IGameMessage
{
    public GameMessageType Type => GameMessageType.LocalEnvironment;
    public string sBodyName, sLocationName, sLanguage;
    public uint uBodyType, uAlertTimeMs;
    public float fGravity, fOxygenPercent, fTemperature;
    public bool bInSpaceship, bIsScanning, bIsLanded;

    public bool TryParse(ReadOnlySpan<byte> data)
    {
        return ReadAndCheckType(ref data, this) &&
            Read(ref data, out sBodyName) &&
            Read(ref data, out uBodyType) &&
            Read(ref data, out uAlertTimeMs) &&
            Read(ref data, out fGravity) &&
            Read(ref data, out fOxygenPercent) &&
            Read(ref data, out fTemperature) &&
            Read(ref data, out sLocationName) &&
            Read(ref data, out bInSpaceship) &&
            Read(ref data, out bIsScanning) &&
            Read(ref data, out bIsLanded) &&
            Read(ref data, out sLanguage);
    }
}

internal struct LocalEnvFrequentMessage : IGameMessage
{
    public GameMessageType Type => GameMessageType.LocalEnvFrequent;
    public float fLocalPlanetTime, fLocalPlanetHoursPerDay, fGalacticStandardTime;

    public bool TryParse(ReadOnlySpan<byte> data)
    {
        return ReadAndCheckType(ref data, this) &&
            Read(ref data, out fLocalPlanetTime) &&
            Read(ref data, out fLocalPlanetHoursPerDay) &&
            Read(ref data, out fGalacticStandardTime);
    }
}

internal struct Effect : IGameMessageValue<Effect>
{
    public string sEffectIcon;
    public uint uiHandle, uiMarkerIconType;
    public float fHeading;

    public bool TryParse(ref ReadOnlySpan<byte> data)
    {
        return
            Read(ref data, out sEffectIcon) &&
            Read(ref data, out uiHandle) &&
            Read(ref data, out fHeading) &&
            Read(ref data, out uiMarkerIconType);
    }
}

internal struct Alert : IGameMessageValue<Alert>
{
    public string sEffectIcon, sAlertText, sAlertSubText;
    public bool bIsPositive;

    public bool TryParse(ref ReadOnlySpan<byte> data)
    {
        return
            Read(ref data, out sEffectIcon) &&
            Read(ref data, out sAlertText) &&
            Read(ref data, out sAlertSubText) &&
            Read(ref data, out bIsPositive);
    }
}

internal struct PersonalEffectsMessage : IGameMessage
{
    public GameMessageType Type => GameMessageType.PersonalEffects;
    public uint uAlertTimeMs;
    public Effect[] aPersonalEffects;

    public bool TryParse(ReadOnlySpan<byte> data)
    {
        return ReadAndCheckType(ref data, this) &&
            Read(ref data, out uAlertTimeMs) &&
            Read(ref data, out aPersonalEffects);
    }
}

internal struct EnvironmentEffectsMessage : IGameMessage
{
    public GameMessageType Type => GameMessageType.EnvironmentEffects;
    public uint uAlertTimeMs, uEnvIconPulseMinMs, uEnvIconPulseMaxMs;
    public float fSoakDamagePct;
    public bool bShouldPlayAlertAtFullSoak;
    public Effect[] aEnvironmentEffects;

    public bool TryParse(ReadOnlySpan<byte> data)
    {
        return ReadAndCheckType(ref data, this) &&
            Read(ref data, out uAlertTimeMs) &&
            Read(ref data, out uEnvIconPulseMinMs) &&
            Read(ref data, out uEnvIconPulseMaxMs) &&
            Read(ref data, out fSoakDamagePct) &&
            Read(ref data, out bShouldPlayAlertAtFullSoak) &&
            Read(ref data, out aEnvironmentEffects);
    }
}

internal struct AlertsMessage : IGameMessage
{
    public GameMessageType Type => GameMessageType.Alerts;
    public Alert[] aAlerts;

    public bool TryParse(ReadOnlySpan<byte> data)
    {
        return ReadAndCheckType(ref data, this) &&
            Read(ref data, out aAlerts);
    }
}
