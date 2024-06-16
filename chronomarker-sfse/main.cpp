
#include <ShlObj_core.h>
#include <string_view>
#include <sstream>
#include <vector>
#include <mutex>
#include <thread>
#include <cassert>
#include <zmq.h>
#include <sfse/GameConsole.h>
#include <sfse/GameTypes.h>
#include <sfse/ScaleformValue.h>
#include <sfse/PluginAPI.h>
#include <sfse_common/sfse_version.h>
#include <sfse_common/BranchTrampoline.h>
#include <sfse_common/Log.h>
#include <sfse_common/DataStream.h>

using namespace std::string_view_literals;

static bool shouldSendAllEvents = false;
static std::atomic<uint32_t> dirtyEvents = 0;

class VectorStream final : public DataStream {
public:
	VectorStream(std::vector<byte> &data) : _data(data) {
		_data.clear();
	}

	const byte *data() const { return _data.data(); }

	virtual u64 write(const void *src, u64 len) override {
		_data.resize(_data.size() + len);
		memcpy(_data.data() + _data.size() - len, src, len);
		return _data.size();
	}
	void wstr(const BSFixedStringCS &string) {
		w64(string.size());
		write(string.c_str(), string.size());
	}
	void wstr(const std::string &string) {
		w64(string.length());
		write(string.c_str(), string.length());
	}
	template<typename TValue> void wvec(const std::vector<TValue> &vector) {
		w64(vector.size());
		for (const TValue &value : vector)
			value.serialize(*this);
	}

	virtual u64 seek(u64 offset) override {
		assert(false && "seek is not supported");
		return 0;
	}
	virtual u64 read(void *dst, u64 len) override {
		assert(false && "read is not supported");
		return 0;
	}

private:
	std::vector<byte> &_data;
};

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
	inline static Me *_instance = nullptr;

	static Me *MyCtor(Me *thiz) {
		_instance = thiz;
		shouldSendAllEvents = true;
		return (*ctor)(thiz);
	}

	static void hook(BranchTrampoline &trampoline) {
		// TODO: Add chainable call instead
		trampoline.write5Call(callAddress, (uintptr_t)MyCtor);
	}

	static Me *instance() { return _instance; }

	inline static RelocPtr<decltype(MyCtor)> ctor{ ctorAddress_ };
	inline static RelocAddr<uintptr_t> callAddress{ callAddress_ };
};

enum class EventType : byte {
	PlayerFrequent = 0,
	LocalEnvironment,
	LocalEnvFrequent,
	PersonalEffects,
	EnvironmentEffects,
	Alerts
};

struct PlayerFrequentDataModel :
	DataModelHooks<PlayerFrequentDataModel, 0x219A9E0, 0x219ADAC> {
	static constexpr auto EventName = "PlayerFrequentData"sv;
	static constexpr auto EventType = EventType::PlayerFrequent;

	void serialize(VectorStream &stream) {
		stream.w8((byte)EventType);
		stream.wf32(fHealth.value);
		stream.wf32(fHealth.value);
		stream.wf32(fMaxHealth.value);
		stream.wf32(fMaxStarPower.value);
		stream.wf32(fHealthGainPct.value);
		stream.wf32(fHealthBarDamage.value);
		stream.wf32(fOxygen.value);
		stream.wf32(fCarbonDioxide.value);
		stream.wf32(fMaxO2CO2.value);
		stream.w32(uDetectionLevel.value);
	}

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
	static constexpr auto EventType = EventType::LocalEnvironment;
	static LocalEnvironmentDataModel *instance();

	void serialize(VectorStream &stream) {
		stream.w8((byte)EventType);
		stream.wstr(sBodyName.value);
		stream.w32(uBodyType.value);
		stream.w32(uAlertTimeMs.value);
		stream.wf32(fGravity.value);
		stream.wf32(fOxygenPercent.value);
		stream.wf32(fTemperature.value);
		stream.wstr(sLocationName.value);
		stream.w8(bInSpaceship.value);
		stream.w8(bIsScanning.value);
		stream.w8(bIsLanded.value);
		stream.wstr(sLanguage.value);
	}

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
	static constexpr auto EventType = EventType::LocalEnvFrequent;
	static LocalEnvFrequentDataModel *instance();

	void serialize(VectorStream &stream) {
		stream.w8((byte)EventType);
		stream.wf32(fLocalPlanetTime.value);
		stream.wf32(fLocalPlanetHoursPerDay.value);
		stream.wf32(fGalacticStandardTime.value);
	}

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

template<typename TValue> struct ArrayNestedUIValue {
	void serialize(VectorStream &stream) {
		stream.w64(size());
		for (auto it = begin; it != end; it++)
			it->value.serialize(stream);
	}

	char garbage[56];
	NestedUIValue<TValue> *begin;
	NestedUIValue<TValue> *end;
	char garbage2[64];

	inline size_t size() const { return end - begin; }
};

struct EffectDataModel {
	void serialize(VectorStream &stream) {
		stream.wstr(sEffectIcon.value);
		stream.w32(uiHandle.value);
		stream.wf32(fHeading.value);
		stream.w32(uiMarkerIconType.value);
	}

	UIValue<BSFixedStringCS> sEffectIcon;
	UIValue<uint> uiHandle;
	UIValue<float> fHeading;
	UIValue<uint> uiMarkerIconType;
	char garbage[32];
};
static_assert(sizeof(NestedUIValue<EffectDataModel>) == 240);
static_assert(sizeof(ArrayNestedUIValue<EffectDataModel>) == 136);

struct AlertDataModel {
	void serialize(VectorStream &stream) {
		stream.wstr(sEffectIcon.value);
		stream.wstr(sAlertText.value);
		stream.wstr(sAlertSubText.value);
		stream.w8(bIsPositive.value);
	}

	UIValue<BSFixedStringCS> sEffectIcon;
	UIValue<BSFixedStringCS> sAlertText;
	UIValue<BSFixedStringCS> sAlertSubText;
	UIValue<bool> bIsPositive;
	char garbage[8];
};
static_assert(sizeof(NestedUIValue<AlertDataModel>) == 216);

struct PersonalEffectsDataModel {
	// : DataModelHooks<PersonalEffectsDataModel, 0x215D646, 0x215DD2B> {
	static constexpr auto EventName = "PersonalEffectsData"sv;
	static constexpr auto EventType = EventType::PersonalEffects;
	static PersonalEffectsDataModel *instance();

	void serialize(VectorStream &stream) {
		stream.w8((byte)EventType);
		stream.w32(uAlertTimeMs.value);
		aPersonalEffects.serialize(stream);
	}

	char garbage[72];
	UIValue<uint> uAlertTimeMs;
	ArrayNestedUIValue<EffectDataModel> aPersonalEffects;
};
static_assert(sizeof(PersonalEffectsDataModel) == 240);

// Alerts are weird, the data is only temporarily stored in the ArrayNestedUIValue, so we take a different route altogether

struct PersonalAlertsDataModel {
	//static constexpr auto EventName = "PersonalAlertsData"sv;
	//static constexpr auto EventType = EventType::PersonalAlerts;
	static PersonalAlertsDataModel *instance();

	void serialize(VectorStream &stream) {
		//stream.w8((byte)EventType);
		aPersonalAlerts.serialize(stream);
		OutputDebugStringA("EnvironmentAlerts");
	}

	char garbage[72];
	ArrayNestedUIValue<AlertDataModel> aPersonalAlerts;
};
static_assert(sizeof(PersonalAlertsDataModel) == 208);

struct EnvironmentEffectsDataModel {
	static constexpr auto EventName = "EnvironmentEffectsData"sv;
	static constexpr auto EventType = EventType::EnvironmentEffects;
	static EnvironmentEffectsDataModel *instance();

	void serialize(VectorStream &stream) {
		stream.w8((byte)EventType);
		stream.w32(uAlertTimeMS.value);
		stream.w32(uEnvIconPulseMinMS.value);
		stream.w32(uEnvIconPulseMaxMS.value);
		stream.wf32(fSoakDamagePct.value);
		stream.w8(bShouldPlayAlertAtFullSoak.value);
		aEnvironmentEffects.serialize(stream);
	}

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
	//static constexpr auto EventName = "EnvironmentAlertsData"sv;
	//static constexpr auto EventType = EventType::EnvironmentAlerts;
	static EnvironmentAlertsDataModel *instance();

	void serialize(VectorStream &stream) {
		//stream.w8((byte)EventType);
		aEnvironmentAlerts.serialize(stream);
		OutputDebugStringA("EnvironmentAlerts");
	}

	char garbage[72];
	ArrayNestedUIValue<AlertDataModel> aEnvironmentAlerts;
};
static_assert(sizeof(EnvironmentAlertsDataModel) == 208);

struct HUDDataModel
	: DataModelHooks<HUDDataModel, 0x2265158, 0x2257EC2> {
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

#define HUDMODEL_INSTANCE(type,member) type *type::instance() { auto b = HUDDataModel::instance(); return b == nullptr ? nullptr : &b->member; }
HUDMODEL_INSTANCE(LocalEnvironmentDataModel, localEnv);
HUDMODEL_INSTANCE(LocalEnvFrequentDataModel, localEnvFrequent);
HUDMODEL_INSTANCE(PersonalEffectsDataModel, personalEffects);
HUDMODEL_INSTANCE(PersonalAlertsDataModel, personalAlerts);
HUDMODEL_INSTANCE(EnvironmentEffectsDataModel, envEffects);
HUDMODEL_INSTANCE(EnvironmentAlertsDataModel, envAlerts);

static PluginHandle myPluginHandle;
static BranchTrampoline trampoline;
static std::vector<byte> messageBuffer;
static void *zmqContext = nullptr;
static void *zmqSocket = nullptr;
static void *zmqMonitorSocket = nullptr;

struct MyAlert {
	std::string sEffectIcon, sAlertText, sAlertSubText;
	bool bIsPositive;

	void serialize(VectorStream &stream) const
	{
		stream.wstr(sEffectIcon);
		stream.wstr(sAlertText);
		stream.wstr(sAlertSubText);
		stream.w8(bIsPositive);
	}
};

struct OnInitAlert {
	static void *myInitAlert(byte* alertPtr) {
		auto *alert = (AlertDataModel *)(alertPtr + 40);
		MyAlert myAlert;
		myAlert.sEffectIcon = alert->sEffectIcon.value.c_str();
		myAlert.sAlertText = alert->sAlertText.value.c_str();
		myAlert.sAlertSubText = alert->sAlertSubText.value.c_str();
		myAlert.bIsPositive = alert->bIsPositive.value;
		{
			std::lock_guard _{ alertsMutex };
			alerts.push_back(std::move(myAlert));
		}
		dirtyEvents |= 1u << (byte)EventType::Alerts;

		/*std::stringstream ss;
		ss << "Alert: " << alert->sEffectIcon.value.c_str() << " (" << alert->sAlertText.value.c_str() << ")" << " (" << alert->sAlertSubText.value.c_str() << ") " << alert->bIsPositive.value;
		auto str = ss.str();
		OutputDebugStringA(str.c_str());*/
		return (*func)(alertPtr);
	}

	inline static RelocAddr<uintptr_t> callAddress1{ 0x215CF3D };
	inline static RelocAddr<uintptr_t> callAddress2{ 0x215D2A2 };
	inline static RelocAddr<uintptr_t> callAddress3{ 0x215EFAE };
	inline static const RelocPtr<decltype(myInitAlert)> func{ 0x21604D4 };
	inline static std::vector<MyAlert> alerts;
	inline static std::mutex alertsMutex;

	static void hook(BranchTrampoline &trampoline) {
		// TODO: also here
		trampoline.write5Call(callAddress1, (uintptr_t)myInitAlert);
		trampoline.write5Call(callAddress2, (uintptr_t)myInitAlert);
		trampoline.write5Call(callAddress3, (uintptr_t)myInitAlert);
	}

	static void serialize(VectorStream &stream) {
		std::lock_guard _{ alertsMutex };
		if (alerts.size() == 0)
			return;
		stream.w8((byte)EventType::Alerts);
		stream.wvec(alerts);
		alerts.clear();
	}
};

template<typename DataModel> void sendDataModel(uint32_t eventBits) {
	if ((eventBits & (1u << (byte)DataModel::EventType)) == 0)
		return;
	auto instance = DataModel::instance();
	VectorStream messageStream(messageBuffer);
	if (instance != nullptr)
		instance->serialize(messageStream);
	if (messageBuffer.size() == 0)
		return;
	zmq_send(zmqSocket, messageBuffer.data(), messageBuffer.size(), ZMQ_NOBLOCK);
}

void sendAlertsDataModel(uint32_t eventBits) {
	if ((eventBits & (1u << (byte)EventType::Alerts)) == 0)
		return;
	VectorStream messageStream(messageBuffer);
	OnInitAlert::serialize(messageStream);
	if (messageBuffer.size() == 0)
		return;
	zmq_send(zmqSocket, messageBuffer.data(), messageBuffer.size(), ZMQ_NOBLOCK);
}

template<typename DataModel> void onFlushDataModel(const char* eventName) {
	if (DataModel::EventName != eventName)
		return;
	dirtyEvents |= 1u << (byte)DataModel::EventType;
}

struct OnFlushHook {
	inline static bool wasCalledAtSomePoint = false;

	static void myInvokeOnFlush(Scaleform::GFx::Value *thiz, const char *methodName, Scaleform::GFx::Value *returnValue, Scaleform::GFx::Value *params, size_t paramCount) {
		(*func)(thiz, methodName, returnValue, params, paramCount);
		wasCalledAtSomePoint = true;

		for (size_t i = 0; i < paramCount; i++) {
			if (!params[i].IsString())
				continue;
			auto eventType = params[i].GetString();
			onFlushDataModel<PlayerFrequentDataModel>(eventType);
			onFlushDataModel<LocalEnvironmentDataModel>(eventType);
			onFlushDataModel<LocalEnvFrequentDataModel>(eventType);
			onFlushDataModel<PersonalEffectsDataModel>(eventType);
			onFlushDataModel<EnvironmentEffectsDataModel>(eventType);
		}
	}

	inline static RelocAddr<uintptr_t> callAddress{ 0x302F6DF };
	inline static const RelocPtr<decltype(myInvokeOnFlush)> func{ 0x2039ABC };

	static void hook(BranchTrampoline &trampoline) {
		// TODO: also here
		trampoline.write5Call(callAddress, (uintptr_t)myInvokeOnFlush);
	}
};

void HandleSFSEMessage(SFSEMessagingInterface::Message *msg) {
	if (msg->type != SFSEMessagingInterface::kMessage_PostLoad)
		return;

	messageBuffer.reserve(1024);

	zmqContext = zmq_ctx_new();
	zmqSocket = zmq_socket(zmqContext, ZMQ_PUB);
	const int lingerValue = 0;
	zmq_setsockopt(zmqSocket, ZMQ_LINGER, &lingerValue, sizeof(int));
	zmq_bind(zmqSocket, "tcp://127.0.0.1:7201");
	zmq_socket_monitor(zmqSocket, "inproc://monitor", ZMQ_EVENT_ACCEPTED);
	zmqMonitorSocket = zmq_socket(zmqContext, ZMQ_PAIR);
	zmq_connect(zmqMonitorSocket, "inproc://monitor");

	trampoline.create(256);
	PlayerFrequentDataModel::hook(trampoline);
	HUDDataModel::hook(trampoline);
	OnFlushHook::hook(trampoline);
	OnInitAlert::hook(trampoline);
}

class SendAllEventsTask : public SFSETaskInterface::ITaskDelegate
{
public:
	virtual void Run() override {
		if (zmqMonitorSocket == nullptr || !OnFlushHook::wasCalledAtSomePoint)
			return;

		const size_t MONITOR_MESSAGE_SIZE = 6;
		byte dummyBuffer[MONITOR_MESSAGE_SIZE];
		uint32_t eventBits = dirtyEvents.exchange(0);
		while (zmq_recv(zmqMonitorSocket, dummyBuffer, MONITOR_MESSAGE_SIZE, ZMQ_DONTWAIT) > 0)
			shouldSendAllEvents = true;

		if (shouldSendAllEvents) {
			Console_Print("Chronomarker: Send all data");
			eventBits = ~0u;
			shouldSendAllEvents = false;
		}

		sendDataModel<PlayerFrequentDataModel>(eventBits);
		sendDataModel<LocalEnvironmentDataModel>(eventBits);
		sendDataModel<LocalEnvFrequentDataModel>(eventBits);
		sendDataModel<PersonalEffectsDataModel>(eventBits);
		sendDataModel<EnvironmentEffectsDataModel>(eventBits);
		sendAlertsDataModel(eventBits);
	}

	virtual void Destroy() override {
		delete this;
	}
};

extern "C" {
	__declspec(dllexport) SFSEPluginVersionData SFSEPlugin_Version =
	{
		SFSEPluginVersionData::kVersion,

		1,
		"chronomarker-sfse",
		"Helco",

		1,	// address independence
		1,	//  structure independence
		{ RUNTIME_VERSION_1_12_32, 0 },	// compatible with 1.7.23 and that's it

		0,	// works with any version of the script extender. you probably do not need to put anything here
		0, 0,	// set these reserved fields to 0
	};

	__declspec(dllexport) bool SFSEPlugin_Load(const SFSEInterface *sfse)
	{
		DebugLog::openRelative(CSIDL_MYDOCUMENTS, "\\My Games\\" SAVE_FOLDER_NAME "\\SFSE\\Logs\\chronomarker.txt");

		myPluginHandle = sfse->GetPluginHandle();
		auto msgIntf = (SFSEMessagingInterface*)sfse->QueryInterface(kInterface_Messaging);
		msgIntf->RegisterListener(myPluginHandle, "SFSE", HandleSFSEMessage);
		auto taskIntf = (SFSETaskInterface *)sfse->QueryInterface(kInterface_Task);
		taskIntf->AddTaskPermanent(new SendAllEventsTask());

		return true;
	}
};

