#ifndef _SERIALIZER_H
#define _SERIALIZER_H

class Serializer
{
public:
  virtual void writeLong(long l) = 0;
  virtual long readLong() = 0;
  virtual void writeShort(short i) = 0;
  virtual short readShort() = 0;
  virtual void writeChar(char c) = 0;
  virtual char readChar() = 0;
  virtual void writeString(const wchar_t * s) = 0;
  virtual void readString(wchar_t * s) = 0;
};

#endif
