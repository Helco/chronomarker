
#include <ctime>
#include <ShlObj_core.h>
#include <string_view>
#include <sfse/GameConsole.h>
#include <sfse/ScaleformValue.h>
#include <sfse/PluginAPI.h>
#include <sfse_common/sfse_version.h>
#include <sfse_common/BranchTrampoline.h>
#include <sfse_common/Log.h>

using namespace std::string_view_literals;

struct BSFixedStringCS {
	void *poolEntry; // TODO: get to string data...
};
static_assert(sizeof(BSFixedStringCS) == 8);

template<typename TValue> struct UIValue {
	void *vtptr;
	void *unk1, *unk2;
	TValue value;
};
static_assert(sizeof(UIValue<float>) == 32);
static_assert(sizeof(UIValue<double>) == 32);
static_assert(sizeof(UIValue<unsigned int>) == 32);
static_assert(sizeof(UIValue<BSFixedStringCS>) == 32);

template<typename Me, uintptr_t ctorAddress_, uintptr_t callAddress_>
struct DataModelHooks {
	inline static Me *instance = nullptr;

	static Me *MyCtor(Me *thiz) {
		instance = thiz;
		return (*ctor)(thiz);
	}

	static void hook(BranchTrampoline &trampoline) {
		// TODO: Add chainable call instead
		trampoline.write5Call(callAddress, (uintptr_t)MyCtor);
	}

	inline static RelocPtr<decltype(MyCtor)> ctor{ ctorAddress_ };
	inline static RelocAddr<uintptr_t> callAddress{ callAddress_ };
};

struct PlayerFrequentDataModel :
	DataModelHooks<PlayerFrequentDataModel, 0x219A960, 0x219AD2C> {
	static constexpr auto EventName = "PlayerFrequentData"sv;

	char garbage[72];
	UIValue<float> fHealth;
	UIValue<float> fMaxHealth;
	UIValue<float> fStarPower;
	UIValue<float> fMaxStarPower;
	UIValue<float> fHealthGainPct;
	UIValue<float> fHealthBarDamage;
	UIValue<float> fOxygen;
	UIValue<float> fCarbonDioxide;
	UIValue<float> fMaxO2CO2;
	UIValue<uint> uDetectionLevel;
};

struct LocalEnvironmentDataModel {
	// : DataModelHooks<LocalEnvironmentDataModel, 0x2176604, 0x21768BA> {
	static constexpr auto EventName = "LocalEnvironmentData"sv;

	char garbage[72];
	void *localEnvDataVtPtr;
	UIValue<BSFixedStringCS> sBodyName;
	UIValue<uint> uBodyType;
	UIValue<uint> uAlertTimeMs;
	UIValue<float> fGravity;
	UIValue<float> fOxygenPercent;
	UIValue<float> fTemperature;
	UIValue<BSFixedStringCS> sLocationName;
	UIValue<bool> bInSpaceship;
	UIValue<bool> bIsScanning;
	UIValue<bool> bIsLanded;
	UIValue<BSFixedStringCS> sLanguage;
	char garbage2[16];
};
static_assert(sizeof(LocalEnvironmentDataModel) == 56 * 8);

struct LocalEnvFrequentDataModel {
	// : DataModelHooks<LocalEnvFrequentDataModel, 0x21766C0, 0x21768CA> {
	static constexpr auto EventName = "LocalEnvData_Frequent"sv;

	char garbage[72];
	void *localEnvFrequentDataVtPtr;
	UIValue<float> fLocalPlanetTime;
	UIValue<float> fLocalPlanetHoursPerDay;
	UIValue<float> fGalacticStandardTime;
	char garbage2[16];
};
static_assert(sizeof(LocalEnvFrequentDataModel) == 192);

template<typename TValue> struct NestedUIValue {
	char nestedUIValueGarbage[40];
	char dataShuttleContainerMapGarbage[40];
	TValue value;
};

struct EffectDataModel {
	UIValue<BSFixedStringCS> sEffectIcon;
	UIValue<uint> uiHandle;
	UIValue<float> fHeading;
	UIValue<uint> uiMarkerIconType;
};

struct AlertDataModel {
	UIValue<BSFixedStringCS> sEffectIcon;
	UIValue<BSFixedStringCS> sAlertText;
	UIValue<BSFixedStringCS> sAlertSubText;
	UIValue<bool> bIsPositive;
};

template<typename TValue> struct ArrayNestedUIValue {
	char garbage[56];
	NestedUIValue<TValue> *begin;
	NestedUIValue<TValue> *end;
	char garbage2[64];

	inline size_t size() const { return end - begin; }
};
static_assert(sizeof(ArrayNestedUIValue<EffectDataModel>) == 136);

struct PersonalEffectsDataModel {
	// : DataModelHooks<PersonalEffectsDataModel, 0x215D646, 0x215DD2B> {
	static constexpr auto EventName = "PersonalEffectsData"sv;

	char garbage[72];
	UIValue<uint> uAlertTimeMs;
	ArrayNestedUIValue<EffectDataModel> aPersonalEffects;
};
static_assert(sizeof(PersonalEffectsDataModel) == 240);

struct PersonalAlertsDataModel {
	static constexpr auto EventName = "PersonalAlertsData"sv;

	char garbage[72];
	ArrayNestedUIValue<AlertDataModel> aPersonalAlerts;
};
static_assert(sizeof(PersonalAlertsDataModel) == 208);

struct EnvironmentEffectsDataModel {
	static constexpr auto EventName = "EnvironmentEffectsData"sv;

	char garbage[72];
	UIValue<uint> uAlertTimeMS;
	UIValue<uint> uEnvIconPulseMinMS;
	UIValue<uint> uEnvIconPulseMaxMS;
	UIValue<float> fSoakDamagePct;
	UIValue<bool> bShouldPlayAlertAtFullSoak;
	ArrayNestedUIValue<EffectDataModel> aEnvironmentEffects;
};
static_assert(sizeof(EnvironmentEffectsDataModel) == 368);

struct EnvironmentAlertsDataModel {
	static constexpr auto EventName = "EnvironmentAlertsData"sv;

	char garbage[72];
	ArrayNestedUIValue<AlertDataModel> aEnvironmentAlerts;
};
static_assert(sizeof(EnvironmentAlertsDataModel) == 208);

struct HUDDataModel
	: DataModelHooks<HUDDataModel, 0x22650D8, 0x2257E42> {
	char gap0[7160];
	char env_gap0[16];
	LocalEnvironmentDataModel localEnv;
	LocalEnvFrequentDataModel localEnvFrequent;
	char gap7816d[2344];
	char watch_gap0[4176];
	PersonalEffectsDataModel personalEffects;
	PersonalAlertsDataModel personalAlerts;
	EnvironmentEffectsDataModel envEffects;
	EnvironmentAlertsDataModel envAlerts;
};
static_assert(offsetof(HUDDataModel, localEnv) == 7160 + 16);
static_assert(offsetof(HUDDataModel, personalEffects) == 10160 + 4176);
static_assert(offsetof(HUDDataModel, envAlerts) == 10160 + 4992);

static PluginHandle myPluginHandle;
static BranchTrampoline trampoline;

template<typename DataModel> void onFlushDataModel(const char* eventName) {
	if (DataModel::EventName != eventName)
		return;
	Console_Print("Chronomarker: %s", DataModel::EventName.data());
}

struct OnFlushHook {
	static void myInvokeOnFlush(Scaleform::GFx::Value *thiz, const char *methodName, Scaleform::GFx::Value *returnValue, Scaleform::GFx::Value *params, size_t paramCount) {
		(*func)(thiz, methodName, returnValue, params, paramCount);

		for (size_t i = 0; i < paramCount; i++) {
			if (!params[i].IsString())
				continue;
			auto eventType = params[i].GetString();
			onFlushDataModel<PlayerFrequentDataModel>(eventType);
			onFlushDataModel<LocalEnvironmentDataModel>(eventType);
			onFlushDataModel<LocalEnvFrequentDataModel>(eventType);
			onFlushDataModel<PersonalEffectsDataModel>(eventType);
			onFlushDataModel<PersonalAlertsDataModel>(eventType);
			onFlushDataModel<EnvironmentEffectsDataModel>(eventType);
			onFlushDataModel<EnvironmentAlertsDataModel>(eventType);
		}
	}

	inline static RelocAddr<uintptr_t> callAddress{ 0x302E6DF };
	inline static const RelocPtr<decltype(myInvokeOnFlush)> func{ 0x2039AEC };

	static void hook(BranchTrampoline &trampoline) {
		// TODO: also here
		trampoline.write5Call(callAddress, (uintptr_t)myInvokeOnFlush);
	}
};

class TestTask : public SFSETaskInterface::ITaskDelegate {
public:
	time_t lastTime = 0;

	virtual void Run() override {
		time_t curTime;
		time(&curTime);
		if (curTime - lastTime > 3) {
			lastTime = curTime;
			PlayerFrequentDataModel *d = PlayerFrequentDataModel::instance;
			if (d == nullptr)
				Console_Print("Chronomarker: no instance yet");
			else
				Console_Print("Chronomarker: O2 %.2f CO2 %.2f MaxO2CO2 %.2f", d->fOxygen.value, d->fCarbonDioxide.value, d->fMaxO2CO2.value);
		}
	}

	virtual void Destroy() override {

	}
} testTask;

void HandleSFSEMessage(SFSEMessagingInterface::Message *msg) {
	if (msg->type != SFSEMessagingInterface::kMessage_PostLoad)
		return;

	trampoline.create(256);
	PlayerFrequentDataModel::hook(trampoline);
	HUDDataModel::hook(trampoline);
	OnFlushHook::hook(trampoline);
}

extern "C" {
	__declspec(dllexport) SFSEPluginVersionData SFSEPlugin_Version =
	{
		SFSEPluginVersionData::kVersion,

		1,
		"chronomarker-sfse",
		"Helco",

		1,	// address independence
		1,	//  structure independence
		{ RUNTIME_VERSION_1_12_30, 0 },	// compatible with 1.7.23 and that's it

		0,	// works with any version of the script extender. you probably do not need to put anything here
		0, 0,	// set these reserved fields to 0
	};

	__declspec(dllexport) bool SFSEPlugin_Load(const SFSEInterface *sfse)
	{
		DebugLog::openRelative(CSIDL_MYDOCUMENTS, "\\My Games\\" SAVE_FOLDER_NAME "\\SFSE\\Logs\\chronomarker.txt");

		myPluginHandle = sfse->GetPluginHandle();
		auto msgIntf = (SFSEMessagingInterface*)sfse->QueryInterface(kInterface_Messaging);
		msgIntf->RegisterListener(myPluginHandle, "SFSE", HandleSFSEMessage);
		auto taskIntf = (SFSETaskInterface*)sfse->QueryInterface(kInterface_Task);
		//taskIntf->AddTaskPermanent(&testTask);

		return true;
	}
};

