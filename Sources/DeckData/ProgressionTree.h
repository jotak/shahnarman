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
  ProgressionTree(char * sEdition, char * sName, u8 uType, DebugManager * pDebug);
  ~ProgressionTree();

  ProgressionElement * findElement(char * sId);
  ProgressionEffect * readXMLEffect(XMLLiteElement * pNode, XMLLiteElement * pRootNode, char * sId, DebugManager * pDebug);
  char * getDescription(char * sBuf, int iBufSize, LocalClient * pLocalClient);

  u8 m_uType;
  ObjectList * m_pElements[NB_PROGRESSION_LEVELS];
  ObjectList * m_pBaseEffects;
  char m_sEdition[NAME_MAX_CHARS];
};

#endif
