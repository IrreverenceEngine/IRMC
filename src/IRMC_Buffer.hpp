#pragma once

#include <cstdio>
#include <cstddef>

namespace IRMC {
    class Buffer {
    public:
        Buffer();
        Buffer(char* data, size_t size);
        ~Buffer();

        void Seek(size_t pos);
        void Skip(signed long pos);

        char ReadByte();
        unsigned char ReadUByte();

        short ReadShort();
        unsigned short ReadUShort();

        int ReadInt();
        unsigned int ReadUInt();

        long ReadLong();
        unsigned long ReadULong();

        float ReadFloat();
        double ReadDouble();

        char* GetData();
        size_t GetSize();
        char* GetPtr();
        size_t Tell();

        bool IsEOF();

    private:
        char* m_Data = nullptr;
        size_t m_Size = 0;
        char* m_Ptr = nullptr;
    };
}