// -----------------------------------------------------------------
// NETWORK DATA
// -----------------------------------------------------------------
#include "NetworkData.h"
#include "../utils.h"
#include "../Debug/DebugManager.h"
#include <stdio.h>

// -----------------------------------------------------------------
// Name : NetworkData
// -----------------------------------------------------------------
NetworkData::NetworkData(long iMessage)
{
    m_pData = NULL;
    m_iCurrentSize = 0;
    addLong(iMessage);
    m_pReadCursor = NULL;
}

// -----------------------------------------------------------------
// Name : NetworkData
// -----------------------------------------------------------------
NetworkData::NetworkData(void * file)
{
    m_pData = NULL;
    loadFromFile(file);
}

// -----------------------------------------------------------------
// Name : ~NetworkData
// -----------------------------------------------------------------
NetworkData::~NetworkData()
{
    if (m_pData != NULL)
        delete[] m_pData;
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
NetworkData * NetworkData::clone()
{
    NetworkData * pData = new NetworkData((long)0);
    delete[] pData->m_pData;
    pData->m_pData = new char[m_iCurrentSize];
    memcpy(pData->m_pData, m_pData, m_iCurrentSize);
    pData->m_iCurrentSize = m_iCurrentSize;
    pData->m_pReadCursor = m_pReadCursor;
    return pData;
}

// -----------------------------------------------------------------
// Name : addLong
// -----------------------------------------------------------------
void NetworkData::addLong(long l)
{
    if (m_iCurrentSize == 0)
    {
        m_iCurrentSize = sizeof(long);
        m_pData = new char[m_iCurrentSize];
        memcpy(m_pData, &l, sizeof(long));
    }
    else
    {
        char * oldData = m_pData;
        long oldSize = m_iCurrentSize;
        m_iCurrentSize += sizeof(long);
        m_pData = new char[m_iCurrentSize];
        char * ptr = m_pData;
        memcpy(ptr, oldData, oldSize);
        ptr += oldSize;
        memcpy(ptr, &l, sizeof(long));
        delete[] oldData;
    }
}

// -----------------------------------------------------------------
// Name : addDouble
// -----------------------------------------------------------------
void NetworkData::addDouble(double d)
{
    if (m_iCurrentSize == 0)
    {
        m_iCurrentSize = sizeof(double);
        m_pData = new char[m_iCurrentSize];
        memcpy(m_pData, &d, sizeof(double));
    }
    else
    {
        char * oldData = m_pData;
        long oldSize = m_iCurrentSize;
        m_iCurrentSize += sizeof(double);
        m_pData = new char[m_iCurrentSize];
        char * ptr = m_pData;
        memcpy(ptr, oldData, oldSize);
        ptr += oldSize;
        memcpy(ptr, &d, sizeof(double));
        delete[] oldData;
    }
}

// -----------------------------------------------------------------
// Name : addString
// -----------------------------------------------------------------
void NetworkData::addString(const char * str)
{
    long size = (long) strlen(str);
    addLong(size);
    char * oldData = m_pData;
    long oldSize = m_iCurrentSize;
    m_iCurrentSize += size * sizeof(char);
    m_pData = new char[m_iCurrentSize];
    char * ptr = m_pData;
    memcpy(ptr, oldData, oldSize);
    ptr += oldSize;
    memcpy(ptr, str, size * sizeof(char));
    delete[] oldData;
}

// -----------------------------------------------------------------
// Name : addCustom
// -----------------------------------------------------------------
void NetworkData::addCustom(void * p, long size)
{
    addLong(size);
    char * oldData = m_pData;
    long oldSize = m_iCurrentSize;
    m_iCurrentSize += size;
    m_pData = new char[m_iCurrentSize];
    char * ptr = m_pData;
    memcpy(ptr, oldData, oldSize);
    ptr += oldSize;
    memcpy(ptr, p, size);
    delete[] oldData;
}

// -----------------------------------------------------------------
// Name : readLong
// -----------------------------------------------------------------
long NetworkData::readLong()
{
    if (m_pReadCursor == NULL)
        m_pReadCursor = m_pData;
    long l;
    memcpy(&l, m_pReadCursor, sizeof(long));
    m_pReadCursor += sizeof(long);
    return l;
}

// -----------------------------------------------------------------
// Name : readDouble
// -----------------------------------------------------------------
double NetworkData::readDouble()
{
    if (m_pReadCursor == NULL)
        m_pReadCursor = m_pData;
    double d;
    memcpy(&d, m_pReadCursor, sizeof(double));
    m_pReadCursor += sizeof(double);
    return d;
}

// -----------------------------------------------------------------
// Name : readString
// -----------------------------------------------------------------
bool NetworkData::readString(char * str, int maxSize)
{
    long size = readLong();
    if (size > maxSize-1)
    {
        memcpy(str, m_pReadCursor, (maxSize-1) * sizeof(char));
        str[maxSize-1] = '\0';
        m_pReadCursor += (maxSize-1) * sizeof(char);
        return false;
    }
    else
    {
        memcpy(str, m_pReadCursor, size * sizeof(char));
        str[size] = '\0';
        m_pReadCursor += size * sizeof(char);
        return true;
    }
}

// -----------------------------------------------------------------
// Name : readString
// -----------------------------------------------------------------
bool NetworkData::readString(char * str, int maxSize, DebugManager * pDebug, const char * sError)
{
    if (!readString(str, maxSize))
    {
        pDebug->notifyErrorMessage(sError);
        return false;
    }
    return true;
}

// -----------------------------------------------------------------
// Name : readCustom
// -----------------------------------------------------------------
long NetworkData::readCustom(void * p)
{
    long size = readLong();
    memcpy(p, m_pReadCursor, size);
    m_pReadCursor += size;
    return size;
}

// -----------------------------------------------------------------
// Name : dataYetToRead
// -----------------------------------------------------------------
long NetworkData::dataYetToRead()
{
    if (m_pReadCursor == NULL)
        m_pReadCursor = m_pData;
    return (long) (m_iCurrentSize - (m_pReadCursor-m_pData));
}

// -----------------------------------------------------------------
// Name : concat
// -----------------------------------------------------------------
void NetworkData::concat(NetworkData * pOther)
{
    if (m_iCurrentSize == 0)
    {
        m_iCurrentSize = pOther->getSize() - sizeof(long);
        if (m_iCurrentSize > 0)
        {
            m_pData = new char[m_iCurrentSize];
            memcpy(m_pData, pOther->m_pData + sizeof(long), m_iCurrentSize);
        }
    }
    else
    {
        long otherSize = pOther->getSize() - sizeof(long);
        if (otherSize > 0)
        {
            char * oldData = m_pData;
            long oldSize = m_iCurrentSize;
            m_iCurrentSize += otherSize;
            m_pData = new char[m_iCurrentSize];
            char * ptr = m_pData;
            memcpy(ptr, oldData, oldSize);
            ptr += oldSize;
            memcpy(ptr, pOther->m_pData + sizeof(long), otherSize);
            delete[] oldData;
        }
    }
}

// -----------------------------------------------------------------
// Name : saveToFile
// -----------------------------------------------------------------
void NetworkData::saveToFile(void * file)
{
    fwrite(&m_iCurrentSize, sizeof(long), 1, (FILE*) file);
    fwrite(m_pData, sizeof(char), m_iCurrentSize, (FILE*) file);
}

// -----------------------------------------------------------------
// Name : loadFromFile
// -----------------------------------------------------------------
void NetworkData::loadFromFile(void * file)
{
    if (m_pData != NULL)
        delete[] m_pData;
    fread(&m_iCurrentSize, sizeof(long), 1, (FILE*) file);
    m_pData = new char[m_iCurrentSize];
    fread(m_pData, sizeof(char), m_iCurrentSize, (FILE*) file);
    m_pReadCursor = NULL;
}
