#include "CustomObjects.h"

bool FormRefID::operator<(const FormRefID& other) const {
    return outerKey < other.outerKey || (outerKey == other.outerKey && innerKey < other.innerKey);
}

FormIDX::FormIDX(FormID id, bool value1, bool value2, std::string value3): id(id), equipped(value1), favorited(value2), name(value3) {}

bool FormRefIDX::operator<(const FormRefIDX& other) const {
    // Here, you can access the boolean values via outerKey
    return outerKey.id < other.outerKey.id ||
           (outerKey.id == other.outerKey.id && innerKey < other.innerKey);
}

bool FormFormID::operator<(const FormFormID& other) const {
    return outerKey < other.outerKey || (outerKey == other.outerKey && innerKey < other.innerKey);
}

Source::Source(const std::uint32_t id, const std::string id_str, const float capacity, const bool cs): cloud_storage(cs), capacity(capacity), formid(id), editorid(id_str) {
    logger::trace("Creating source with formid: {}, editorid: {}, capacity: {}, cloud storage: {}", formid, editorid, capacity, cs);
    if (!formid) {
        logger::trace("Formid is not found. Attempting to find formid for editorid {}.", editorid);
        auto form = RE::TESForm::LookupByEditorID(editorid);

        if (form) formid = form->GetFormID();
        else logger::info("Could not find formid for editorid {}", editorid);
    }
}

inline std::string_view Source::GetName() const {
    logger::trace("Getting name for formid: {}", formid);
    if (auto* form = GetFormByID(formid, editorid)) return form->GetName();
    return "";
}

RE::TESBoundObject* Source::GetBoundObject() const {return GetFormByID<RE::TESBoundObject>(formid, editorid);}

void Source::AddInitialItem(const FormID form_id, const Count count) {
    logger::trace("Adding initial item with formid: {} and count: {}", form_id, count);
    initial_items[form_id] = count;
}