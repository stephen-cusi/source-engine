#include "cbase.h"
#include "lua_mapadd.h"
#include "filesystem.h"
#include "fmtstr.h"

#include "map_parser.h"
#include "ai_node.h"
#include "ai_network.h"
#include "ai_basenpc.h"

#include "memdbgon.h"

#define DEBUG_MSG TRUE // Change this to "TRUE" to show Debug Info on Release Mode

ConVar mapadd_lua_debugmode("mapadd_lua_debugmode","0");

#if DEBUG_MSG || DEBUG
#define DebugColorMsg(msg) ConColorMsg(Color(0, 124, 252, 255), msg)
#else
#define DebugColorMsg(msg)
#endif

MapAddLua* g_LuaHandle;

static int luaprint(lua_State* L) {
	int nargs = lua_gettop(L);

	for (int i = 1; i <= nargs; i++) {
		if (lua_isstring(L, i)) {
			Msg("%s\n", lua_tostring(L, i)); /* Pop the next arg using lua_tostring(L, i) and do your print */
		}
		else {
			/* Do something with non-strings if you like */
		}
	}

	return 0;
}

static int luaCallMapaddLabel(lua_State* L) {
	int nargs = lua_gettop(L);

	for (int i = 1; i <= nargs; i++) {
		if (lua_isstring(L, i)) {
			char* buffer = strdup(lua_tostring(L, i));
			char* labels = strtok(buffer, ":");
			while (labels != NULL)
			{
				char labelname[FILENAME_MAX];
				Q_snprintf(labelname, sizeof(labelname), "entities:%s", labels);
				CMapScriptParser *parser = GetMapScriptParser();
				if (parser) {
					KeyValues* m_pLabel = parser->m_pMapScript->FindKey(labelname);
					if (m_pLabel)
						parser->ParseEntities(m_pLabel);
				}
				labels = strtok(NULL, ":");
			}
			free(buffer);
			Msg("%s\n", lua_tostring(L, i)); /* Pop the next arg using lua_tostring(L, i) and do your print */
		}
		else {
			/* Do something with non-strings if you like */
		}
	}

	return 0;
}

static int luaGetCurrentTime(lua_State* L) {

	lua_pushnumber(L, gpGlobals->curtime);

	return 1;
}

static int luaExecuteRandomSpawn(lua_State *L)
{
	GetMapScriptParser()->ExecuteRandomEntites();

	return 0;
}

static int luaExecuteRandomLabel(lua_State *L)
{
	char* buffer = strdup(lua_tostring(L, 1));
	char* labels = strtok(buffer, ":");
	while (labels != NULL)
	{
		char labelname[FILENAME_MAX];
		Q_snprintf(labelname, sizeof(labelname), "randomspawn:%s", labels);
		CMapScriptParser *parser = GetMapScriptParser();
		if (parser) {
			KeyValues* m_pLabel = parser->m_pMapScript->FindKey(labelname);
			if (m_pLabel)
				parser->ParseRandomEntities(m_pLabel);
		}
		labels = strtok(NULL, ":");
	}
	free(buffer);

	return 0;
}

static int luaRandomInt(lua_State *L)
{
	int nargs = lua_gettop(L);
	float randomInt;
	if (nargs == 1)
		randomInt = RandomInt(1, lua_tointeger(L, 1));
	else if(nargs == 2)
		randomInt = RandomInt(lua_tointeger(L, 1), lua_tointeger(L, 2));
	else
		randomInt = RandomFloat(0.0f, 1.0f);

	lua_pushnumber(L, randomInt);

	return 1;
}

static int luaRandomFloat(lua_State *L)
{
	int nargs = lua_gettop(L);
	float randomInt;
	if (nargs == 1)
		randomInt = RandomFloat(0.0f, lua_tointeger(L, 1));
	else if (nargs == 2)
		randomInt = RandomFloat(lua_tointeger(L, 1), lua_tointeger(L, 2));
	else
		randomInt = RandomFloat(0.0f, 1.0f);

	lua_pushnumber(L, randomInt);

	return 1;
}

static int luaChangeLevel(lua_State* L) {
	int nargs = lua_gettop(L);

	for (int i = 1; i <= nargs; i++) {
		if (lua_isstring(L, i)) {
			engine->ChangeLevel(lua_tostring(L, i), NULL); /* Pop the next arg using lua_tostring(L, i) and do your print */
		}
		else {
			/* Do something with non-strings if you like */
		}
	}

	return 0;
}

static int luaCreateEntity(lua_State *l)
{
	int nargs = lua_gettop(l);
	const char * name = luaL_checkstring(l, 1);

	CBaseEntity *udata = CreateEntityByName(name);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaCreateEntity Created NULL Entity\n"));
		return 0;
	}
	Vector *pos = (Vector *)lua_touserdata(l, 2);
	if (!pos)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaCreateEntity is trying to use NULL Origin Vector\n"));
		return 0;
	}
	Vector *vecAng = (Vector *)lua_touserdata(l, 3);
	if (!vecAng)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaCreateEntity is trying to use NULL Angle Vector\n"));
		return 0;
	}

	QAngle ang = QAngle(vecAng->x, vecAng->y, vecAng->z);
	if (nargs >= 2)
		udata->SetAbsOrigin(*pos);
	if (nargs >= 3)
		udata->SetAbsAngles(ang);

	lua_pushlightuserdata(l, udata);

	return 1;
}

static int luaEntitySetAbsPos(lua_State *l)
{
	if (!lua_isuserdata(l, 1))
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] %s is not an object\n", lua_tostring(l, 1)));
		return 0;
	}

	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaEntitySetAbsPos is trying to use NULL Entity\n"));
		return 0;
	}
	Vector *pos = (Vector *)lua_touserdata(l, 2);
	if (!pos)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaEntitySetAbsPos is trying to use NULL Vector\n"));
		return 0;
	}
	udata->SetAbsOrigin(*pos);

	return 0;
}

static int luaSpawnEntity(lua_State *l)
{
	if (!lua_isuserdata(l, 1))
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] %s is not an object\n", lua_tostring(l, 1)));
		return 0;
	}

	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	udata->Spawn();

	return 1;
}

static int luaVector(lua_State *l)
{
	float x = lua_tonumber(l,1);
	float y = lua_tonumber(l, 2);
	float z = lua_tonumber(l, 3);

	Vector *position = new Vector(x,y,z);

	lua_pushlightuserdata(l, position);
	return 1;
}

static int luaFindEntityByName(lua_State *l)
{

	CBaseEntity *udata = gEntList.FindEntityByName((CBaseEntity *)lua_touserdata(l,1),lua_tostring(l, 2));

	if (!udata)
	{
		DebugColorMsg(CFmtStr("[SOURCE-ERR] luaFindEntityByName found NULL Entity\n"));
	}

	lua_pushlightuserdata(l,udata);

	return 1;
}

static int luaFindEntityByClass(lua_State *l)
{
	CBaseEntity *udata = gEntList.FindEntityByClassname((CBaseEntity *)lua_touserdata(l, 1), lua_tostring(l, 2));

	if (!udata)
	{
		DebugColorMsg(CFmtStr("[SOURCE-ERR] luaFindEntityByClass found NULL Entity\n"));
	}

	lua_pushlightuserdata(l, udata);

	return 1;
}

static int luaKeyValue(lua_State *l)
{
	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	
	if (udata)
		udata->KeyValue(lua_tostring(l, 2), lua_tostring(l, 3));
	else
		Warning("[SOURCE-ERR] luaKeyValue got NULL Entity\n");

	return 0;
}

static int luaGetAbsOrigin(lua_State *l)
{

	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	if(!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaGetAbsOrigin is trying to use NULL Entity\n"));
	return 0;
	}
	Vector *position = new Vector(udata->GetAbsOrigin());
	if (!position)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaGetAbsOrigin is trying to use NULL Vector\n"));
		return 0;
	}

	lua_pushlightuserdata(l, position);

	return 1;
}

static int luaGetAbsVelocity(lua_State *l)
{

	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaGetAbsVelocity is trying to use NULL Entity\n"));
		return 0;
	}
	Vector *velocity = new Vector(udata->GetAbsVelocity());
	if (!velocity)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaGetAbsVelocity is trying to use NULL Vector\n"));
		return 0;
	}

	lua_pushlightuserdata(l, velocity);

	return 1;
}

static int luaSetAbsVelocity(lua_State *l)
{

	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetAbsVelocity is trying to use NULL Entity\n"));
		return 0;
	}
	Vector *velocity = (Vector *)lua_touserdata(l, 2);
	if (!velocity)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetAbsVelocity is trying to use NULL Vector\n"));
		return 0;
	}

	udata->SetAbsVelocity(*velocity);

	return 0;
}

static int luaSetAbsAngles(lua_State *l)
{
	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetAbsAngles is trying to use NULL Entity\n"));
		return 0;
	}
	Vector *angle = (Vector *)lua_touserdata(l, 2);
	if (!angle)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetAbsAngles is trying to use NULL Vector\n"));
		return 0;
	}

	udata->SetAbsAngles(QAngle(angle->x, angle->y, angle->z));

	return 0;
}

static int luaGetAbsAngles(lua_State *l)
{

	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaGetAbsAngles is trying to use NULL Entity\n"));
		return 0;
	}
	Vector *angle = new Vector(udata->GetAbsAngles().x, udata->GetAbsAngles().y, udata->GetAbsAngles().z);
	if (!angle)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaGetAbsAngles is trying to use NULL Vector\n"));
		return 0;
	}

	lua_pushlightuserdata(l, angle);

	return 1;
}

static int luaSetEntityPhysFreeze(lua_State *l)
{
	CBaseEntity *udata = (CBaseEntity *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetEntityPhysFreeze is trying to use NULL Entity\n"));
		return 0;
	}

	if (!udata->VPhysicsGetObject())
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetEntityPhysFreeze Entity Dont have VPhysicsObject! \n"));
		return 0;
	}
	if(Q_strcmp(lua_tostring(l, 2), "freeze"))
		udata->VPhysicsGetObject()->EnableMotion(false);
	if (Q_strcmp(lua_tostring(l, 2), "unfreeze"))
		udata->VPhysicsGetObject()->EnableMotion(false);

	return 0;
}

static int luaGetPlayer(lua_State *l)
{
	CBaseEntity *udata = UTIL_GetLocalPlayer();

	if (!udata)
	{
		DebugColorMsg(CFmtStr("[SOURCE-ERR] luaFindEntityByName found NULL Entity\n"));
		lua_pushlightuserdata(l, udata);
		return 1;

	}
	lua_pushlightuserdata(l, udata);

	return 1;
}

static int luaSetEntityRelationShip(lua_State *l)
{
	CAI_BaseNPC *udata = (CAI_BaseNPC *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetEntityRelationShip is trying to use NULL Entity\n"));
		return 0;
	}
	DebugColorMsg(CFmtStr("[SOURCE-ERR] luaSetEntityRelationShip Doesnt have an implementation\n"));

	// not now
	/*CBaseEntity *entity = gEntList.FindEntityByName(NULL, udata->GetClassname());
	while (entity)
	{
		udata->AddEntityRelationship(entity, disposition, priority);
		entity = gEntList.FindEntityByName(entity, udata->GetClassname());
	}*/
	

	return 0;
}

static int luaSetEntityRelationShip2(lua_State *l)
{
	CAI_BaseNPC *udata = (CAI_BaseNPC *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetEntityRelationShip2 is trying to use NULL Entity\n"));
		return 0;
	}
	DebugColorMsg(CFmtStr("[SOURCE-ERR] luaSetEntityRelationShip2 Doesnt have an implementation\n"));

	// not now
	/*CBaseEntity *entity = gEntList.FindEntityByName(NULL, udata->GetClassname());
	while (entity)
	{
	udata->AddEntityRelationship(entity, disposition, priority);
	entity = gEntList.FindEntityByName(entity, udata->GetClassname());
	}*/


	return 0;
}

static int luaSetEntityRelationShipName(lua_State *l)
{
	CAI_BaseNPC *udata = (CAI_BaseNPC *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetEntityRelationShipName is trying to use NULL Entity\n"));
		return 0;
	}
	DebugColorMsg(CFmtStr("[SOURCE-ERR] luaSetEntityRelationShipName Doesnt have an implementation\n"));

	// not now
	/*CBaseEntity *entity = gEntList.FindEntityByName(NULL, udata->GetClassname());
	while (entity)
	{
	udata->AddEntityRelationship(entity, disposition, priority);
	entity = gEntList.FindEntityByName(entity, udata->GetClassname());
	}*/


	return 0;
}

static int luaSetEntityRelationShipName2(lua_State *l)
{
	CAI_BaseNPC *udata = (CAI_BaseNPC *)lua_touserdata(l, 1);
	if (!udata)
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] luaSetEntityRelationShipName2 is trying to use NULL Entity\n"));
		return 0;
	}
	DebugColorMsg(CFmtStr("[SOURCE-ERR] luaSetEntityRelationShipName2 Doesnt have an implementation\n"));

	// not now
	/*CBaseEntity *entity = gEntList.FindEntityByName(NULL, udata->GetClassname());
	while (entity)
	{
	udata->AddEntityRelationship(entity, disposition, priority);
	entity = gEntList.FindEntityByName(entity, udata->GetClassname());
	}*/


	return 0;
}

static int luaGetNodeCounts(lua_State *l)
{
	int nodes = g_pBigAINet->NumNodes();

	lua_pushinteger(l, nodes);

	return 1;
}

void RegisterHL2(lua_State * l)
{
	luaL_Reg sHL2Regs[] =
	{
	{ "CallMapaddLabel", luaCallMapaddLabel },
	{ "ChangeLevel", luaChangeLevel },
	{ "GetCurrentTime", luaGetCurrentTime},
	{ "ExecuteRandomSpawn", luaExecuteRandomSpawn },
	{ "ExecuteRandomSpawnLabel", luaExecuteRandomLabel },
	{ "RandomInt", luaRandomInt },
	{ "RandomFloat", luaRandomFloat },
	{ "CreateEntity",luaCreateEntity },
	{ "SpawnEntity",luaSpawnEntity },
	{ "Vector", luaVector},
	{ "GetPlayer", luaGetPlayer },
	{ "SetEntityAbsOrigin", luaEntitySetAbsPos },
	{ "GetEntityAbsOrigin", luaGetAbsOrigin },
	{ "SetEntityPhysVelocity", luaSetAbsVelocity },
	{ "GetEntityPhysVelocity", luaGetAbsVelocity },
	{ "SetEntityAbsAngles", luaSetAbsAngles },
	{ "GetEntityAbsAngles", luaGetAbsAngles },
	{ "SetEntityPhysFreeze", luaSetEntityPhysFreeze },
	{ "FindEntityByName", luaFindEntityByName },
	{ "FindEntityByClass", luaFindEntityByClass },
	{ "GetNodeCounts", luaGetNodeCounts},
	{ "SetEntityRelationship", luaSetEntityRelationShip },
	{ "SetEntityRelationship2", luaSetEntityRelationShip2 },
	{ "SetEntityRelationshipName", luaSetEntityRelationShipName },
	{ "SetEntityRelationshipName2", luaSetEntityRelationShipName2 },
	{ "KeyValue", luaKeyValue },
	{ NULL, NULL }
	};

	lua_newtable(l);

	luaL_setfuncs(l, sHL2Regs, 0);
	lua_pushvalue(l, -1);

	lua_setfield(l, -1, "__index");
	lua_setglobal(l, "HL2");
}

MapAddLua::MapAddLua() : LuaHandle()
{
	m_bCommonLuaLoaded = false;
	Register();
}

MapAddLua::~MapAddLua()
{
}

void MapAddLua::Init()
{
}

void MapAddLua::Shutdown()
{
}

void MapAddLua::OpenCommon()
{
	//Load common.lua into buffer
	char commonpath[_MAX_PATH];
	Q_snprintf(commonpath, sizeof(commonpath), "scripts/%s.lua", "common");


	FileHandle_t cf = filesystem->Open(commonpath, "rb", "MOD");
	if (!cf)
		return;

	// load file into a null-terminated buffer
	int commonfileSize = filesystem->Size(cf);
	unsigned commonbufSize = ((IFileSystem *)filesystem)->GetOptimalReadSize(cf, commonfileSize + 1);

	char *commonbuffer = (char*)((IFileSystem *)filesystem)->AllocOptimalReadBuffer(cf, commonbufSize);
	Assert(buffer);

	((IFileSystem *)filesystem)->ReadEx(commonbuffer, commonbufSize, commonfileSize, cf); // read into local buffer
	commonbuffer[commonfileSize] = '\0'; // null terminate file as EOF
	filesystem->Close(cf);	// close file after reading

	m_szOldCommonBuffer = *commonbuffer;
	if (m_szOldCommonBuffer == m_szNewCommonBuffer)
		return;

	m_szNewCommonBuffer = m_szOldCommonBuffer;

	if (luaL_loadbufferx(GetLua(), commonbuffer, commonfileSize, commonpath, NULL))
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] %s\n", lua_tostring(GetLua(), -1)));
		lua_pop(GetLua(), 1);
		DebugColorMsg(CFmtStr("[LUA-ERR] One or more errors occured while loading lua script!\n"));
		return;
	}
	else
	{
		DebugColorMsg(CFmtStr("[LUA] Loaded file %s\n", commonpath));
		if (mapadd_lua_debugmode.GetInt() >= 1)
			DebugColorMsg(CFmtStr("[LUA-DEBUG]\n%s \n", commonbuffer));
	}
	CallLUA(GetLua(), 0, LUA_MULTRET, 0, commonpath);
	m_bCommonLuaLoaded = true;
}

void MapAddLua::OpenFile(const char* filename)
{
	if(!m_bCommonLuaLoaded)
		OpenCommon();

	//Load into buffer
	char path[_MAX_PATH];
	Q_snprintf(path, sizeof(path), "mapadd/%s.lua", filename);


	FileHandle_t f = filesystem->Open(path, "rb", "MOD");
	if (!f)
		return;

	// load file into a null-terminated buffer
	int fileSize = filesystem->Size(f);
	unsigned bufSize = ((IFileSystem *)filesystem)->GetOptimalReadSize(f, fileSize + 1);

	char *buffer = (char*)((IFileSystem *)filesystem)->AllocOptimalReadBuffer(f, bufSize);
	Assert(buffer);

	((IFileSystem *)filesystem)->ReadEx(buffer, bufSize, fileSize, f); // read into local buffer
	buffer[fileSize] = '\0'; // null terminate file as EOF
	filesystem->Close(f);	// close file after reading

	m_szOldBuffer = *buffer;
	if (m_szOldBuffer == m_szNewBuffer)
		return;

	m_szNewBuffer = m_szOldBuffer;

	if (luaL_loadbuffer(GetLua(), buffer, fileSize, path))
	{
		DebugColorMsg(CFmtStr("[LUA-ERR] %s\n", lua_tostring(GetLua(), -1)));
		lua_pop(GetLua(), 1);  
		DebugColorMsg(CFmtStr("[LUA-ERR] One or more errors occured while loading lua script!\n"));
		return;
	}
	else
	{
		DebugColorMsg(CFmtStr("[LUA] Loaded file %s\n", path));
		if (mapadd_lua_debugmode.GetInt() >= 1)
			DebugColorMsg(CFmtStr("[LUA-DEBUG]\n%s \n", buffer));
	}
	CallLUA(GetLua(), 0, LUA_MULTRET, 0, path);
	m_bLuaLoaded = true;
}

void MapAddLua::CallFunction(const char *function)
{
	lua_getglobal(GetLua(), function);
	//CallLUA(GetLua(), 0, LUA_MULTRET, 0, function);
	int error = lua_pcall(GetLua(), 0, LUA_MULTRET, 0);
	switch (error)
	{
	case LUA_ERRRUN:
		DebugColorMsg(CFmtStr("[LUA-ERR] runtime error\n"));
		DebugColorMsg(CFmtStr("[LUA-ERR] Error running function \"%s\": %s\n", function, lua_tostring(GetLua(), -1)));
		break;
	case LUA_ERRMEM:
		DebugColorMsg(CFmtStr("[LUA-ERR] memory allocation \n"));
		DebugColorMsg(CFmtStr("[LUA-ERR] Error running function \"%s\": %s\n", function, lua_tostring(GetLua(), -1)));
		break;
	case LUA_ERRERR:
		DebugColorMsg(CFmtStr("[LUA-ERR] error while running the error handler function \n"));
		DebugColorMsg(CFmtStr("[LUA-ERR] Error running function \"%s\": %s\n", function, lua_tostring(GetLua(), -1)));
		break;
	default:
		break;
	}
	lua_pop(GetLua(), 1);
}

void MapAddLua::RegFunctions()
{
}

void MapAddLua::RegGlobals()
{
	lua_getglobal(GetLua(), "_G");
	lua_register(GetLua(),"print", luaprint);
	lua_pop(GetLua(), 1);

	RegisterHL2(GetLua());
}