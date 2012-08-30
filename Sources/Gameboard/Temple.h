#ifndef _TEMPLE_H
#define _TEMPLE_H

#include "MapObject.h"

#define STRING_MANATYPE         L"manatype"
#define STRING_AMOUNT           L"amount"

class Temple : public MapObject
{
public:
  // Constructor / destructor
  Temple(CoordsMap mapPos, Map * pMap, ObjectList ** pGlobalEffects);
  ~Temple();

  void init(u32 uTempleId, u8 uType, u8 uAmount);
  void serialize(NetworkData * pData);
  void deserialize(NetworkData * pData, LocalClient * pLocalClient);
  void initGraphics(DisplayEngine * pDisplay);

  // GraphicObject virtual functions
  virtual u32 getType() { return MapObject::getType() | GOTYPE_TEMPLE; };
  virtual void display();

  // MapObject virtual functions
  virtual int getTexture();
  virtual void updateIdentifiers() { swprintf_s(m_sIdentifiers, 16, L"temple %d %ld", (int) m_uOwner, (long) m_uTempleId); };

  // Other functions
  virtual wchar_t * getInfo(wchar_t * sBuf, int iSize, InfoDest eDest);

  // Member access
  u32 getId() { return m_uTempleId; };

protected:
  // Permanent data
  u32 m_uTempleId;
  wchar_t m_sTexture[MAX_PATH];
};

#endif
