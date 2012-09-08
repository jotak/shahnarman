#ifndef _LUA_CALLBACKS_H
#define _LUA_CALLBACKS_H

#include <lua5.1/lua.hpp>

#define SELECT_TYPE_UNIT              1
#define SELECT_TYPE_DEAD_UNIT         2
#define SELECT_TYPE_TOWN              3
#define SELECT_TYPE_BUILDING          4
#define SELECT_TYPE_SPELL_IN_PLAY     5
#define SELECT_TYPE_SPELL_IN_HAND     6
#define SELECT_TYPE_SPELL_IN_DECK     7
#define SELECT_TYPE_SPELL_IN_DISCARD  8
#define SELECT_TYPE_ANY_SPELL         9
#define SELECT_TYPE_TILE              10
#define SELECT_TYPE_PLAYER            11
#define SELECT_TYPE_TEMPLE            12

#define SELECT_CONSTRAINT_NONE        0x00000000
#define SELECT_CONSTRAINT_INRANGE     0x00000001
#define SELECT_CONSTRAINT_OWNED       0x00000002
#define SELECT_CONSTRAINT_OPPONENT    0x00000004
#define SELECT_CONSTRAINT_NOT_AVATAR  0x00000008
#define SELECT_CONSTRAINT_CUSTOM      0x00000010

extern void registerLuaCallbacks(lua_State * pState);

extern unsigned char getTargetTypeFromName(const char * sType);
extern unsigned char getTargetTypeFromName(const char * sType);

#endif
