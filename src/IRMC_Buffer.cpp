#include <IRMC_Buffer.hpp>

#include <cstdlib>
#include <cstring>

namespace IRMC {

    Buffer::Buffer() {}

    Buffer::Buffer(char* data, size_t size)
    {
        m_Data = new char[size];
        memcpy(m_Data, data, size);
        m_Ptr = m_Data;

        m_Size = size;
    }
    
    Buffer::~Buffer()
    {
        delete[] m_Data;
    }
    
    void Buffer::Seek(size_t pos)
    {
        m_Ptr = m_Data + pos;
    }
    
    void Buffer::Skip(signed long pos)
    {
        m_Ptr += pos;

        if (m_Ptr < m_Data) {
            m_Ptr = m_Data;
        }

        if (m_Ptr >= m_Data + m_Size) {
            m_Ptr = m_Data + m_Size - 1;
        }
    }
    
    char Buffer::ReadByte()
    {
        if (m_Ptr + sizeof(char) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        char val = *(char*)m_Ptr;
        Skip(sizeof(char));

        return val;
    }

    unsigned char Buffer::ReadUByte()
    {
        if (m_Ptr + sizeof(unsigned char) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        unsigned char val = *(unsigned char*)m_Ptr;
        Skip(sizeof(unsigned char));

        return val;
    }

    short Buffer::ReadShort()
    {
        if (m_Ptr + sizeof(short) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        short val = *(short*)m_Ptr;
        Skip(sizeof(short));

        return val;
    }
    
    unsigned short Buffer::ReadUShort()
    {
        if (m_Ptr + sizeof(unsigned short) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        unsigned short val = *(unsigned short*)m_Ptr;
        Skip(sizeof(unsigned short));

        return val;
    }

    int Buffer::ReadInt()
    {
        if (m_Ptr + sizeof(int) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        int val = *(int*)m_Ptr;
        Skip(sizeof(int));

        return val;
    }
    
    unsigned int Buffer::ReadUInt()
    {
        if (m_Ptr + sizeof(unsigned int) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        unsigned int val = *(unsigned int*)m_Ptr;
        Skip(sizeof(unsigned int));

        return val;
    }
    
    long Buffer::ReadLong()
    {
        if (m_Ptr + sizeof(long) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        long val = *(long*)m_Ptr;
        Skip(sizeof(long));

        return val;
    }
    
    unsigned long Buffer::ReadULong()
    {
        if (m_Ptr + sizeof(unsigned long) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        unsigned long val = *(unsigned long*)m_Ptr;
        Skip(sizeof(unsigned long));

        return val;
    }
    
    float Buffer::ReadFloat()
    {
        if (m_Ptr + sizeof(float) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        float val = *(float*)m_Ptr;
        Skip(sizeof(float));

        return val;
    }
    
    double Buffer::ReadDouble()
    {
        if (m_Ptr + sizeof(double) >= m_Data + m_Size) {
            fprintf(stderr, "Tried reading out of bounds\n");
            exit(EXIT_FAILURE);
        }

        double val = *(double*)m_Ptr;
        Skip(sizeof(double));

        return val;
    }
    
    char* Buffer::GetData()
    {
        return m_Data;
    }

    char* Buffer::GetPtr()
    {
        return m_Ptr;
    }

    bool Buffer::IsEOF()
    {
        if (m_Ptr >= m_Data + m_Size) {
            return true;
        }

        return false;
    }

}