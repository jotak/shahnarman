#ifndef _GUI_DOCUMENT_H
#define _GUI_DOCUMENT_H

#include "ComponentOwnerInterface.h"
#include "guiComponent.h"

class guiContainer;

class guiDocument : public guiObject, public ComponentOwnerInterface
{
public:
  // Constructor / destructor
  guiDocument();
  ~guiDocument();

  // GraphicObject virtual function
  virtual u32 getType() { return guiObject::getType() | GOTYPE_DOCUMENT; };
  virtual void update(double delta);
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);

  // Active input functions
  virtual guiObject * onButtonEvent(ButtonAction * pEvent);
  virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);

  // Size and position
  virtual void onResize(int iOldWidth, int iOldHeight);

  // Creation process functions
  void setTitle(const char * sTitle) { wsafecpy(m_sTitle, 32, sTitle); };
  void setTitleId(const char * sTitleId);

  // Components management
  void addComponent(guiComponent * cpnt);
  void deleteAllComponents();
  guiComponent * getFirstComponent() { return (guiComponent*) m_pComponentsList->getFirst(m_iCpntIt2); };
  guiComponent * getNextComponent() { return (guiComponent*) m_pComponentsList->getNext(m_iCpntIt2); };
  guiComponent * getLastComponent() { return (guiComponent*) m_pComponentsList->getLast(m_iCpntIt2); };
  guiComponent * getPrevComponent() { return (guiComponent*) m_pComponentsList->getPrev(m_iCpntIt2); };
  int getComponentsCount() { return m_pComponentsList->size; };
  bool deleteComponent(guiComponent * pCpnt);
  guiComponent * deleteCurrentComponent(bool bSetToNext);
  guiComponent * getComponentAt(int xPxl, int yPxl);
  ObjectList * getComponentsList() { return m_pComponentsList; };
  guiComponent * getComponent(const char * cpntId);

  // Other member access functions
  char * getTitle() { return m_sTitle; };
  void setFocusedComponent(guiComponent * pCpnt);
  guiComponent * getFocusedComponent() { return m_pFocusedComponent; };
  bool isEnabled() { return m_bEnabled; };
  void setEnabled(bool bEnabled) { m_bEnabled = bEnabled; };
  void doClick(const char * sCpntId);

  // Handlers
  virtual void onLoad() {};
  virtual void onShow() {};
  virtual void onHide() {};
  virtual void bringAbove(guiComponent * pCpnt);

  // Other functions
  void close() { m_bNeedDestroy = true; };
  bool doesNeedDestroy() { return m_bNeedDestroy; };
  virtual void onDestroy(void * pDestroyInfo) {};
  bool didContentChange() { return m_bContentChanged; };
  void setContentChanged() { m_bContentChanged = true; };
  virtual BaseObject * getTargetedObject(u8 * isLuaPlayerGO) { return NULL; };
  virtual void setTargetValid(bool bValid) {};
  void setOwner(guiContainer * pOwner) { m_pOwner = pOwner; };

  // Clone / init
  virtual void init(const char * sTitle, int iTexId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();

protected:
  char m_sTitle[32];
  guiComponent * m_pFocusedComponent;
  bool m_bNeedDestroy;
  bool m_bContentChanged;
  bool m_bEnabled;
  int m_iCpntIt1, m_iCpntIt2;
  guiContainer * m_pOwner;

private:
  ObjectList * m_pComponentsList;
};

#endif
