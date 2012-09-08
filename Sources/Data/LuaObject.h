#ifndef _LUA_OBJECT_H
#define _LUA_OBJECT_H

#include "../Common/ObjectList.h"
#include "../Players/Mana.h"
#include <lua5.1/lua.hpp>

class DebugManager;
class NetworkData;
class LuaObject;

#define LUA_FUNCTION_PARAMS_MAX_CHARS   256
#define LUAOBJECT_SPELL                 0x00000001
#define LUAOBJECT_SKILL                 0x00000002
#define LUAOBJECT_BUILDING              0x00000004
#define LUAOBJECT_SPECIALTILE           0x00000008

class ChildEffect : public BaseObject
{
public:
    ChildEffect()
    {
        pTargets = new ObjectList(false);
    };
    ~ChildEffect()
    {
        delete pTargets;
    };
    int id;
    char sName[NAME_MAX_CHARS];
    char sIcon[MAX_PATH];
    Mana cost;
    ObjectList * pTargets;
    void resetResolveParameters()
    {
        wsafecpy(sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, "");
    };
    void addResolveParameters(char * sParams)
    {
        wsafecat(sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, " ");
        wsafecat(sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, sParams);
    };
    char sResolveParams[LUA_FUNCTION_PARAMS_MAX_CHARS];
    LuaObject * getLua()
    {
        return (LuaObject*) getAttachment();
    };
};

class LuaObject : public BaseObject
{
public:
    LuaObject(u32 uInstance, const char * sEdition, const char * sObjectType, const char * sObjectName, DebugManager * pDebug);
    ~LuaObject();

    virtual u32 getType() = 0;
    virtual void loadBasicData(DebugManager * pDebug) = 0;
    bool isLoaded()
    {
        return m_pLuaState != NULL;
    };
    bool callLuaFunction(const char * sFunc, int iNbResults, const char * sParamsType, ...);
    lua_State * prepareLuaFunction(const char * sFunc);
    bool callPreparedLuaFunction(int iNbParams, int iNbResults, const char * sFunc, const char * sParams);
    double getLuaNumber();
    void getLuaString(char * sString, int size);
    bool getLuaVarNumber(const char * sVarName, double * d);
    bool getLuaVarString(const char * sVarName, char * sString, int size);
    bool getLuaVarNumberArray(const char * sVarName, double * pArray, int size);
    bool getLuaVarStringArray(const char * sVarName, char ** pArray, int tabSize, int strSize);
    u32 getInstanceId()
    {
        return m_uInstanceId;
    };
    char * getUniqueId(char * sId, int iSize);
    bool isUniqueId(const char * sEdition, const char * sObjectType, const char * sObjectName);
    char * getObjectEdition()
    {
        return m_sObjectEdition;
    };
    char * getObjectName()
    {
        return m_sObjectName;
    };
    virtual char * getLocalizedName()
    {
        return NULL;
    };
    virtual char * getLocalizedDescription()
    {
        return NULL;
    };
    virtual char * getIconPath() = 0;
    void setExtraMana(Mana mana)
    {
        m_ExtraMana = mana;
    };
    Mana getExtraMana()
    {
        return m_ExtraMana;
    };

    // Child effects
    int getNbChildEffects()
    {
        return m_iNbChildEffects;
    };
    ChildEffect * getChildEffect(int idx)
    {
        return &(m_pChildEffects[idx]);
    };
    void setCurrentEffect(int id)
    {
        m_iCurrentEffect = id;
    };
    int getCurrentEffect()
    {
        return m_iCurrentEffect;
    };

    // Targets
    ObjectList * getTargets()
    {
        return m_pTargets;
    };
    void addTarget(BaseObject * pTarget, long iType)
    {
        m_pTargets->addLast(pTarget, iType);
    };
    void removeTarget(BaseObject * pTarget)
    {
        m_pTargets->deleteObject(pTarget, true);
    };
    void removeFromTargets();

    static u32 static_uIdGenerator;
    static LuaObject * static_pCurrentLuaCaller;

protected:
    u32 m_uInstanceId;
    char m_sObjectEdition[NAME_MAX_CHARS];
    char m_sObjectName[NAME_MAX_CHARS];
    char m_sObjectType[NAME_MAX_CHARS];
    lua_State * m_pLuaState;
    DebugManager * m_pDebug;
    int m_iNbChildEffects;
    ChildEffect * m_pChildEffects;
    ObjectList * m_pTargets;
    int m_iCurrentEffect;
    Mana m_ExtraMana;
};

#endif
