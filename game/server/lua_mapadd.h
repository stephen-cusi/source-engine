#ifndef MAPADDLUA_H
#define MAPADDLUA_H

#include "cbase.h"
#include "coolmod/luamanager.h"

class MapAddLua : public LuaHandle
{
public:
	MapAddLua();
	~MapAddLua();

	void Init();
	void Shutdown();

	void OpenFile(const char* filename);
	void OpenCommon();
	void CallFunction(const char* function);

	void RegFunctions();
	void RegGlobals();

private:
	bool m_bLuaLoaded;
	bool m_bCommonLuaLoaded;
	unsigned m_szNewBuffer;
	unsigned m_szOldBuffer;

	unsigned m_szNewCommonBuffer;
	unsigned m_szOldCommonBuffer;
};

extern MapAddLua* g_LuaHandle;
#endif // !MAPADDLUA_H