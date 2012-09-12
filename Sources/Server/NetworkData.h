#ifndef _NETWORK_DATA_H
#define _NETWORK_DATA_H

class DebugManager;

class NetworkData
{
public:
    // Constructor / destructor
    NetworkData(long iMessage);
    NetworkData(void * file);
    ~NetworkData();

    NetworkData * clone();

    void addLong(long l);
    void addString(const char * str);
    void addDouble(double d);
    void addCustom(void * p, long size);

    long readLong();
    bool readString(char * str, int maxSize);
    bool readString(char * str, int maxSize, DebugManager * pDebug, const char * sError);
    double readDouble();
    long readCustom(void * p);

    long dataYetToRead();
    void rewindCursor()
    {
        m_pReadCursor = m_pData;
    };
    long getSize()
    {
        return m_iCurrentSize;
    };
    void concat(NetworkData * pOther);

    void saveToFile(void * file);
    void loadFromFile(void * file);

protected:
    char * m_pData;
    long m_iCurrentSize;
    char * m_pReadCursor;
};

#endif
