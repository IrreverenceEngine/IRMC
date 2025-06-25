#include <IRMC_Log.hpp>
#include <IRMC_CTypes.hpp>

#include <cstdarg>
#include <cstdio>

#include <cstdlib>

namespace IRMC::Log {

    static struct {
        const char* colorcode;
        const char* prefix;
    } s_MsgTypeInfos[] = {
        { "1", "INFO" },
        { "1;33", "WARN" },
        { "1;31", "ERROR" },
        { "1;31;4", "FATAL" }
    };

    void Msg(MsgType type, const char* fmt, ...)
    {
        printf("\033[%sm[IRMC %s]\033[0m - ", s_MsgTypeInfos[(int)type].colorcode, s_MsgTypeInfos[(int)type].prefix);

        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);

        fwrite("\n", 1, 1, stdout);

        if (type == MsgType::FATAL) {
            abort();
        }
    }

}