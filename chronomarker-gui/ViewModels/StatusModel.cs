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

    public float O2 { get; private set; } = 60f;
    public float CO2 { get; private set; } = 20f;
    public float InvCO2 => MaxO2CO2 - CO2;
    public float MaxO2CO2 { get; private set; } = 120f;
    public bool IsInSpaceship { get; private set; } = false;
    public bool IsLanded { get; private set; } = true;
    public bool IsScanning { get; private set; } = true;
    public float Gravity { get; private set; } = 0.9f;
    public float Oxygen { get; private set; } = 2;
    public float Temperature { get; private set; } = 23f;
    public string BodyName { get; private set; } = "Hemerlo IV";
    public string LocationName { get; private set; } = "Soft Taco Planes";
    public float LocalTime { get; private set; } = 0.3f;
    public bool HasCardioEffect { get; private set; }
    public bool HasSkeletalEffect { get; private set; }
    public bool HasNervousEffect { get; private set; }
    public bool HasDigestiveEffect { get; private set; }
    public bool HasMiscEffect { get; private set; }
    public bool HasRadiationEffect { get; private set; }
    public bool HasThermalEffect { get; private set; }
    public bool HasAirborneEffect { get; private set; }
    public bool HasCorrosiveEffect { get; private set; }
    public ObservableCollection<AlertModel> Alerts { get; } = [
        new() { EffectIcon="i1", AlertText="Warning", AlertSubText="That's not good", IsPositive = false},
        new() { EffectIcon="i2", AlertText="Information", AlertSubText="That is better", IsPositive = true},
    ];
}
