#include "NetworkSerializer.h"
#include "../Debug/DebugManager.h"

NetworkSerializer * NetworkSerializer::mInst = NULL;

// -----------------------------------------------------------------
// Name : NetworkSerializer
// -----------------------------------------------------------------
NetworkSerializer::NetworkSerializer()
{
    m_pData = NULL;
}

// -----------------------------------------------------------------
// Name : ~NetworkSerializer
// -----------------------------------------------------------------
NetworkSerializer::~NetworkSerializer()
{
}

// -----------------------------------------------------------------
// Name : writeLong
// -----------------------------------------------------------------
void NetworkSerializer::writeLong(long l)
{
    m_pData->addLong(l);
}

// -----------------------------------------------------------------
// Name : readLong
// -----------------------------------------------------------------
long NetworkSerializer::readLong()
{
    return m_pData->readLong();
}

// -----------------------------------------------------------------
// Name : writeShort
// -----------------------------------------------------------------
void NetworkSerializer::writeShort(short i)
{
    m_pData->addLong((long) i);
}

// -----------------------------------------------------------------
// Name : readShort
// -----------------------------------------------------------------
short NetworkSerializer::readShort()
{
    return (short) m_pData->readLong();
}

// -----------------------------------------------------------------
// Name : writeChar
// -----------------------------------------------------------------
void NetworkSerializer::writeChar(char c)
{
    m_pData->addLong((long) c);
}

// -----------------------------------------------------------------
// Name : readChar
// -----------------------------------------------------------------
char NetworkSerializer::readChar()
{
    return (char) m_pData->readLong();
}

// -----------------------------------------------------------------
// Name : writeString
// -----------------------------------------------------------------
void NetworkSerializer::writeString(const char * s)
{
    m_pData->addString(s);
}

// -----------------------------------------------------------------
// Name : readString
// -----------------------------------------------------------------
bool NetworkSerializer::readString(char * s, int maxSize)
{
    return m_pData->readString(s, maxSize);
}

// -----------------------------------------------------------------
// Name : readString
// -----------------------------------------------------------------
bool NetworkSerializer::readString(char * str, int maxSize, DebugManager * pDebug, const char * sErrorMessage)
{
    if (!readString(str, maxSize))
    {
        pDebug->notifyErrorMessage(sErrorMessage);
        return false;
    }
    return true;
}
