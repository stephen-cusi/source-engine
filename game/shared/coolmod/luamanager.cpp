///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : Server
//   File        : ge_luamanager.cpp
//   Description :
//      Updated for LUA 5.2
//
//   Created On: 3/5/2009 4:58:54 PM
//   Created By:  <mailto:admin[at]lodle[dot]net>
//   Created By:  <mailto:matt.shirleys[at]gmail[dot]com>
////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "luamanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CLUAManager gLuaMng;

CLUAManager* GetLuaManager()
{
	return &gLuaMng;
}

////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////

LuaHandle* g_NLuaHandle = NULL;

LuaHandle* GetNLuaHandle()
{
	return g_NLuaHandle;
}

LuaHandle::LuaHandle()
{
	pL = NULL;
}

LuaHandle::~LuaHandle()
{
	GetLuaManager()->DeRegisterLuaHandle(this);
}

void LuaHandle::InitDll()
{
	//Create an instance; Load the core libs.
	pL = luaL_newstate();
	luaopen_base(pL);		/* opens the basic library */
	luaopen_table(pL);		/* opens the table library */
	luaopen_string(pL);		/* opens the string lib. */
	luaopen_math(pL);		/* opens the math lib. */
#ifdef _DEBUG
	luaopen_debug(pL);
#endif
	Init();
}

void LuaHandle::ShutdownDll()
{
	Shutdown();
	lua_close(pL);
}

void LuaHandle::Register()
{
	GetLuaManager()->RegisterLuaHandle(this);
}

////////////////////////////////////////////////////////////////
// LuaManager
////////////////////////////////////////////////////////////////

CLUAManager::CLUAManager()
{
	m_bInit = false;
}

CLUAManager::~CLUAManager()
{

}

void CLUAManager::InitDll()
{
	// Register our LUA Functions and Globals so we can call them
	// from .lua scripts
	for (size_t x = 0; x<m_vHandles.size(); x++)
	{
		if (m_vHandles[x])
			continue;

		m_vHandles[x]->InitDll();
	}

	m_bInit = true;
	//ZMSLuaGamePlay *p = GetGP();
	//p->m_bLuaLoaded = true;
}

void CLUAManager::ShutdownDll()
{
	for (size_t x = 0; x<m_vHandles.size(); x++)
	{
		if (m_vHandles[x])
			continue;

		m_vHandles[x]->Shutdown();
	}

	m_vHandles.clear();
}


void CLUAManager::DeRegisterLuaHandle(LuaHandle* handle)
{
	if (!handle)
		return;

	for (size_t x = 0; x<m_vHandles.size(); x++)
	{
		if (m_vHandles[x] == handle)
			m_vHandles.erase(m_vHandles.begin() + x);
	}
}

void CLUAManager::RegisterLuaHandle(LuaHandle* handle)
{
	if (!handle)
		return;

	for (size_t x = 0; x<m_vHandles.size(); x++)
	{
		if (m_vHandles[x] == handle)
			return;
	}

	m_vHandles.push_back(handle);

	//if we are late to the game
	if (m_bInit)
		handle->InitDll();
}