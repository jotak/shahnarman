#ifndef _GUI_COMPONENT_H
#define _GUI_COMPONENT_H

#include "guiObject.h"

#define HIGHLIGHT_TYPE_NONE         0
#define HIGHLIGHT_TYPE_NORMAL_RED   1

#define CPNT_ID_MAX_CHARS   128

class ComponentOwnerInterface;

class guiComponent : public guiObject
{
public:
  // Constructor / destructor
  guiComponent();
  ~guiComponent();

  // GraphicObject functions
  virtual u32 getType() { return GraphicObject::getType() | GOTYPE_COMPONENT; };

  // Events
  virtual void onFocusLost() {};

  // Status
  bool isVisible() { return m_bVisible; };
  virtual void setVisible(bool bVisible);
  bool isEnabled() { return m_bEnabled; };
  virtual void setEnabled(bool bEnabled) { m_bEnabled = bEnabled; };

  // Member access
  void setId(const wchar_t * id) { wsafecpy(m_sCpntId, 32, id); };
  wchar_t * getId() { return m_sCpntId; };
  void setOwner(ComponentOwnerInterface * pDoc) { m_pOwner = pDoc; };
  ComponentOwnerInterface * getOwner() { return m_pOwner; };

  // Other
  virtual bool isAt(int xPxl, int yPxl) { return m_bVisible && guiObject::isAt(xPxl, yPxl); };
  void highlight(u8 type);
  void centerOnComponent(guiComponent * pOther);

  // Clone / init
  virtual void init(const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl);
//  virtual guiObject * clone();

protected:
  bool m_bVisible;
  bool m_bEnabled;
  u8 m_uHighlight;
  wchar_t m_sCpntId[CPNT_ID_MAX_CHARS];
  ComponentOwnerInterface * m_pOwner;
};

#endif
