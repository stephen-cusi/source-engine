//========= Copyright Msalinas2877, All rights reserved. ======================//
//
// Purpose: MapAdd Script System
//
//=============================================================================//
#include "cbase.h"
#include "map_parser.h"
#include "filesystem.h"
#include "ai_baseactor.h"
#include "ai_network.h"
#include "ai_node.h"

#include "smod_nodelistparser.h"

#include "lua_mapadd.h"

#define DEBUG_MSG FALSE // Change this to "TRUE" to show Debug Info on Release Mode

#if DEBUG_MSG || DEBUG
#define DebugColorMsg(msg) ConColorMsg(Color(124, 252, 0, 255), msg)
#else
#define DebugColorMsg(msg)
#endif

ConVar disable_loadmapadd("disable_loadmapadd", "0");

//Im lazy to put this on another file
class CEvent : public CLogicalEntity
{
public:
	DECLARE_CLASS(CEvent, CLogicalEntity)
	void Spawn()
	{
		SetNextThink(gpGlobals->curtime + m_flDelay);
	}
	void Think()
	{
		CBaseEntity *pEntity = gEntList.FindEntityByName(NULL, m_szTarget);
		if (pEntity)
			UTIL_Remove(pEntity);
	}

	string_t m_szTarget;
	string_t m_szInput;
	float m_flDelay;
};

LINK_ENTITY_TO_CLASS( event, CEvent )

CMapScriptParser::CMapScriptParser(char const *name) : CAutoGameSystem(name)
{
	m_bRestored = false;
}

CMapScriptParser g_MapScriptParser("MapScriptParser");

CMapScriptParser *GetMapScriptParser()
{
	return &g_MapScriptParser;
}

void CMapScriptParser::LevelInitPostEntity()
{
	if (m_bRestored)
		return;

	if (!g_LuaHandle)
		g_LuaHandle = new MapAddLua();

	char filename[FILENAME_MAX];
	//if (GetNodeListParser())
	{
		Q_snprintf(filename, sizeof(filename), "mapadd/nodes/%s.snl", gpGlobals->mapname);
		GetNodeListParser()->ParseScriptFile(filename);
	}

	Q_snprintf(filename, sizeof(filename), "mapadd/official/%s.txt", gpGlobals->mapname);
	ParseScriptFile(filename, true);

	Q_snprintf(filename, sizeof(filename), "mapadd/%s.txt", gpGlobals->mapname);
	ParseScriptFile(filename);
}

void CMapScriptParser::OnRestore()
{
	return;
}

void CMapScriptParser::ParseScriptFile(const char* filename, bool official)
{
	// Hold on a sec, two keyvalues to load 1 keyvalue file?
	// Yes, we need to do this because the old mapadd sintax is missing the "MainKey" and just uses SubKeys directly
	// MainKey <- Where mapadd scripts SHOULD start
	// {
	//		SubKey	<- Where old mapadd scripts really start		
	//		{
	//		}
	// }

	if (!m_pMapScript)
		m_pMapScript = new KeyValues("MapScript");
	KeyValues *m_pMapScriptMain;
	if (official)
		m_pMapScriptMain = new KeyValues("Official");
	else
		m_pMapScriptMain = new KeyValues("Unofficial");
	m_pMapScript->AddSubKey(m_pMapScriptMain);
	if (!m_pMapScriptMain->LoadFromFile(filesystem, filename))
	{
		if (official)
			DebugColorMsg("Error: Could not load official mapadd file \n");
		else
			DebugColorMsg("Error: Could not load mapadd file \n");
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
		return;
	}

	if (disable_loadmapadd.GetBool())
		return;

	KeyValues *pPrecache = m_pMapScript->FindKey("precache");
	if (pPrecache)
	{
		DebugColorMsg("Found precache Key \n");
		FOR_EACH_VALUE(pPrecache, pType)
		{
			if (!Q_strcmp(pType->GetName(), "entity"))
			{
				UTIL_PrecacheOther(pType->GetString());

				DebugColorMsg("Precaching Entity: ");
				DebugColorMsg(pType->GetString());
				DebugColorMsg("\n");
			}
			else if (!Q_strcmp(pType->GetName(), "model") || !Q_strcmp(pType->GetName(), "Model"))
			{
				CBaseEntity::PrecacheModel(pType->GetString());

				DebugColorMsg("Precaching Model: ");
				DebugColorMsg(pType->GetString());
				DebugColorMsg("\n");
			}
		}
	}

	KeyValues *pEntities = m_pMapScript->FindKey("entities");
	if (pEntities)
	{
		ParseEntities(pEntities);
	}

	KeyValues *pRandom = m_pMapScript->FindKey("randomspawn");
	if(pRandom)
	{
		ParseRandomEntities(pRandom);
	}
}

void CMapScriptParser::LevelShutdownPostEntity()
{
	if (m_pMapScript)
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
	}
}

void CMapScriptParser::ExecuteRandomEntites()
{
	KeyValues *pRandom = m_pMapScript->FindKey("randomspawn");
	if (pRandom)
	{
		ParseRandomEntities(pRandom);
	}
}

void CMapScriptParser::ParseRandomEntities(KeyValues *keyvalues)
{
	FOR_EACH_SUBKEY(keyvalues, pClassname)
	{
		if (!Q_strcmp(pClassname->GetName(), "removenodes"))
		{
			// no implementation for it now.
			continue;
		}
		if (!Q_strcmp(pClassname->GetName(), "removeairnodes"))
		{
			// no implementation for it now.
			continue;
		}


		int iCount = pClassname->GetInt("count", 0);
		for (int currentNumber = 0; currentNumber < iCount; currentNumber++)
		{
			if (!g_pBigAINet->GetNode(0))
				return;
			CBaseEntity *pEntity = CreateEntityByName(pClassname->GetName());
			if (pEntity)
			{
				Vector pos = g_pBigAINet->GetNode(RandomInt(1, g_pBigAINet->NumNodes() - 1))->GetPosition(HULL_HUMAN);

				const char *sWeapon = pClassname->GetString("weapon", NULL);
				if (sWeapon)
					pEntity->KeyValue("additionalequipment", sWeapon);
				const char *sModel = pClassname->GetString("model", NULL);
				if (sModel)
					pEntity->KeyValue("model", sModel);

				char key[512];
				char value[512];
				char tokenname[512];

				const char *pValue = pClassname->GetString("values", NULL);
				if (pValue)
				{
					pValue = nexttoken(tokenname, pValue, ' ');
					while (pValue && Q_strlen(tokenname) > 0)
					{
						Q_strcpy(key, tokenname);
						pValue = nexttoken(tokenname, pValue, ' ');
						Q_strcpy(value, tokenname);
						pEntity->KeyValue(key, value);
						pValue = nexttoken(tokenname, pValue, ' ');
					}
				}

				pEntity->SetAbsOrigin(pos);
				pEntity->Precache();
				DispatchSpawn(pEntity);
				pEntity->Activate();
			}
		}

	}
}

void CMapScriptParser::ParseEntities(KeyValues *keyvalues)
{
	DebugColorMsg("ParseEntities Called with: ");
	DebugColorMsg(keyvalues->GetName());
	DebugColorMsg("\n");
	FOR_EACH_SUBKEY(keyvalues, pClassname)
	{

		DebugColorMsg("Creating Entity: ");
		DebugColorMsg(pClassname->GetName());
		DebugColorMsg("\n");

		if (!Q_strcmp(pClassname->GetName(), "lua") && g_LuaHandle)
		{
			m_hasLua = true;
			g_LuaHandle->OpenFile(gpGlobals->mapname.ToCStr());
			g_LuaHandle->CallFunction(pClassname->GetString("callfunc", ""));
			continue;
		}
		else
		{
			m_hasLua = false;
		}

		if (!Q_strcmp(pClassname->GetName(), "player"))
		{
			FOR_EACH_VALUE(pClassname, value)
			{
				DebugColorMsg(value->GetName());
				if (!Q_strcmp(value->GetName(), "origin"))
				{
					Vector VecOrigin;
					UTIL_StringToVector(VecOrigin.Base(), value->GetString());

					UTIL_GetLocalPlayer()->SetAbsOrigin(VecOrigin);
				}
				else if (!Q_strcmp(value->GetName(), "angle"))
				{
					Vector VecAngle;
					UTIL_StringToVector(VecAngle.Base(), value->GetString());

					UTIL_GetLocalPlayer()->SetLocalAngles(QAngle(VecAngle.x, VecAngle.y, VecAngle.z));
				}
				else if (!Q_strcmp(value->GetName(), "message"))
				{
					UTIL_ShowMessage(value->GetString(), UTIL_GetLocalPlayer());
				}
				else if (!Q_strcmp(value->GetName(), "fadein"))
				{
					color32 black = { 0, 0, 0, 255 };
					UTIL_ScreenFade(UTIL_GetLocalPlayer(), black, 0, value->GetInt(), FFADE_IN);
				}
				else if (!Q_strcmp(value->GetName(), "fadeout"))
				{
					color32 black = { 32, 63, 100, 200 };
					UTIL_ScreenFade(UTIL_GetLocalPlayer(), black, value->GetInt(), 0.5, FFADE_OUT);
				}
				else if (!Q_strcmp(value->GetName(), "music"))
				{
					char play[FILENAME_MAX];
					Q_snprintf(play, sizeof(play), "playgamesound ", value->GetString());
					engine->ClientCommand(UTIL_GetLocalPlayer()->edict(), play);
				}
			}
			continue;
		}
		else if (!Q_strcmp(pClassname->GetName(), "removeentity"))
		{
			FOR_EACH_VALUE(pClassname, value)
			{
				CBaseEntity *pEntity = nullptr;
				if (!Q_strcmp(value->GetName(), "classname"))
				{
					pEntity = gEntList.FindEntityByName(NULL, value->GetString());
				}
				else if (!Q_strcmp(value->GetName(), "model") || !Q_strcmp(value->GetName(), "Model"))
				{
					pEntity = gEntList.FindEntityByModel(NULL, value->GetString());
				}
				if (pEntity)
					UTIL_Remove(pEntity);
			}
		}
		else if (!Q_strcmp(pClassname->GetName(), "event"))
		{
			CEvent *pEvent = (CEvent*)CreateEntityByName("event");
			if (pEvent)
			{
				FOR_EACH_VALUE(pClassname, value)
				{
					if (!Q_strcmp(value->GetName(), "targetname"))
					{
						pEvent->m_szTarget = AllocPooledString(value->GetString());
					}
					else if (!Q_strcmp(value->GetName(), "action"))
					{
						pEvent->m_szInput = AllocPooledString(value->GetString());
					}
					else if (!Q_strcmp(value->GetName(), "delaytime"))
					{
						pEvent->m_flDelay = value->GetFloat();
					}
				}
				pEvent->Precache();
				DispatchSpawn(pEvent);
				pEvent->Activate();
			}
		}

		CBaseEntity *pEntity = CreateEntityByName(pClassname->GetName());
		if (pEntity)
		{
			FOR_EACH_VALUE(pClassname, value)
			{
				if (!Q_strcmp(value->GetName(), "origin"))
				{
					Vector VecOrigin;
					UTIL_StringToVector(VecOrigin.Base(), value->GetString());

					pEntity->SetAbsOrigin(VecOrigin);
					DebugColorMsg("With Origin: ");
					DebugColorMsg(value->GetString());
					DebugColorMsg("\n");
				}
				else if (!Q_strcmp(value->GetName(), "angle"))
				{
					Vector VecAngle;
					UTIL_StringToVector(VecAngle.Base(), value->GetString());

					pEntity->SetAbsAngles(QAngle(VecAngle.x, VecAngle.y, VecAngle.z));
					DebugColorMsg("With Angles: ");
					DebugColorMsg(value->GetString());
					DebugColorMsg("\n");
				}
			}

			KeyValues *pKeyValues = pClassname->FindKey("keyvalues");
			if (pKeyValues)
			{
				FOR_EACH_VALUE(pKeyValues, pValues)
				{
					if (!Q_strcmp(pValues->GetName(), "model") || !Q_strcmp(pValues->GetName(), "Model"))
					{
						DebugColorMsg("With Model: ");
						DebugColorMsg(pValues->GetString());
						DebugColorMsg("\n");
						CBaseEntity::PrecacheModel(pValues->GetString());
						pEntity->SetModel(pValues->GetString());
						continue;
					}
					pEntity->KeyValue(pValues->GetName(), pValues->GetString());
				}
			}
			pEntity->Precache();
			DispatchSpawn(pEntity);
			pEntity->Activate();
		}
	}
}