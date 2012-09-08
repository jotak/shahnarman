#ifndef _SKILL_H
#define _SKILL_H

#include "../Data/LuaObject.h"
#include "../Server/NetworkData.h"

#define SKILL_OBJECT_NAME   "skills"

class UnitData;
class Unit;

class Skill : public LuaObject
{
public:
    Skill(const char * sEdition, const char * sObjectName, const char * sParams, DebugManager * pDebug);
    Skill(u32 uId, const char * sEdition, const char * sObjectName, const char * sParams, DebugManager * pDebug);
    ~Skill();

    virtual u32 getType()
    {
        return LUAOBJECT_SKILL;
    };
    virtual void loadBasicData(DebugManager * pDebug);
    static Skill * deserialize(u32 uInstanceId, NetworkData * pData, DebugManager * pDebug);
    void serialize(NetworkData * pData);
    Skill * clone(bool bKeepInstance, DebugManager * pDebug);
    char * getLocalizedName()
    {
        return m_sName;
    };
    char * getLocalizedDescription()
    {
        return m_sDescription;
    };
    char * getParameters()
    {
        return m_sParameters;
    };
    void setCaster(Unit * pCaster)
    {
        m_pCaster = pCaster;
    };
    Unit * getCaster()
    {
        return m_pCaster;
    };
    char * getIconPath()
    {
        return m_sIconPath;
    };
    bool isMergeable()
    {
        return m_bMergeable;
    };
    bool isCumulative()
    {
        return m_bCumulative;
    };
    void merge(Skill * pOther);

protected:
    void init(const char * sParams, DebugManager * pDebug);

    char m_sName[NAME_MAX_CHARS];
    char m_sDescription[DESCRIPTION_MAX_CHARS];
    char m_sParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
    char m_sIconPath[MAX_PATH];
    bool m_bMergeable;
    bool m_bCumulative;

    Unit * m_pCaster; // Temporary data used when activating skill
};

#endif
