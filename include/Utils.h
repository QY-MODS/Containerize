#pragma once
#include <windows.h>
#include "ClibUtil/editorID.hpp"
#include "SimpleIni.h"
#include <unordered_set>


const auto mod_name = static_cast<std::string>(SKSE::PluginDeclaration::GetSingleton()->GetName());
constexpr auto po3path = "Data/SKSE/Plugins/po3_Tweaks.dll";
constexpr auto po3_UoTpath = "Data/SKSE/Plugins/po3_UseOrTake.dll";
inline bool IsPo3Installed() { return std::filesystem::exists(po3path); };
inline bool IsPo3_UoTInstalled() { return std::filesystem::exists(po3_UoTpath); };
const auto po3_use_or_take = IsPo3_UoTInstalled();


const auto no_src_msgbox = std::format(
    "{}: You currently do not have any container set up. Check your ini file or see the mod page for instructions.",
    mod_name);

const auto po3_err_msgbox = std::format(
    "{}: You have given an invalid FormID. If you are using Editor IDs, you must have powerofthree's Tweaks "
    "installed. See mod page for further instructions.",
    mod_name);
const auto general_err_msgbox = std::format("{}: Something went wrong. Please contact the mod author.", mod_name);
const auto init_err_msgbox = std::format("{}: The mod failed to initialize and will be terminated.", mod_name);


void SetupLog();
std::filesystem::path GetLogPath();
std::vector<std::string> ReadLogFile();

std::string DecodeTypeCode(std::uint32_t typeCode);

inline bool isValidHexWithLength7or8(const char* input);

template <class T = RE::TESForm>
static T* GetFormByID(const RE::FormID id, const std::string& editor_id="") {
    if (!editor_id.empty()) {
        if (auto* form = RE::TESForm::LookupByEditorID<T>(editor_id)) return form;
    }
    if (T* form = RE::TESForm::LookupByID<T>(id)) return form;
    return nullptr;
};

std::string GetEditorID(const FormID a_formid);
FormID GetFormEditorIDFromString(const std::string formEditorId);

namespace Functions {

    template <typename Key, typename Value>
    bool containsValue(const std::map<Key, Value>& myMap, const Value& valueToFind) {
        for (const auto& pair : myMap) {
            if (pair.second == valueToFind) {
                return true;
            }
        }
        return false;
    }

    template <typename Key, typename Value>
    void printMap(const std::map<Key, Value>& myMap) {
        for (const auto& pair : myMap) {
			logger::trace("Key: {}, Value: {}", pair.first, pair.second);
		}
	}
}

namespace Math {

    /*float Round(float value, int n);
    float Ceil(float value, int n);*/

    namespace LinAlg {
        namespace R3 {
            void rotateX(RE::NiPoint3& v, float angle);

            // Function to rotate a vector around the y-axis
            void rotateY(RE::NiPoint3& v, float angle);

            // Function to rotate a vector around the z-axis
            void rotateZ(RE::NiPoint3& v, float angle);

            void rotate(RE::NiPoint3& v, float angleX, float angleY, float angleZ);
        };
    };
};

namespace String {
    inline std::string trim(const std::string& str);

    inline std::string toLowercase(const std::string& str);

    inline std::string replaceLineBreaksWithSpace(const std::string& input);

    bool includesWord(const std::string& input, const std::vector<std::string>& strings);
}

namespace FunctionsSkyrim {

    inline std::size_t GetExtraDataListLength(const RE::ExtraDataList* dataList);


    template <typename T>
    struct FormTraits {
        static float GetWeight(T* form) {
            // Default implementation, assuming T has a member variable 'weight'
            return form->weight;
        }

        static void SetWeight(T* form, float weight) {
            // Default implementation, set the weight if T has a member variable 'weight'
            form->weight = weight;
        }

        static int GetValue(T* form) {
			// Default implementation, assuming T has a member variable 'value'
			return form->value;
		}

        static void SetValue(T* form, int value) {
            form->value = value;
        }
    };

    // Specialization for TESAmmo
    template <>
    struct FormTraits<RE::TESAmmo> {
        static float GetWeight(RE::TESAmmo*) {
            // Handle TESAmmo case where 'weight' is not a member
            // You might return a default value or calculate it based on other factors
            return 0.0f;  // For example, returning 0 as a default value
        }

        static void SetWeight(RE::TESAmmo*, float) {
            // Handle setting the weight for TESAmmo
            // (implementation based on your requirements)
            // For example, if TESAmmo had a SetWeight method, you would call it here
        }

        static int GetValue(RE::TESAmmo* form) {
			return form->value;
		}
        static void SetValue(RE::TESAmmo* form, int value) {
			form->value = value;
		}
    };

    template <>
    struct FormTraits<RE::AlchemyItem> {
        static float GetWeight(RE::AlchemyItem* form) { 
            return form->weight;
        }

        static void SetWeight(RE::AlchemyItem* form, float weight) { 
            form->weight = weight;
        }

        static int GetValue(RE::AlchemyItem* form) {
        	return form->GetGoldValue();
        }
        static void SetValue(RE::AlchemyItem* form, int value) { 
            logger::trace("CostOverride: {}", form->data.costOverride);
            form->data.costOverride = value;
        }
    };

}


namespace MsgBoxesNotifs {

    // https://github.com/SkyrimScripting/MessageBox/blob/ac0ea32af02766582209e784689eb0dd7d731d57/include/SkyrimScripting/MessageBox.h#L9
    class SkyrimMessageBox {
        class MessageBoxResultCallback : public RE::IMessageBoxCallback {
            std::function<void(unsigned int)> _callback;

        public:
            ~MessageBoxResultCallback() override {}
            MessageBoxResultCallback(std::function<void(unsigned int)> callback) : _callback(callback) {}
            void Run(RE::IMessageBoxCallback::Message message) override {
                _callback(static_cast<unsigned int>(message));
            }
        };

    public:
        static void Show(const std::string& bodyText, std::vector<std::string> buttonTextValues,
                         std::function<void(unsigned int)> callback);
    };

    inline void ShowMessageBox(const std::string& bodyText, const std::vector<std::string>& buttonTextValues,
                               const std::function<void(unsigned int)>& callback) {
        SkyrimMessageBox::Show(bodyText, buttonTextValues, callback);
    }

    namespace Windows {

        inline int Po3ErrMsg() {
            MessageBoxA(nullptr, po3_err_msgbox.c_str(), "Error", MB_OK | MB_ICONERROR);
            return 1;
        };
    };

    namespace InGame{
        inline void CustomMsg(const std::string& msg) { RE::DebugMessageBox((mod_name + ": " + msg).c_str()); };
        inline void GeneralErr() { RE::DebugMessageBox(general_err_msgbox.c_str()); };
        inline void InitErr() { RE::DebugMessageBox(init_err_msgbox.c_str()); };

		inline void FormTypeErr(RE::FormID id) {
			RE::DebugMessageBox(
				std::format("{}: The form type of the item with FormID ({:x}) is not supported. Please contact the mod author.",
					mod_name, id).c_str());
        };

		inline void UninstallSuccessful() {
			RE::DebugMessageBox(
				std::format("{}: Uninstall successful. You can now safely remove the mod.", mod_name).c_str());
        }

        inline void UninstallFailed() {
			RE::DebugMessageBox(
				std::format("{}: Uninstall failed. Please contact the mod author.", mod_name).c_str());
        }

        inline void ProblemWithContainer(std::string id) {
                RE::DebugMessageBox(
					std::format("{}: Problem with one of the items with the form id ({}). This is expected if you have changed the list of containers in the INI file between saves. Corresponding items will be returned to your inventory. You can suppress this message by changing the setting in your INI.",
                        								mod_name, id)
						.c_str());
            };

    };
    
};

namespace Inventory {
    bool EntryHasXData(const RE::InventoryEntryData* entry);

    inline bool HasItemEntry(RE::TESBoundObject* item, const RE::TESObjectREFR::InventoryItemMap& inventory,
                             bool nonzero_entry_check = false);

    inline bool HasItem(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner) {
        if (HasItemEntry(item, inventory_owner->GetInventory(), true)) return true;
        return false;
    };

    std::int32_t GetItemCount(RE::TESBoundObject* item, const RE::TESObjectREFR::InventoryItemMap& inventory);

    std::int32_t GetItemValue(RE::TESBoundObject* item,
                              const RE::TESObjectREFR::InventoryItemMap& inventory);

    bool IsQuestItem(const FormID formid, RE::TESObjectREFR* inv_owner);

    inline int32_t GetEntryCostOverride(const RE::InventoryEntryData* entry);

    int GetValueInContainer(RE::TESObjectREFR* container);

    void FavoriteItem(const RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner);

    bool IsFavorited(RE::TESBoundObject* item, RE::TESObjectREFR* inventory_owner);

    void EquipItem(const RE::TESBoundObject* item, bool unequip = false);

    inline void EquipItem(const FormID formid, bool unequip = false) {
	    EquipItem(GetFormByID<RE::TESBoundObject>(formid), unequip);
    }

    [[nodiscard]] bool IsEquipped(RE::TESBoundObject* item);

    [[nodiscard]] inline bool IsEquipped(const FormID formid) {
	    return IsEquipped(GetFormByID<RE::TESBoundObject>(formid));
    }

};


namespace WorldObject {
    /*int16_t GetObjectCount(RE::TESObjectREFR* ref);

    void SetObjectCount(RE::TESObjectREFR* ref, Count count);*/

    RE::TESObjectREFR* DropObjectIntoTheWorld(RE::TESBoundObject* obj, Count count=1, bool player_owned=true);

    void SwapObjects(RE::TESObjectREFR* a_from, RE::TESBoundObject* a_to, bool apply_havok=true);

	//float GetDistanceFromPlayer(const RE::TESObjectREFR* ref);

 //   [[nodiscard]] bool PlayerPickUpObject(RE::TESObjectREFR* item, Count count, unsigned int max_try = 3);

 //   RefID TryToGetRefIDFromHandle(const RE::ObjectRefHandle& handle);

	//RE::TESObjectREFR* TryToGetRefFromHandle(RE::ObjectRefHandle& handle, unsigned int max_try = 1);

	//RE::TESObjectREFR* TryToGetRefInCell(FormID baseid, Count count, float radius = 180);

    /*template <typename T>
    void ForEachRefInCell(T func) {
        const auto player_cell = RE::PlayerCharacter::GetSingleton()->GetParentCell();
        if (!player_cell) {
			logger::error("Player cell is null.");
			return;
		}
        auto& runtimeData = player_cell->GetRuntimeData();
        RE::BSSpinLockGuard locker(runtimeData.spinLock);
        for (auto& ref : runtimeData.references) {
			if (!ref) continue;
			func(ref.get());
		}
    }*/

};

namespace xData {

    namespace Copy {
        void CopyEnchantment(const RE::ExtraEnchantment* from, RE::ExtraEnchantment* to);
            
        void CopyHealth(const RE::ExtraHealth* from, RE::ExtraHealth* to);

        void CopyRank(const RE::ExtraRank* from, RE::ExtraRank* to);

        void CopyTimeLeft(const RE::ExtraTimeLeft* from, RE::ExtraTimeLeft* to);

        void CopyCharge(const RE::ExtraCharge* from, RE::ExtraCharge* to);

        void CopyScale(const RE::ExtraScale* from, RE::ExtraScale* to);

        void CopyUniqueID(const RE::ExtraUniqueID* from, RE::ExtraUniqueID* to);

        void CopyPoison(const RE::ExtraPoison* from, RE::ExtraPoison* to);

        void CopyObjectHealth(const RE::ExtraObjectHealth* from, RE::ExtraObjectHealth* to);

        void CopyLight(const RE::ExtraLight* from, RE::ExtraLight* to);

        void CopyRadius(const RE::ExtraRadius* from, RE::ExtraRadius* to);

        void CopyHorse(const RE::ExtraHorse* from, RE::ExtraHorse* to);

        void CopyHotkey(const RE::ExtraHotkey* from, RE::ExtraHotkey* to);

        void CopyTextDisplayData(const RE::ExtraTextDisplayData* from, RE::ExtraTextDisplayData* to);

        void CopySoul(const RE::ExtraSoul* from, RE::ExtraSoul* to);

        void CopyOwnership(const RE::ExtraOwnership* from, RE::ExtraOwnership* to);
    };

    template <typename T>
    void CopyExtraData(T* from, T* to){
        if (!from || !to) return;
        switch (T->EXTRADATATYPE) {
            case RE::ExtraDataType::kEnchantment:
                CopyEnchantment(from, to);
                break;
            case RE::ExtraDataType::kHealth:
                CopyHealth(from, to);
                break;
            case RE::ExtraDataType::kRank:
                CopyRank(from, to);
                break;
            case RE::ExtraDataType::kTimeLeft:
                CopyTimeLeft(from, to);
                break;
            case RE::ExtraDataType::kCharge:
                CopyCharge(from, to);
                break;
            case RE::ExtraDataType::kScale:
                CopyScale(from, to);
                break;
            case RE::ExtraDataType::kUniqueID:
                CopyUniqueID(from, to);
                break;
            case RE::ExtraDataType::kPoison:
                CopyPoison(from, to);
                break;
            case RE::ExtraDataType::kObjectHealth:
                CopyObjectHealth(from, to);
                break;
            case RE::ExtraDataType::kLight:
                CopyLight(from, to);
                break;
            case RE::ExtraDataType::kRadius:
                CopyRadius(from, to);
                break;
            case RE::ExtraDataType::kHorse:
                CopyHorse(from, to);
				break;
            case RE::ExtraDataType::kHotkey:
                CopyHotkey(from, to);
				break;
            case RE::ExtraDataType::kTextDisplayData:
				CopyTextDisplayData(from, to);
				break;
            case RE::ExtraDataType::kSoul:
				CopySoul(from, to);
                break;
            case RE::ExtraDataType::kOwnership:
                CopyOwnership(from, to);
                break;
            default:
                logger::warn("ExtraData type not found");
                break;
        };
    }

    [[nodiscard]] bool UpdateExtras(RE::ExtraDataList* copy_from, RE::ExtraDataList* copy_to);

    [[nodiscard]] bool UpdateExtras(RE::TESObjectREFR* copy_from, RE::TESObjectREFR* copy_to);

    int32_t GetXDataCostOverride(RE::ExtraDataList* xList);

    void AddTextDisplayData(RE::ExtraDataList* extraDataList, const std::string& displayName);

};

namespace DynamicForm {

    void copyBookAppearence(RE::TESForm* source, RE::TESForm* target);

    template <class T>
    static void copyComponent(RE::TESForm* from, RE::TESForm* to) {
        auto fromT = from->As<T>();

        auto toT = to->As<T>();

        if (fromT && toT) {
            toT->CopyComponent(fromT);
        }
    }

    void copyFormArmorModel(RE::TESForm* source, RE::TESForm* target);

    void copyFormObjectWeaponModel(RE::TESForm* source, RE::TESForm* target);

    void copyMagicEffect(RE::TESForm* source, RE::TESForm* target);

    void copyAppearence(RE::TESForm* source, RE::TESForm* target);

};