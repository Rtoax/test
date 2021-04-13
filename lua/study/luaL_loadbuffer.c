#include <stdio.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
};
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
	char buff[256];
	int error;
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	while (fgets(buff, sizeof(buff), stdin) != NULL) {
		error = luaL_loadbuffer(L, buff, strlen(buff),
						"line") || lua_pcall(L, 0, 0, 0);
		if (error) {
			fprintf(stderr, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);/* pop error message from the stack */
		}
	}
	lua_close(L);
	return 0;
}
