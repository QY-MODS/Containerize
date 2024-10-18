#include "MCP.h"

Manager* M = nullptr;
OurEventSink* eventSink;
bool eventsinks_added = false;

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        // Start
        const auto sources = LoadSources();
        if (sources.empty()) {
            logger::critical("Failed to load INI sources.");
            return;
        }
        M = Manager::GetSingleton(sources);
		if (!M) {
			logger::critical("Failed to load Manager.");
			return;
		}
        eventSink = OurEventSink::GetSingleton(M);
        UI::Register(M);
        logger::info("MCP registered.");
    }
    if (message->type == SKSE::MessagingInterface::kPostLoadGame ||
        message->type == SKSE::MessagingInterface::kNewGame) {
        if (eventsinks_added) return;
        if (!M || !eventSink) return;
        // EventSink
        auto* eventSourceHolder = RE::ScriptEventSourceHolder::GetSingleton();
        eventSourceHolder->AddEventSink<RE::TESEquipEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESActivateEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESContainerChangedEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESFurnitureEvent>(eventSink);
        eventSourceHolder->AddEventSink<RE::TESFormDeleteEvent>(eventSink);
        RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(eventSink);
        RE::BSInputDeviceManager::GetSingleton()->AddEventSink(eventSink);
        SKSE::GetCrosshairRefEventSource()->AddEventSink(eventSink);
        eventsinks_added = true;
    }
}



#define DISABLE_IF_UNINSTALLED if (!M || M->isUninstalled) return;
void SaveCallback(SKSE::SerializationInterface* serializationInterface) {
    DISABLE_IF_UNINSTALLED 
    logger::trace("Saving Data to skse co-save.");
	eventSink->SetBlockSinks(true);
    M->SendData();
    if (!M->Save(serializationInterface, Settings::kDataKey, Settings::kSerializationVersion)) {
        logger::critical("Failed to save Data");
    }
	auto* DFT = DynamicFormTracker::GetSingleton();
    DFT->SendData();
    if (!DFT->Save(serializationInterface, Settings::kDFDataKey, Settings::kSerializationVersion)) {
        logger::critical("Failed to save Data");
    }
    logger::trace("Data saved to skse co-save.");
	eventSink->SetBlockSinks(false);
}

void LoadCallback(SKSE::SerializationInterface* serializationInterface) {
    DISABLE_IF_UNINSTALLED
    
    logger::info("Loading Data from skse co-save.");
    
    eventSink->Reset();
    M->Reset();
    auto* DFT = DynamicFormTracker::GetSingleton();
    DFT->Reset();

    std::uint32_t type;
    std::uint32_t version;
    std::uint32_t length;

    eventSink->SetBlockSinks(true);

    while (serializationInterface->GetNextRecordInfo(type, version, length)) {
        bool is_before_0_7 = false;
        
        auto temp = DecodeTypeCode(type);

        if (version == Settings::kSerializationVersion-2) {
            logger::warn("Loading data is from an older version < v0.7. Recieved ({}) - Expected ({}) for Data Key ({})",
							 version, Settings::kSerializationVersion, temp);
			is_before_0_7= true;
            std::string err_message =
                "It seems you haven't followed the latest update instructions for the mod correctly. "
                "Please refer to the mod page for the latest instructions. "
                "In case of a failure you will see an error message box displayed after this one. If not, you are probably fine.";
            MsgBoxesNotifs::InGame::CustomMsg(err_message);
            Settings::is_pre_0_7_1 = true;
        } else if (version == Settings::kSerializationVersion - 1) {
			logger::warn("Loading data is from an older version < v0.7.1. Recieved ({}) - Expected ({}) for Data Key ({})",
							 version, Settings::kSerializationVersion, temp);
            Settings::is_pre_0_7_1 = true;
        }
        else if (version != Settings::kSerializationVersion) {
            logger::critical("Loaded data has incorrect version. Recieved ({}) - Expected ({}) for Data Key ({})",
                             version, Settings::kSerializationVersion, temp);
            continue;
        }
        switch (type) {
            case Settings::kDataKey: {
                logger::trace("Loading Record: {} - Version: {} - Length: {}", temp, version, length);
                if (!M->Load(serializationInterface, is_before_0_7)) {
                    logger::critical("Failed to Load Data");
                    return MsgBoxesNotifs::InGame::CustomMsg("Failed to Load Data.");
                }
            } break;
            case Settings::kDFDataKey: {
                logger::trace("Loading Record: {} - Version: {} - Length: {}", temp, version, length);
                if (!DFT->Load(serializationInterface, is_before_0_7)) logger::critical("Failed to Load Data for DFT");
            } break;
            default:
                logger::critical("Unrecognized Record Type: {}", temp);
                break;
        }
    }

    logger::info("Receiving Data.");
    DFT->ReceiveData();
    SKSE::GetTaskInterface()->AddTask([]() { 
        M->ReceiveData(); 
        logger::info("Data loaded from skse co-save.");
	    eventSink->SetBlockSinks(false);
        }
    );
}
#undef DISABLE_IF_UNINSTALLED

void InitializeSerialization() {
    auto* serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(Settings::kDataKey);
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
    SKSE::log::trace("Cosave serialization initialized.");
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SetupLog();
    LoadOtherSettings();
    SKSE::Init(skse);
    InitializeSerialization();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}