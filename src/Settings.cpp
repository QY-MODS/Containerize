#include "Settings.h"

std::vector<Source> LoadSources()
{
    std::vector<Source> sources;
    const auto IniSources = LoadINISources();
    logger::trace("IniSources size: {}", IniSources.size());
    sources.insert(sources.end(), IniSources.begin(), IniSources.end());
    const auto YamlSources = LoadYAMLSources();
    logger::trace("YamlSources size: {}", YamlSources.size());
    sources.insert(sources.end(), YamlSources.begin(), YamlSources.end());
    return sources;
}

void LoadOtherSettings()
{
    using namespace Settings;
    logger::info("Loading ini settings: OtherStuff");

    std::unordered_map<std::string, bool> others;

    CSimpleIniA ini;
    CSimpleIniA::TNamesDepend otherkeys;

    ini.SetUnicode();
    ini.LoadFile(path);


    // other stuff section
    bool val1 = ini.GetBoolValue(InISections[2], otherstuffKeys[0]);
    bool val2 = ini.GetBoolValue(InISections[2], otherstuffKeys[1]);
    bool val3 = ini.GetBoolValue(InISections[2], otherstuffKeys[2]);
    bool val4 = ini.GetBoolValue(InISections[2], otherstuffKeys[3]);
    other_settings[otherstuffKeys[0]] = val1;
    other_settings[otherstuffKeys[1]] = val2;
    other_settings[otherstuffKeys[2]] = val3;
    other_settings[otherstuffKeys[3]] = val4;

    // log the values
    logger::info("INI_changed_msg: {}", val1);
    logger::info("RemoveCarryBoosts: {}", val2);
    logger::info("ReturnToInitialMenu: {}", val3);
    logger::info("BatchSell: {}", val4);
}

Source parseSource_(const YAML::Node& config)
{
	using namespace Settings;

    auto temp_formeditorid = config["FormEditorID"] && !config["FormEditorID"].IsNull() ? config["FormEditorID"].as<std::string>() : "";
    FormID temp_formid = temp_formeditorid.empty() ? 0 : GetFormEditorIDFromString(temp_formeditorid);
    const auto temp_weight_limit = config["weight_limit"] && !config["weight_limit"].IsNull() ? config["weight_limit"].as<float>() : 0.f;
    const auto cloud_storage = config["cloud_storage"] && !config["cloud_storage"].IsNull() ? config["cloud_storage"].as<bool>() : cloud_storage_enabled;
    logger::trace("FormEditorID: {}, FormID: {}, WeightLimit: {}, CloudStorage: {}", temp_formeditorid, temp_formid, temp_weight_limit, cloud_storage);
    Source source(temp_formid, "", temp_weight_limit, cloud_storage);

    if (!config["initial_items"] || config["initial_items"].size() == 0) {
        logger::info("initial_items are empty.");
        return source;
    }
    for (const auto& itemNode : config["initial_items"]) {
        temp_formeditorid = itemNode["FormEditorID"] && !itemNode["FormEditorID"].IsNull()
                                           ? itemNode["FormEditorID"].as<std::string>()
                                           : "";

        temp_formid = temp_formeditorid.empty() ? 0 : GetFormEditorIDFromString(temp_formeditorid);
        if (!temp_formid && !temp_formeditorid.empty()) {
            logger::error("Formid could not be obtained for {}", temp_formid, temp_formeditorid);
            continue;
        }
        if (!itemNode["count"] || itemNode["count"].IsNull()) {
            logger::error("Count is null.");
            continue;
        }
        logger::trace("Count");
        const Count temp_count = itemNode["count"].as<Count>();
        if (temp_count == 0) {
            logger::error("Count is 0.");
            continue;
        }
        source.AddInitialItem(temp_formid, temp_count);
    }
    return source;
}

std::vector<Source> LoadYAMLSources()
{
    logger::info("Loading yaml settings: Sources");
    std::vector<Source> sources;
    const auto folder_path = std::format("Data/SKSE/Plugins/{}", mod_name) + "/presets";
    std::filesystem::create_directories(folder_path);
    logger::trace("Custom path: {}", folder_path);
    for (const auto& entry : std::filesystem::directory_iterator(folder_path)) {
        if (entry.is_regular_file() && entry.path().extension() == ".yml") {
            const auto filename = entry.path().string();
            YAML::Node config = YAML::LoadFile(filename);

            if (!config["containers"]) {
                logger::trace("OwnerLists not found in {}", filename);
                continue;
            }

            for (const auto& node : config["containers"]) {
                // we have list of owners at each node or a scalar owner
                const auto source = parseSource_(node);
                if (source.formid == 0 && source.editorid.empty()) {
                    logger::error("LoadYAMLSources: File {} has invalid source: {}, {}", filename, source.formid, source.editorid);
					continue;
				}
                sources.push_back(source);
            }
        }
    }
    return sources;
}

std::vector<Source> LoadINISources()
{
	using namespace Settings;

    logger::info("Loading ini settings: Sources");

    std::vector<Source> sources;

    // Check if powerofthree's Tweaks is installed
    if (IsPo3Installed()) {
        logger::info("powerofthree's Tweaks is installed. Enabling EditorID support.");
        po3installed = true;
    } else {
        logger::info("powerofthree's Tweaks is not installed. Disabling EditorID support.");
        po3installed = false;
    }


    CSimpleIniA ini;
    CSimpleIniA::TNamesDepend source_names;
    CSimpleIniA::TNamesDepend otherkeys;

    ini.SetUnicode();
    ini.LoadFile(path);

    // Create Sections with defaults if they don't exist
    for (int i = 0; i < 2; ++i) {
        logger::info("Checking section {}", InISections[i]);
        if (!ini.SectionExists(InISections[i])) {
            logger::info("Section {} does not exist. Creating it.", InISections[i]);
            ini.SetValue(InISections[i], nullptr, nullptr);
            logger::info("Setting default keys for section {}", InISections[i]);
            ini.SetValue(InISections[i], InIDefaultKeys[i], InIDefaultVals[i], section_comments[i].c_str());
            logger::info("Default values set for section {}", InISections[i]);
        }
    }

    // Create Sections with defaults if they don't exist
    if (!ini.SectionExists(InISections[2])) {
        ini.SetBoolValue(InISections[2], otherstuffKeys[0], otherstuffVals[0], section_comments[2].c_str());
        logger::info("Default values set for section {}", InISections[2]);
    }

    ini.GetAllKeys(InISections[2], otherkeys);
    auto numOthers = otherkeys.size();
    logger::info("otherkeys size {}", numOthers);

    if (numOthers == 0 || numOthers != otherstuffKeys.size()) {
        logger::warn(
            "No other settings found in the ini file or Invalid number of other settings . Using defaults.");
        size_t index = 0;
        for (const auto& key : otherstuffKeys) {
			ini.SetBoolValue(InISections[2], key, otherstuffVals[index], os_comments[index].c_str());
			index++;
		}
    }


    // Sections: Containers, Capacities
    ini.GetAllKeys(InISections[0], source_names);
    auto numSources = source_names.size();
    logger::info("source_names size {}", numSources);

    sources.reserve(numSources);

    cloud_storage_enabled = ini.GetBoolValue(InISections[2], otherstuffKeys[4]);

    for (CSimpleIniA::TNamesDepend::const_iterator it = source_names.begin(); it != source_names.end(); ++it) {
        logger::info("source name {}", it->pItem);
        const char* val1 = ini.GetValue(InISections[0], it->pItem);
        const char* val2 = ini.GetValue(InISections[1], it->pItem);
        if (!val1 || !val2 || !std::strlen(val1) || !std::strlen(val2)) {
            logger::warn("Source {} is missing a value. Skipping.", it->pItem);
            continue;
        }
        logger::info("Source {} has a value of {}", it->pItem, val1);
        logger::info("We have valid entries for container: {} and capacity: {}", val1, val2);
        // back to container_id and capacity
        uint32_t id = static_cast<uint32_t>(std::strtoul(val1, nullptr, 16));
        std::string id_str = std::string(val1);

        // if both formid is valid hex, use it
        if (isValidHexWithLength7or8(val1)) {
            logger::info("Formid {} is valid hex", val1);
            sources.emplace_back(id, "", std::stof(val2), cloud_storage_enabled);
        }
        else if (!po3installed) {
            logger::error("No formid AND powerofthree's Tweaks is not installed.", val1);
            MsgBoxesNotifs::Windows::Po3ErrMsg();
            return sources;
        } 
        else sources.emplace_back(0, id_str, std::stof(std::string(val2)), cloud_storage_enabled);

        logger::trace("Source {} has a value of {}", it->pItem, val1);
        ini.SetValue(InISections[0], it->pItem, val1);
        ini.SetValue(InISections[1], it->pItem, val2);
        logger::info("Loaded container: {} with capacity: {}", val1, std::stof(val2));
    }

    ini.SaveFile(path);

    return sources;
}
