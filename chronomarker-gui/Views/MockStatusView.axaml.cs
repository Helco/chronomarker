using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Chronomarker.Services;
using Chronomarker.ViewModels;
using Microsoft.Extensions.DependencyInjection;

namespace Chronomarker.Views
{
    public partial class MockStatusView : UserControl
    {
        MockGameService gameService = new();
        StatusModel statusModel = new();

        public MockStatusView()
        {
            InitializeComponent();
            if (!Design.IsDesignMode)
            {
                var services = (IServiceProvider)App.Current!.Resources[typeof(IServiceProvider)]!;
                gameService = (MockGameService)services.GetRequiredService<IGameService>();
                statusModel = services.GetRequiredService<StatusModel>();

                gameService.SendMessage(new PlayerFrequentMessage()
                {
                    fOxygen = 60,
                    fCarbonDioxide = 20,
                    fMaxO2CO2 = 120
                });
                gameService.SendMessage(new LocalEnvironmentMessage()
                {
                    fGravity = 1.3f,
                    fOxygenPercent = 90,
                    fTemperature = 24,
                    sBodyName = "Hemerlo IV",
                    sLocationName = "Hemerlo IV"
                });
                gameService.SendMessage(new LocalEnvFrequentMessage()
                {
                    fLocalPlanetTime = 0.7f
                });
            }
        }

        private void O2_ValueChanged(object? sender, RangeBaseValueChangedEventArgs e)
        {
            gameService.SendMessage(PlayerFrequentMessage with
            {
                fOxygen = (float)e.NewValue,
                fCarbonDioxide = e.NewValue > statusModel.InvCO2 ? (float)(statusModel.MaxO2CO2 - e.NewValue) : statusModel.CO2
            });
        }

        private void InvCO2_ValueChanged(object? sender, RangeBaseValueChangedEventArgs e)
        {
            gameService.SendMessage(PlayerFrequentMessage with
            {
                fOxygen = e.NewValue < statusModel.O2 ? (float)e.NewValue : statusModel.O2,
                fCarbonDioxide = statusModel.MaxO2CO2 - (float)e.NewValue
            });
        }

        private PlayerFrequentMessage PlayerFrequentMessage => new()
        {
            fOxygen = statusModel.O2,
            fCarbonDioxide = statusModel.CO2,
            fMaxO2CO2 = statusModel.MaxO2CO2
        };

        private void InSpaceshipChanged(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                bInSpaceship = ((ToggleButton)sender!).IsChecked ?? false
            });
        }

        private void IsLandedChanged(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                bIsLanded = ((ToggleButton)sender!).IsChecked ?? false
            });
        }

        private void IsScanningChanged(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                bIsScanning = ((ToggleButton)sender!).IsChecked ?? false
            });
        }

        private void GravityChanged(object? sender, RangeBaseValueChangedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                fGravity = (float)e.NewValue
            });
        }

        private void OxygenChanged(object? sender, RangeBaseValueChangedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                fOxygenPercent = (float)e.NewValue
            });
        }

        private void TemperatureChanged(object? sender, RangeBaseValueChangedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                fTemperature = (float)e.NewValue
            });
        }

        private void BodyNameUnfocused(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                sBodyName = ((TextBox)sender!).Text ?? "<null>"
            });
        }

        private void BodyNameChanged(object? sender, TextChangedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                sBodyName = ((TextBox)sender!).Text ?? "<null>"
            });
        }

        private void LocationNameUnfocused(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                sLocationName = ((TextBox)sender!).Text ?? "<null>"
            });
        }

        private void LocationNameChanged(object? sender, TextChangedEventArgs e)
        {
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                sLocationName = ((TextBox)sender!).Text ?? "<null>"
            });
        }

        private void BodyTypeChanged(object? sender, Avalonia.Controls.SelectionChangedEventArgs e)
        {
            var comboBox = (ComboBox)sender!;
            gameService.SendMessage(LocalEnvironmentMessage with
            {
                uBodyType = (uint)comboBox.SelectedIndex
            });
        }

        private LocalEnvironmentMessage LocalEnvironmentMessage => new()
        {
            bInSpaceship = statusModel.IsInSpaceship,
            bIsLanded = statusModel.IsLanded,
            bIsScanning = statusModel.IsScanning,
            fGravity = statusModel.Gravity,
            fOxygenPercent = statusModel.Oxygen,
            fTemperature = statusModel.Temperature,
            sBodyName = statusModel.BodyName,
            sLocationName = statusModel.LocationName,
            uBodyType = (uint)statusModel.BodyType
        };

        private void LocalTimeChanged(object? sender, RangeBaseValueChangedEventArgs e)
        {
            gameService.SendMessage(LocalEnvFrequentMessage with
            {
                fLocalPlanetTime = (float)e.NewValue
            });
        }

        private LocalEnvFrequentMessage LocalEnvFrequentMessage => new()
        {
            fLocalPlanetTime = statusModel.LocalTime
        };

        private void PersonalEffectChanged(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            var button = (ToggleButton)sender!;
            if (button.IsChecked is true)
                SendPersonalEffects(PersonalEffects.Append(button.Tag?.ToString()));
            else
                SendPersonalEffects(PersonalEffects.Except([ button.Tag?.ToString() ]));
        }

        private IEnumerable<string?> PersonalEffects =>
        [
            statusModel.HasCardioEffect ? "PersonalEffect_CardioRespiratoryCirculatory" : null,
            statusModel.HasSkeletalEffect ? "PersonalEffect_SkeletalMuscular" : null,
            statusModel.HasNervousEffect ? "PersonalEffect_NervousSystem" : null,
            statusModel.HasDigestiveEffect ? "PersonalEffect_DigestiveImmune" : null,
            statusModel.HasMiscEffect ? "PersonalEffect_Misc" : null
        ];
        private void SendPersonalEffects(IEnumerable<string?> effectNames)
        {
            gameService.SendMessage(new PersonalEffectsMessage
            {
                aPersonalEffects = effectNames
                    .Distinct()
                    .Where(e => e is not null)
                    .Select(e => new Effect() { sEffectIcon = e! })
                    .ToArray()
            });
        }

        private void EnvEffectChanged(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            var button = (ToggleButton)sender!;
            if (button.IsChecked is true)
                SendEnvEffects(EnvEffects.Append(button.Tag?.ToString()));
            else
                SendEnvEffects(EnvEffects.Except([button.Tag?.ToString()]));
        }

        private IEnumerable<string?> EnvEffects =>
        [
            statusModel.HasRadiationEffect ? "HazardEffect_Radiation" : null,
            statusModel.HasThermalEffect ? "HazardEffect_Thermal" : null,
            statusModel.HasAirborneEffect ? "HazardEffect_Airborne" : null,
            statusModel.HasCorrosiveEffect ? "HazardEffect_Corrosive" : null
        ];

        private void SendEnvEffects(IEnumerable<string?> effectNames)
        {
            gameService.SendMessage(new EnvironmentEffectsMessage
            {
                aEnvironmentEffects = effectNames
                    .Distinct()
                    .Where(e => e is not null)
                    .Select(e => new Effect() { sEffectIcon = e! })
                    .ToArray()
            });
        }

        private void SendPersonalAlert(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            gameService.SendMessage(new AlertsMessage()
            {
                aAlerts = [ new() {
                    bIsPositive = false,
                    sEffectIcon = "PersonalEffect_NervousSystem",
                    sAlertText = "BROKEN BONES",
                    sAlertSubText = ""
                } ]
            });
        }

        private void SendPosEnvironmentalAlert(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            gameService.SendMessage(new AlertsMessage()
            {
                aAlerts = [ new() {
                    bIsPositive = true,
                    sEffectIcon = "HazardEffect_RestoreSoak",
                    sAlertText = "RESTORING",
                    sAlertSubText = "GOOD STUFF"
                } ]
            });
        }

        private void SendNegEnvironmentalAlert(object? sender, Avalonia.Interactivity.RoutedEventArgs e)
        {
            gameService.SendMessage(new AlertsMessage()
            {
                aAlerts = [ new() {
                    bIsPositive = false,
                    sEffectIcon = "HazardEffect_Airborne",
                    sAlertText = "AIRBORNE",
                    sAlertSubText = "GAS VENT"
                } ]
            });
        }
    }
}
