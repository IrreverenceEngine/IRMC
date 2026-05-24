#pragma once

#include <IRX_Common.hpp>

#include <SDL3/SDL_endian.h>

#include <ostream>
#include <vector>

namespace IRMC {

    template<typename T>
    void WriteLE(std::vector<char>& stream, T value)
    {
        if constexpr (sizeof(T) == 4) {
            UInt32 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt32 v = SDL_Swap32LE(tmp);
            char* vp = (char*)&v;
            stream.reserve(stream.size() + sizeof(v));
            stream.insert(stream.end(), vp, vp + sizeof(v));
        } else if constexpr (sizeof(T) == 8) {
            UInt64 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt64 v = SDL_Swap64LE(tmp);
            char* vp = (char*)&v;
            stream.reserve(stream.size() + sizeof(v));
            stream.insert(stream.end(), vp, vp + sizeof(v));
        } else if constexpr (sizeof(T) == 2) {
            UInt16 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt16 v = SDL_Swap16LE(tmp);
            char* vp = (char*)&v;
            stream.reserve(stream.size() + sizeof(v));
            stream.insert(stream.end(), vp, vp + sizeof(v));
        } else {
            char* vp = (char*)&value;
            stream.insert(stream.end(), vp, vp + sizeof(value));
        }
    }

    template<typename T>
    void WriteLE(std::ostream& stream, T value) {
        if constexpr (sizeof(T) == 4) {
            UInt32 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt32 v = SDL_Swap32LE(tmp);
            stream.write((const char*)(&v), sizeof(v));
        } else if constexpr (sizeof(T) == 8) {
            UInt64 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt64 v = SDL_Swap64LE(tmp);
            stream.write((const char*)(&v), sizeof(v));
        } else if constexpr (sizeof(T) == 2) {
            UInt16 tmp;
            memcpy(&tmp, &value, sizeof(tmp));
            UInt16 v = SDL_Swap16LE(tmp);
            stream.write((const char*)(&v), sizeof(v));
        } else {
            stream.write((const char*)(&value), sizeof(value));
        }
    }

    static void Write(std::vector<char>& stream, const char* beg, UInt64 len)
    {
        stream.insert(stream.end(), beg, beg + len);
    }

    static void Write(std::ostream& stream, const char* beg, UInt64 len)
    {
        stream.write(beg, len);
    }

}
