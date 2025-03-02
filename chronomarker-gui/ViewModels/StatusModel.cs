using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avalonia.Media;
using Chronomarker.Services;
using ReactiveUI;

namespace Chronomarker.ViewModels;

internal class AlertModel : ViewModelBase
{
    public string EffectIcon { get; init; } = "";
    public string AlertText { get; init; } = "";
    public string AlertSubText { get; init; } = "";
    public bool IsPositive { get; init; }

    public IImmutableSolidColorBrush Border => IsPositive ? Brushes.Green : Brushes.Red;
}

internal class StatusModel : ViewModelBase
{
    private readonly IGameService gameService;

    [DesignOnly(true)] public StatusModel() { gameService = null!; }
    public StatusModel(IGameService gameService)
    {
        this.gameService = gameService;
        gameService.OnStatusChanged += HandleStatusChanged;
        gameService.OnMessage += HandleMessage;
    }

    public bool IsConnected { get; private set; }
    public string ConnectionText => IsConnected ? "Connected" : "Disconnected";
    public IImmutableSolidColorBrush ConnectionColor => IsConnected ? Brushes.Green : Brushes.Red;

    public float O2 { get; private set; }
    public float CO2 { get; private set; }
    public float InvCO2 => MaxO2CO2 - CO2;
    public float MaxO2CO2 { get; private set; }
    public bool IsInSpaceship { get; private set; }
    public bool IsLanded { get; private set; }
    public bool IsScanning { get; private set; }
    public float Gravity { get; private set; }
    public float Oxygen { get; private set; }
    public float Temperature { get; private set; }
    public BodyType BodyType { get; private set; }
    public string BodyName { get; private set; } = "";
    public string LocationName { get; private set; } = "";
    public float LocalTime { get; private set; }
    public bool HasCardioEffect { get; private set; }
    public bool HasSkeletalEffect { get; private set; }
    public bool HasNervousEffect { get; private set; }
    public bool HasDigestiveEffect { get; private set; }
    public bool HasMiscEffect { get; private set; }
    public bool HasRadiationEffect { get; private set; }
    public bool HasThermalEffect { get; private set; }
    public bool HasAirborneEffect { get; private set; }
    public bool HasCorrosiveEffect { get; private set; }
    public ObservableCollection<AlertModel> Alerts { get; } = [];

    private void HandleStatusChanged(GameConnection connection)
    {
        IsConnected = connection == GameConnection.Connected;
        Raise(nameof(IsConnected), nameof(ConnectionText), nameof(ConnectionColor));
    }

    private void HandleMessage(IGameMessage message)
    {
        switch(message)
        {
            case PlayerFrequentMessage msg:
                O2 = msg.fOxygen;
                CO2 = msg.fCarbonDioxide;
                MaxO2CO2 = msg.fMaxO2CO2;
                Raise(nameof(O2), nameof(CO2), nameof(MaxO2CO2), nameof(InvCO2));
                break;

            case LocalEnvironmentMessage msg:
                BodyName = msg.sBodyName;
                LocationName = msg.sLocationName;
                Gravity = msg.fGravity;
                Oxygen = msg.fOxygenPercent;
                Temperature = msg.fTemperature;
                BodyType = TinyProtocol.BodyTypeFromInt(msg.uBodyType);
                IsInSpaceship = msg.bInSpaceship;
                IsScanning = msg.bIsScanning;
                IsLanded = msg.bIsLanded;
                Raise(
                    nameof(BodyName), nameof(LocationName),
                    nameof(Gravity), nameof(Oxygen), nameof(Temperature), nameof(BodyType),
                    nameof(IsInSpaceship), nameof(IsScanning), nameof(IsLanded));
                break;

            case LocalEnvFrequentMessage msg:
                LocalTime = msg.fLocalPlanetTime;
                Raise(nameof(LocalTime));
                break;

            case PersonalEffectsMessage msg:
                HasCardioEffect = HasSkeletalEffect = HasNervousEffect = HasDigestiveEffect = HasMiscEffect = false;
                foreach (var effect in msg.aPersonalEffects)
                {
                    switch(TinyProtocol.PersonalEffectFromIconName(effect.sEffectIcon))
                    {
                        case TinyProtocol.PersonalEffectType.Cardio: HasCardioEffect = true; break;
                        case TinyProtocol.PersonalEffectType.Skeletal: HasSkeletalEffect = true; break;
                        case TinyProtocol.PersonalEffectType.Nervous: HasNervousEffect = true; break;
                        case TinyProtocol.PersonalEffectType.Digestive: HasDigestiveEffect = true; break;
                        case TinyProtocol.PersonalEffectType.Misc: HasMiscEffect = true; break;
                    }
                }
                Raise(nameof(HasCardioEffect), nameof(HasSkeletalEffect), nameof(HasNervousEffect), nameof(HasDigestiveEffect), nameof(HasMiscEffect));
                break;

            case EnvironmentEffectsMessage msg:
                HasRadiationEffect = HasThermalEffect = HasAirborneEffect = HasCorrosiveEffect = false;
                foreach (var effect in msg.aEnvironmentEffects)
                {
                    switch(TinyProtocol.EnvEffectFromIconName(effect.sEffectIcon))
                    {
                        case TinyProtocol.EnvEffectType.Radiation: HasRadiationEffect = true; break;
                        case TinyProtocol.EnvEffectType.Thermal: HasThermalEffect = true; break;
                        case TinyProtocol.EnvEffectType.Airborne: HasAirborneEffect = true; break;
                        case TinyProtocol.EnvEffectType.Corrosive: HasCorrosiveEffect = true; break;
                    }
                }
                Raise(nameof(HasRadiationEffect), nameof(HasThermalEffect), nameof(HasAirborneEffect), nameof(HasCorrosiveEffect));
                break;

            case AlertsMessage msg:
                foreach (var alert in msg.aAlerts)
                {
                    Alerts.Insert(0, new()
                    {
                        EffectIcon = TinyProtocol.AlertIconFromName(alert.sEffectIcon)?.ToString() ?? "<unknown>",
                        AlertText = alert.sAlertText,
                        AlertSubText = alert.sAlertSubText,
                        IsPositive = alert.bIsPositive
                    });
                }
                break;
        }
    }

    private void Raise(params string[] properties)
    {
        foreach (var property in properties)
            this.RaisePropertyChanged(property);
    }
}
