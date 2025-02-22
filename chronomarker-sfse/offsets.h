#pragma once
#include <stdint.h>

// in a separate file for easier updates
// The hope with ID+CallOffset would be that offsets inside functions change rarer thus
// some game updates will not break this plugin (after upgrading the version database)

namespace C
{
	using ID = uint64_t;
	using Offset = uint64_t;
	using Size = uint64_t;

	constexpr ID ID_PlayerDataModels_ctor = 134825;
	constexpr ID ID_PlayerDataModel_dtor = 134886;
	constexpr ID ID_HUDDataModel_ctor = 138166;
	constexpr ID ID_HUDDataModel_dtor = 138504;
	constexpr ID ID_ScaleformInvokeOnFlush = 187162; // in method FlushEventsToFlash

	constexpr Offset Call_DtorToDtor2 = 0xf;
	constexpr Offset Call_PlayerFrequentDataModel_ctor = 0x14223BDEC - 0x14223BC34;
	constexpr Offset Call_HUDDataModel_ctor = 0x1422F9C42 - 0x1422F9C04;
	constexpr Offset Call_ScaleformInvokeOnFlush = 0x143171A7F - 0x1431717EC;

	constexpr Offset Offset_HUDDataModel_ToEnv = 7176;
	constexpr Offset Offset_HUDDataModel_AfterEnv = 7832;
	constexpr Offset Offset_HUDDataModel_ToWatchEffects = 10176;
	constexpr Offset Offset_WatchEffects_ToPersonalEffects = 4176;
	constexpr Offset Offset_WatchEffects_ToPersonalAlerts = 4416;
	constexpr Offset Offset_WatchEffects_ToEnvironmentEffects = 4624;
	constexpr Offset Offset_WatchEffects_ToEnvironmentAlerts = 4992;
	constexpr Offset Offset_WatchEffects_ToCustomAlerts = 5200;

	constexpr Offset Gap_PlayerFrequentDataModel_ToValues = 72;
	constexpr Offset Gap_HUDDataModel_ToEnv = Offset_HUDDataModel_ToEnv;
	constexpr Offset Gap_LocalEnvModel_ToStaticUIData = 2 * sizeof(void *);
	constexpr Offset Gap_LocalEnvModel_ToFrequentUIData = 58 * sizeof(void *);
	constexpr Offset Gap_HUDDataModel_AfterEnvToWatchEffects = Offset_HUDDataModel_ToWatchEffects - Offset_HUDDataModel_AfterEnv;

	constexpr Size Size_LocalEnvironmentData = Gap_LocalEnvModel_ToFrequentUIData - Gap_LocalEnvModel_ToStaticUIData;
	constexpr Size Size_Env = Offset_HUDDataModel_AfterEnv - Offset_HUDDataModel_ToEnv;
	constexpr Size Size_LocalEnvFrequentData = Size_Env - Size_LocalEnvironmentData - Gap_LocalEnvModel_ToStaticUIData;
	constexpr Size Size_PersonalEffects = Offset_WatchEffects_ToPersonalAlerts - Offset_WatchEffects_ToPersonalEffects;
	constexpr Size Size_PersonalAlerts = Offset_WatchEffects_ToEnvironmentEffects - Offset_WatchEffects_ToPersonalAlerts;
	constexpr Size Size_EnvironmentEffects = Offset_WatchEffects_ToEnvironmentAlerts - Offset_WatchEffects_ToEnvironmentEffects;
	constexpr Size Size_EnvironmentAlerts = Offset_WatchEffects_ToCustomAlerts - Offset_WatchEffects_ToEnvironmentAlerts;

	constexpr ID ID_AlertDataModel_setMembers_0 = 133649;
	constexpr Offset Call_AlertDataModel_setMembers_0 = 0x1421FDD5D - 0x1421FDCF0;
	constexpr ID ID_AlertDataModel_setMembers_1 = 133655;
	constexpr Offset Call_AlertDataModel_setMembers_1 = 0x1421FE0C2 - 0x1421FE034;
	constexpr ID ID_AlertDataModel_setMembers_2 = 133697;
	constexpr Offset Call_AlertDataModel_setMembers_2 = 0x1421FFDCE - 0x1421FF600;
}
