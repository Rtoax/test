#include <stdio.h>
#define MAX_COLOR       255 
#if 0
extern "C"
{
#include "lua-5.1/src/lauxlib.h"
#include "lua-5.1/src/lualib.h"
#include "lua-5.1/src/lstate.h"
}
#endif
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
};

int getfield(const char* key, lua_State* L)
{
    int result = 0;
    lua_pushstring(L, key);
    lua_gettable(L, -2);//以栈顶的值作为key来访问-2位置上的table。并将其值放入栈顶
    if (!lua_isnumber(L, -1))//将栈顶元素转换成double类型
    {
    }
    result = (int)lua_tonumber(L, -1) * MAX_COLOR;//lua_tonumber只是转换栈顶元素是不出栈的
    lua_pop(L, 1);//从栈中弹出1个元素
    return result;
}

int main(int argc, char *argv[])
{
    const char* buf = "background = {r=30, g=10, b=0} ";
    lua_State* L = luaL_newstate();
    luaL_dostring(L, buf);
    lua_getglobal(L, "background");//获取全局变量的background的值,并将其放入栈顶
    if (!lua_istable(L, -1))//判断是否是table
    {
        
    }
    int red = getfield("r", L);
    int green = getfield("g", L);
    int black = getfield("b", L);

    getchar();

    return 0;
}
