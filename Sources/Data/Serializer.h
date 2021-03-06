#ifndef _SERIALIZER_H
#define _SERIALIZER_H

class DebugManager;

class Serializer
{
public:
    virtual void writeLong(long l) = 0;
    virtual long readLong() = 0;
    virtual void writeShort(short i) = 0;
    virtual short readShort() = 0;
    virtual void writeChar(char c) = 0;
    virtual char readChar() = 0;
    virtual void writeString(const char * s) = 0;
    virtual bool readString(char * s, int maxSize) = 0;
    virtual bool readString(char * s, int maxSize, DebugManager * pDebug, const char * sErrorMessage) = 0;
};

#endif
