#include "sfse/PluginAPI.h"
#include "sfse_common/sfse_version.h"
#include <ctime>
#include <sfse/GameConsole.h>
#include <sfse_common/Log.h>
#include <sfse_common/BranchTrampoline.h>
#include <ShlObj_core.h>

template<typename TValue> struct UIValue {
	void *vtptr;
	void *unk1, *unk2;
	TValue value;
};
static_assert(sizeof(UIValue<float>) == 32);
static_assert(sizeof(UIValue<double>) == 32);
static_assert(sizeof(UIValue<unsigned int>) == 32);

struct PlayerFrequentDataModel {
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

	inline static PlayerFrequentDataModel *instance = nullptr;

	static PlayerFrequentDataModel *MyCtor(PlayerFrequentDataModel *thiz) {
		Console_Print("ATTACHED");
		instance = thiz;
		return (*ctor)(thiz);
	}

	inline static RelocPtr<decltype(MyCtor)> ctor{0x219A960};
	inline static RelocAddr<uintptr_t> callAddress{ 0x219AD2C };
};

static PluginHandle myPluginHandle;
static BranchTrampoline trampoline;

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
	trampoline.write5Call(PlayerFrequentDataModel::callAddress, (uintptr_t)PlayerFrequentDataModel::MyCtor);
}

extern "C" {
	__declspec(dllexport) SFSEPluginVersionData SFSEPlugin_Version =
	{
		SFSEPluginVersionData::kVersion,

		1,
		"chronomarker-sfse",
		"Helco",

		0,	// not address independent
		0,	// not structure independent
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
		taskIntf->AddTaskPermanent(&testTask);

		return true;
	}
};

