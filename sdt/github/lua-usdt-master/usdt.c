#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>
#include <usdt.h>
#include <string.h>

static int
provider(lua_State *L)
{
        const char *name = luaL_checklstring(L, 1, NULL);
        const char *module = luaL_checklstring(L, 2, NULL);
        usdt_provider_t **object = lua_newuserdata(L, sizeof(usdt_provider_t *));

        *object = usdt_create_provider(name, module);

        luaL_getmetatable(L, "usdt_provider_t");
        lua_setmetatable(L, -2);
        return 1;
}

static int
provider_probe(lua_State *L)
{
        usdt_provider_t **self = luaL_checkudata(L, 1, "usdt_provider_t");
        usdt_probedef_t **object;
        const char *name = luaL_checklstring(L, 2, NULL);
        const char *func = luaL_checklstring(L, 3, NULL);
        const char *argv[USDT_ARG_MAX];
        int i, argc = 0;

        for (i = 0; i < USDT_ARG_MAX; i++) {
                if (lua_isstring(L, i + 4)) {
                        argv[i] = luaL_checkstring(L, i + 4);
                        argc++;
                }
                else {
                        argv[i] = NULL;
                }
        }

        object = lua_newuserdata(L, sizeof(usdt_probedef_t *));
        *object = usdt_create_probe(name, func, argc, argv);
        usdt_provider_add_probe(*self, *object);

        luaL_getmetatable(L, "usdt_probedef_t");
        lua_setmetatable(L, -2);
        return 1;
}

static int
provider_enable(lua_State *L)
{
        usdt_provider_t **self = luaL_checkudata(L, 1, "usdt_provider_t");
        usdt_provider_enable(*self);
        return 0;
}

static int
provider_disable(lua_State *L)
{
        usdt_provider_t **self = luaL_checkudata(L, 1, "usdt_provider_t");
        usdt_provider_disable(*self);
        return 0;
}

static int
probedef_fire(lua_State *L)
{
        usdt_probedef_t **self = luaL_checkudata(L, 1, "usdt_probedef_t");
        usdt_probedef_t *probedef = *self;
        void *argv[USDT_ARG_MAX];
        int i;

        for (i = 0; i < probedef->argc; i++) {
                if (strncmp(probedef->types[i], "char *", 6) == 0) {
                        if (lua_isstring(L, i + 2))
                                argv[i] = (void *)luaL_checkstring(L, i + 2);
                        else
                                argv[i] = NULL;
                }
                else if (strncmp(probedef->types[i], "int", 3) == 0) {
                        if (lua_isnumber(L, i + 2))
                                argv[i] = (void *)luaL_checkinteger(L, i + 2);
                        else
                                argv[i] = NULL;
                }
                else {
                        argv[i] = NULL;
                }
        }

        usdt_fire_probe(probedef->probe, probedef->argc, argv);
        return 0;
}

static int
probedef_enabled(lua_State *L)
{
        usdt_probedef_t **self = luaL_checkudata(L, 1, "usdt_probedef_t");
        usdt_probedef_t *probedef = *self;
        int is = usdt_is_enabled(probedef->probe);
        lua_pushinteger(L, is);
        return 1;
}

static const luaL_reg usdt_functions[] = {
        {"provider", provider},
        {NULL, NULL}
};

static const luaL_reg usdt_provider_methods[] = {
        {"probe", provider_probe},
        {"enable", provider_enable},
        {"disable", provider_disable},
        {NULL, NULL}
};

static const luaL_reg usdt_probedef_methods[] = {
        {"fire", probedef_fire},
        {"enabled", probedef_enabled},
        {NULL, NULL}
};

int
luaopen_usdt(lua_State *L){
        luaL_newmetatable(L, "usdt_provider_t");
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        luaL_register(L, NULL, usdt_provider_methods);

        luaL_newmetatable(L, "usdt_probedef_t");
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
        luaL_register(L, NULL, usdt_probedef_methods);

        luaL_register(L, "usdt", usdt_functions);

        return 1;
}
