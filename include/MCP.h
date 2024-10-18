#pragma once
#include "Events.h"
#include "SKSEMCP/SKSEMenuFramework.hpp"


static void HelpMarker(const char* desc);

namespace UI {

    inline ImGuiTableFlags table_flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

	inline Manager* M;
    void __stdcall RenderStatus();
    void __stdcall RenderSettings();
    void __stdcall RenderInspect();
    void __stdcall RenderLog();
    void Register(Manager* manager);

    //inline std::map<RefID, FormID> current_containers;
    inline std::map<FormID,std::pair<std::string,int>> dynamic_forms;
    inline size_t n_sources;

    inline std::string log_path = GetLogPath().string();
    inline std::vector<std::string> logLines;

    inline std::string last_generated;
    void RefreshButton();
    void Refresh();

}