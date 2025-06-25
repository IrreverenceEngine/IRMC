#pragma once

#include <IRMC_Macro.hpp>

namespace IRMC::Log {
    enum class MsgType {
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    void Msg(MsgType type, const char* fmt, ...);
}

#define IRMC_MSG(_type, ...) IRMC::Log::Msg(IRMC::Log::MsgType::_type, __VA_ARGS__)