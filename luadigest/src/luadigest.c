#include <string.h>
#include <polarssl/md4.h>
#include <polarssl/md5.h>
#include <polarssl/sha1.h>

#include "lua.h"
#include "lauxlib.h"

#define PASTER(x,y)	x ## y
#define EVALUATOR(x,y)	PASTER(x,y)

#define STR(s)		#s
#define XSTR(s)		STR(s)

#define LIB_NAME	XSTR(DIGEST)
#define LIB_VERSION	LIB_NAME " library for " LUA_VERSION
#define LIB_TYPE	LIB_NAME " context"

#define CTX		EVALUATOR(DIGEST, _context)
#define DIGEST_INIT	EVALUATOR(DIGEST, _starts)
#define DIGEST_UPDATE	EVALUATOR(DIGEST, _update)
#define DIGEST_FINISH	EVALUATOR(DIGEST, _finish)
#define LUAOPEN		EVALUATOR(luaopen_, DIGEST)

static CTX *Pget(lua_State *L, int i)
{
	return luaL_checkudata(L, i, LIB_TYPE);
}

static CTX *Pnew(lua_State *L)
{
	CTX 	*ctx = lua_newuserdata(L, sizeof(CTX));

	luaL_getmetatable(L, LIB_TYPE);
	lua_setmetatable(L, -2);
	return ctx;
}

static int Lnew(lua_State *L)			/** new() */
{
	CTX 	*ctx = Pnew(L);

	DIGEST_INIT(ctx);
	return 1;
}

static int Lclone(lua_State *L)			/** clone(c) */
{
	CTX 	*ctx = Pget(L, 1);
	CTX 	*n   = Pnew(L);

	*n = *ctx;
	return 1;
}

static int Lreset(lua_State *L)			/** reset(c) */
{
	CTX 	*ctx = Pget(L, 1);

	DIGEST_INIT(ctx);
	lua_settop(L, 1);
	return 1;
}

static int Lupdate(lua_State *L)		/** update(c,s,...) */
{
	CTX 	*ctx = Pget(L,1);
	int 	i, n=lua_gettop(L);

	for (i=2; i<=n; i++) {
		size_t l;
		const char *s = luaL_checklstring(L, i, &l);
		
		DIGEST_UPDATE(ctx, s, l);
	}
	lua_settop(L,1);
	return 1;
}

static int Ldigest(lua_State *L)		/** digest(c or s,[raw]) */
{
	unsigned char digest[DIGEST_LEN];

	if (lua_isuserdata(L,1))
	{
		CTX 		ctx = *Pget(L,1);
		
		DIGEST_FINISH(&ctx, digest);
	} else {
		CTX 		ctx;
		size_t		l;
		const char	*s = luaL_checklstring(L, 1, &l);

		DIGEST_INIT(&ctx);
		DIGEST_UPDATE(&ctx, s, l);
		DIGEST_FINISH(&ctx, digest);
	}

	if (lua_toboolean(L,2))
		lua_pushlstring(L,(char*)digest,sizeof(digest));
	else {
		char	*digit="0123456789abcdef";
		char	hex[2*sizeof(digest)], *h;
		int	i;

		for (h=hex,i=0; i<sizeof(digest); i++) {
			*h++=digit[digest[i] >> 4];
			*h++=digit[digest[i] & 0x0F];
		}
		lua_pushlstring(L,hex,sizeof(hex));
	}
	return 1;
}

static int Ltostring(lua_State *L)		/** __tostring(c) */
{
	CTX 	*ctx = Pget(L, 1);
	
	lua_pushfstring(L,"%s %p", LIB_TYPE, (void*)ctx);
	return 1;
}

static const luaL_Reg R[] =
{
	{ "__tostring",	Ltostring},
	{ "clone",	Lclone	},
	{ "digest",	Ldigest	},
	{ "new",	Lnew	},
	{ "reset",	Lreset	},
	{ "update",	Lupdate	},
	{ NULL,		NULL	}
};

LUALIB_API int LUAOPEN(lua_State *L)
{
	luaL_newmetatable(L, LIB_TYPE);
	lua_setglobal(L, LIB_NAME);
	luaL_register(L, LIB_NAME, R);
	lua_pushliteral(L, "version");
	lua_pushliteral(L, LIB_VERSION);
	lua_settable(L, -3);
	lua_pushliteral(L, "__index");
	lua_pushvalue(L, -2);
	lua_settable(L, -3);
	return 1;
}
