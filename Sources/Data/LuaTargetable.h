#ifndef _LUA_TARGETABLE_H
#define _LUA_TARGETABLE_H

#include "../lua_callbacks.h"
#include "../Common/ObjectList.h"
#include "LuaObject.h"

#define HANDLER_RESULT_TYPE_NONE    0
#define HANDLER_RESULT_TYPE_BAND    1
#define HANDLER_RESULT_TYPE_BOR     2
#define HANDLER_RESULT_TYPE_ADD     3

class LocalClient;

class LuaTargetable
{
public:
    LuaTargetable(ObjectList ** pGlobalEffects, const char * sIdentifiers);
    ~LuaTargetable();

    // General
    char * getIdentifiers()
    {
        return m_sIdentifiers;
    };

    // Effects
    LuaObject * getFirstEffect(int _it)
    {
        return (LuaObject*) m_pEffects->getFirst(_it);
    };
    LuaObject * getNextEffect(int _it)
    {
        return (LuaObject*) m_pEffects->getNext(_it);
    };
    ObjectList * getAllEffects()
    {
        return m_pEffects;
    };
    int getNumberOfEffects()
    {
        return m_pEffects->size;
    };
    void attachEffect(LuaObject * pEffect);
    void detachEffect(LuaObject * pEffect);
    void removeAllEffects()
    {
        m_pEffects->deleteAll();
    };
    void disableEffect(LuaObject * pEffect);
    void enableAllEffects();
    ObjectList * getDisabledEffects()
    {
        return m_pDisabledEffects;
    }

    // Child effects
    ChildEffect * getFirstChildEffect(int _it)
    {
        return (ChildEffect*) m_pChildEffects->getFirst(_it);
    };
    ChildEffect * getNextChildEffect(int _it)
    {
        return (ChildEffect*) m_pChildEffects->getNext(_it);
    };
    void attachChildEffect(ChildEffect * pEffect);
    bool detachChildEffect(ChildEffect * pEffect);

    // Variables
    void registerValue(const char * sName, long baseValue);
    virtual long getValue(const char * sName, bool bBase = false, bool * bFound = NULL);
    virtual bool setBaseValue(const char * sName, long val);

    // Other
    long callEffectHandler(const char * sFunc, const char * sArgsType = "", void ** pArgs = NULL, u8 uResultType = HANDLER_RESULT_TYPE_NONE);
    void getInfo_AddValue(char * sBuf, int iSize, const char * sKey, const char * sSeparator);
    static LuaTargetable * convertFromBaseObject(BaseObject * pObj, u8 uType);
    BaseObject * convertToBaseObject(u8 uType);

protected:
    void serializeValues(NetworkData * pData);
    void deserializeValues(NetworkData * pData);

    long_hash m_lValues;
    ObjectList ** m_pGlobalEffects;
    char m_sIdentifiers[16];

private:
    bool _callEffectHandlerForEffect(LuaObject * pLua, int iChild, const char * sFunc, const char * sArgsType, void ** pArgs, int nbResults);

    ObjectList * m_pEffects;
    ObjectList * m_pDisabledEffects;
    ObjectList * m_pChildEffects; // Child effects are not LuaObjects
    u16 m_uGetModInstance;
};

#endif
