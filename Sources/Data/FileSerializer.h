#ifndef _FILE_SERIALIZER_H
#define _FILE_SERIALIZER_H

#include "Serializer.h"
#include "../utils.h"

// This is a singleton
class FileSerializer : public Serializer
{
public:
    ~FileSerializer();
    static FileSerializer * getInstance(FILE * f)
    {
        if (mInst == NULL) mInst = new FileSerializer();
        mInst->m_pFile = f;
        return mInst;
    };

    virtual void writeLong(long l);
    virtual long readLong();
    virtual void writeShort(short i);
    virtual short readShort();
    virtual void writeChar(char c);
    virtual char readChar();
    virtual void writeString(const char * s);
    virtual void readString(char * s);

private:
    FileSerializer();
    static FileSerializer * mInst;
    FILE * m_pFile;
};

#endif
