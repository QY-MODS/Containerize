#pragma once
#include "Hooks.h"

// Thanks and credits to Bloc: https://discord.com/channels/874895328938172446/945560222670393406/1093262407989731338
class ConversationCallbackFunctor final : public RE::BSScript::IStackCallbackFunctor {



    std::string rename;
	Manager* M;

    void operator()(RE::BSScript::Variable a_result) override {
        if (a_result.IsNoneObject()) {
            logger::trace("Result: None");
        } else if (a_result.IsString()) {
            rename = a_result.GetString();
            logger::trace("Result: {}", rename);
            if (!rename.empty()) {
				M->RenameContainer(rename);
			}
        }
    }

    void SetObject(const RE::BSTSmartPointer<RE::BSScript::Object>&) override {};

public:
    ConversationCallbackFunctor(Manager* mngr) : M(mngr) {}
};

class OurEventSink : public RE::BSTEventSink<RE::TESEquipEvent>,
                     public RE::BSTEventSink<RE::TESActivateEvent>,
                     public RE::BSTEventSink<SKSE::CrosshairRefEvent>,
                     public RE::BSTEventSink<RE::MenuOpenCloseEvent>,
                     public RE::BSTEventSink<RE::TESFurnitureEvent>,
                     public RE::BSTEventSink<RE::TESContainerChangedEvent>,
                     public RE::BSTEventSink<RE::InputEvent*>,
                     public RE::BSTEventSink<RE::TESFormDeleteEvent> {

    OurEventSink() = default;
    OurEventSink(const OurEventSink&) = delete;
    OurEventSink(OurEventSink&&) = delete;
    OurEventSink& operator=(const OurEventSink&) = delete;
    OurEventSink& operator=(OurEventSink&&) = delete;


	std::atomic<bool> block_droptake = false;


    FormID fake_equipped_id = 0;
	FormID fake_id_ = 0;

	bool equipped = false;

    std::string ReShowMenu;

    RE::NiPointer<RE::TESObjectREFR> furniture;

public:

	std::atomic<bool> block_eventsinks = false;
	std::atomic<bool> listen_crosshair_ref = true;
	std::atomic<bool> furniture_entered = false;
	std::atomic<bool> listen_weight_limit = false;
    Manager* M = nullptr;
    RefID external_container_refid = 0;  // set in input event

    OurEventSink(Manager* mngr)
        :  M(mngr){}

    static OurEventSink* GetSingleton(Manager* manager) {
        static OurEventSink singleton(manager);
        return &singleton;
    }


    RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* event, RE::BSTEventSource<RE::TESEquipEvent>*) override;

    // Prompts Messagebox
    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event,
                                          RE::BSTEventSource<RE::TESActivateEvent>*) override;

    // to disable ref activation and external container-fake container placement
    RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* event,
                                          RE::BSTEventSource<SKSE::CrosshairRefEvent>*) override;

    // to close chest and save the contents and remove items
    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESFurnitureEvent* event,
                                          RE::BSTEventSource<RE::TESFurnitureEvent>*) {
        
        if (block_eventsinks.load()) return RE::BSEventNotifyControl::kContinue;
        if (!event) return RE::BSEventNotifyControl::kContinue;
        if (!event->actor->IsPlayerRef()) return RE::BSEventNotifyControl::kContinue;
        if (furniture_entered && event->type == RE::TESFurnitureEvent::FurnitureEventType::kEnter)
            return RE::BSEventNotifyControl::kContinue;
        if (!furniture_entered && event->type == RE::TESFurnitureEvent::FurnitureEventType::kExit)
			return RE::BSEventNotifyControl::kContinue;
        if (event->targetFurniture->GetBaseObject()->formType.underlying() != 40) return RE::BSEventNotifyControl::kContinue;

        logger::trace("Furniture event");

        const auto bench = event->targetFurniture->GetBaseObject()->As<RE::TESFurniture>();
        if (!bench) return RE::BSEventNotifyControl::kContinue;
        if (const auto bench_type = static_cast<std::uint8_t>(bench->workBenchData.benchType.get()); bench_type != 2 && bench_type != 3 && bench_type != 7) return RE::BSEventNotifyControl::kContinue;

        if (event->type == RE::TESFurnitureEvent::FurnitureEventType::kEnter) {
            logger::trace("Furniture event: Enter {}", event->targetFurniture->GetName());
            furniture_entered = true;
            furniture = event->targetFurniture;
            //M->HandleCraftingEnter();
        }
        else if (event->type == RE::TESFurnitureEvent::FurnitureEventType::kExit) {
            logger::trace("Furniture event: Exit {}", event->targetFurniture->GetName());
            if (event->targetFurniture == furniture) {
                M->HandleCraftingExit();
                furniture_entered = false;
                furniture = nullptr;
            }
        }
        else {
			logger::trace("Furniture event: Unknown");
		}


		return RE::BSEventNotifyControl::kContinue;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::TESContainerChangedEvent* event,
                                                                   RE::BSTEventSource<RE::TESContainerChangedEvent>*) override;

    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* evns, RE::BSTEventSource<RE::InputEvent*>*) override;

    RE::BSEventNotifyControl ProcessEvent(const RE::TESFormDeleteEvent* a_event,
                                          RE::BSTEventSource<RE::TESFormDeleteEvent>*) override;
};