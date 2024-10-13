#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include <spdlog/sinks/basic_file_sink.h>

namespace logger = SKSE::log;
using namespace std::literals;

const uint32_t player_refid = 20;

using FormID = RE::FormID;
using RefID = RE::FormID;
using Count = RE::TESObjectREFR::Count;

constexpr float EPSILON = 1e-6f;