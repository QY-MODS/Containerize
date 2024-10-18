#include "MCP.h"

void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void __stdcall UI::RenderStatus()
{
    constexpr auto color_operational = ImVec4(0, 1, 0, 1);
    constexpr auto color_not_operational = ImVec4(1, 0, 0, 1);

    if (!M) {
		ImGui::TextColored(color_not_operational, "Mod is not working! Check log for more info.");
        return;
	}
    ImGui::Text("Status: ");
	ImGui::SameLine();
    ImGui::TextColored(color_operational, std::format("Sources ({})",n_sources).c_str());

}

void __stdcall UI::RenderSettings()
{
    if (ImGui::BeginTable("table_settings", 2, table_flags)) {
        for (const auto& [setting_name, setting] : other_settings) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text(setting_name.c_str());
            ImGui::TableNextColumn();
            const auto temp_setting_val = setting;
            const char* value = temp_setting_val ? "Enabled" : "Disabled";
            const auto color = setting ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1);
            ImGui::TextColored(color, value);
        }
        ImGui::EndTable();
    }
}

void __stdcall UI::RenderInspect()
{
	RefreshButton();

	ImGui::Text("Dynamic Forms");
	if (dynamic_forms.empty()) {
		ImGui::Text("No dynamic forms found.");
		return;
	}
	// dynamic forms table: FormID, Name, Status
	if (ImGui::BeginTable("table_dynamic_forms", 3, table_flags)) {
		for (const auto& [formid, form] : dynamic_forms) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text(std::format("{:08X}", formid).c_str());
			ImGui::TableNextColumn();
			ImGui::Text(form.first.c_str());
			ImGui::TableNextColumn();
			const auto color = form.second == 2 ? ImVec4(0, 1, 0, 1) : form.second == 1 ? ImVec4(1, 1, 0, 1) : ImVec4(1, 0, 0, 1);
			ImGui::TextColored(color, form.second == 2 ? "Active" : form.second == 1 ? "Protected" : "Inactive");
		}
		ImGui::EndTable();
	}

}

void __stdcall UI::RenderLog()
{
#ifndef NDEBUG
    ImGui::Checkbox("Trace", &LogSettings::log_trace);
#endif
    ImGui::SameLine();
    ImGui::Checkbox("Info", &LogSettings::log_info);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &LogSettings::log_warning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &LogSettings::log_error);

    // if "Generate Log" button is pressed, read the log file
    if (ImGui::Button("Generate Log")) logLines = ReadLogFile();

    // Display each line in a new ImGui::Text() element
    for (const auto& line : logLines) {
        if (!LogSettings::log_trace && line.find("trace") != std::string::npos) continue;
        if (!LogSettings::log_info && line.find("info") != std::string::npos) continue;
        if (!LogSettings::log_warning && line.find("warning") != std::string::npos) continue;
        if (!LogSettings::log_error && line.find("error") != std::string::npos) continue;
        ImGui::Text(line.c_str());
    }
}

void UI::Register(Manager* manager)
{
    if (!SKSEMenuFramework::IsInstalled()) {
        return;
    }
    SKSEMenuFramework::SetSection(mod_name);
    SKSEMenuFramework::AddSectionItem("Status", RenderStatus);
    SKSEMenuFramework::AddSectionItem("Settings", RenderSettings);
    SKSEMenuFramework::AddSectionItem("Inspect", RenderInspect);
    SKSEMenuFramework::AddSectionItem("Log", RenderLog);
    M = manager;
	n_sources = M->GetSources().size();
}

void UI::RefreshButton()
{
    FontAwesome::PushSolid();

    if (ImGui::Button((FontAwesome::UnicodeToUtf8(0xf021) + " Refresh").c_str()) || last_generated.empty()) {
		Refresh();
    }
    FontAwesome::Pop();

    ImGui::SameLine();
    ImGui::Text(("Last Generated: " + last_generated).c_str());
}

void UI::Refresh()
{

    last_generated = std::format("{} (in-game hours)", RE::Calendar::GetSingleton()->GetHoursPassed());
    n_sources = M->GetSources().size();
    const auto DFT = DynamicFormTracker::GetSingleton();
	for (const auto& df : DFT->GetDynamicForms()) {
		if (const auto form = RE::TESForm::LookupByID(df); form) {
			auto status = DFT->IsActive(df) ? 2 : DFT->IsProtected(df) ? 1 : 0;
			dynamic_forms[df] = { form->GetName(), status };
		}
    }
}
