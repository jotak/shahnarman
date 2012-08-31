#ifndef _DEBUG_MANAGER_H
#define _DEBUG_MANAGER_H

#include "../Common/TimeController.h"

class GeometryText;

#define DBG_MAX_LINES       40
#define DBG_MAX_CHARS       128
#define ERROR_MESSAGE_SIZE  128

class LocalClient;
class TextureEngine;
class FontEngine;

class DebugManager
{
public:
  // Constructor / destructor
  DebugManager(LocalClient * pLocalClient);
  ~DebugManager();

  // Texture(s) registration
  void registerTextures(TextureEngine * pTexEngine, FontEngine * pFontEngine);

  // Manager functions
  virtual void Init();
  virtual void Display();
  virtual void Update(double delta);

  // Other function
  void log(const wchar_t * sMsg);
  void addCustomeLine(const wchar_t * sLine);
  void notifyErrorMessage(s16 errorCode, const wchar_t * additionalInfo = NULL);
  void notifyErrorMessage(const wchar_t * errorMsg);
  void notifyINIErrorMessage(wchar_t * sFile, int errorCode);
  void notifyXMLErrorMessage(wchar_t * sFile, int errorCode, int line, int col);
  void notifyLoadingMessage(const wchar_t * msg);
  wchar_t * getErrorMessage(wchar_t * errorMsg, s16 errorCode);
  void clear();
  void switchShowFPS() { m_bShowFPS = !m_bShowFPS; };
  void autoStartGame();

private:
  LocalClient * m_pLocalClient;
  TimeController m_refreshTC;
  GeometryText * m_pFPSGeometry;
  GeometryText * m_pGeometries[DBG_MAX_LINES];
  short m_iCustomInfoNbLines;
  int m_iFontId;
  bool m_bShowFPS;
};

#endif
