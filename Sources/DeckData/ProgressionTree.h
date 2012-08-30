#ifndef _PROGRESSION_TREE_H
#define _PROGRESSION_TREE_H

#include "ProgressionElement.h"

#define NB_PROGRESSION_LEVELS     5
#define PROGRESSION_ETHNICITY     0
#define PROGRESSION_MAGIC         1
#define PROGRESSION_TRAIT         2

class DebugManager;
class XMLLiteElement;

class ProgressionTree : public XMLObject
{
public:
  ProgressionTree(wchar_t * sEdition, wchar_t * sName, u8 uType, DebugManager * pDebug);
  ~ProgressionTree();

  ProgressionElement * findElement(wchar_t * sId);
  ProgressionEffect * readXMLEffect(XMLLiteElement * pNode, XMLLiteElement * pRootNode, wchar_t * sId, DebugManager * pDebug);
  wchar_t * getDescription(wchar_t * sBuf, int iBufSize, LocalClient * pLocalClient);

  u8 m_uType;
  ObjectList * m_pElements[NB_PROGRESSION_LEVELS];
  ObjectList * m_pBaseEffects;
  wchar_t m_sEdition[NAME_MAX_CHARS];
};

#endif
