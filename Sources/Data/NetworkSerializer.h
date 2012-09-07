#ifndef _NETWORK_SERIALIZER_H
#define _NETWORK_SERIALIZER_H

#include "Serializer.h"
#include "../Server/NetworkData.h"
#include "../utils.h"

// This is a singleton
class NetworkSerializer : public Serializer
{
public:
  ~NetworkSerializer();
  static NetworkSerializer * getInstance(NetworkData * pData) { if (mInst == NULL) mInst = new NetworkSerializer(); mInst->m_pData = pData; return mInst; };

  virtual void writeLong(long l);
  virtual long readLong();
  virtual void writeShort(short i);
  virtual short readShort();
  virtual void writeChar(char c);
  virtual char readChar();
  virtual void writeString(const char * s);
  virtual void readString(char * s);

private:
	NetworkSerializer();
  static NetworkSerializer * mInst;
  NetworkData * m_pData;
};

#endif
