using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Avalonia.Media;

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
}
