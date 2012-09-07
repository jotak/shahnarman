#include "FileSerializer.h"
#include <stdio.h>

FileSerializer * FileSerializer::mInst = NULL;

// -----------------------------------------------------------------
// Name : FileSerializer
// -----------------------------------------------------------------
FileSerializer::FileSerializer()
{
  m_pFile = NULL;
}

// -----------------------------------------------------------------
// Name : ~FileSerializer
// -----------------------------------------------------------------
FileSerializer::~FileSerializer()
{
}

// -----------------------------------------------------------------
// Name : writeLong
// -----------------------------------------------------------------
void FileSerializer::writeLong(long l)
{
  fwrite(&l, sizeof(long), 1, m_pFile);
}

// -----------------------------------------------------------------
// Name : readLong
// -----------------------------------------------------------------
long FileSerializer::readLong()
{
  long l = 0;
  fread(&l, sizeof(long), 1, m_pFile);
  return l;
}

// -----------------------------------------------------------------
// Name : writeShort
// -----------------------------------------------------------------
void FileSerializer::writeShort(short i)
{
  fwrite(&i, sizeof(short), 1, m_pFile);
}

// -----------------------------------------------------------------
// Name : readShort
// -----------------------------------------------------------------
short FileSerializer::readShort()
{
  short i = 0;
  fread(&i, sizeof(short), 1, m_pFile);
  return i;
}

// -----------------------------------------------------------------
// Name : writeChar
// -----------------------------------------------------------------
void FileSerializer::writeChar(char c)
{
  fwrite(&c, sizeof(char), 1, m_pFile);
}

// -----------------------------------------------------------------
// Name : readChar
// -----------------------------------------------------------------
char FileSerializer::readChar()
{
  char c = 0;
  fread(&c, sizeof(char), 1, m_pFile);
  return c;
}

// -----------------------------------------------------------------
// Name : writeString
// -----------------------------------------------------------------
void FileSerializer::writeString(const char * s)
{
  int len = strlen(s) + 1;
  fwrite(&len, sizeof(int), 1, m_pFile);
  fwrite(s, sizeof(char), len, m_pFile);
}

// -----------------------------------------------------------------
// Name : readString
// -----------------------------------------------------------------
void FileSerializer::readString(char * s)
{
  int len = 0;
  fread(&len, sizeof(int), 1, m_pFile);
  fread(s, sizeof(char), len, m_pFile);
}
