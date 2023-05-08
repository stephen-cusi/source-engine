//========= Copyright Msalinas2877, All rights reserved. ======================//
//
// Purpose: MapAdd Script System
//
//=============================================================================//
#include "cbase.h"
#include "npc_replace_model.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DEBUG_MSG TRUE // Change this to "TRUE" to show Debug Info on Release Mode

#if DEBUG_MSG || DEBUG
#define DebugColorMsg(msg) ConColorMsg(Color(255, 124, 0, 255), msg)
#else
#define DebugColorMsg(msg)
#endif

//-----------------------------------------------------------------------------
// Class Override implementation.
//-----------------------------------------------------------------------------

const char *ReplaceModel(CBaseEntity *pEntity)
{
	char entityname = *pEntity->GetClassname();

	CReplaceModelParser *parser = GetReplaceModelParser();
	if (parser)
	{
		if (parser->m_pMapScript)
		{
			char *newName = NULL;

			KeyValues *pStandardMap = parser->m_pMapScript->FindKey("default_map");
			if (pStandardMap)
			{
				KeyValues *entity = pStandardMap->FindKey(entityname);
				if (entity)
				{
					newName = (char*)parser->GetEntityNameFromFile(entity);
				}
			}

			char mapname[FILENAME_MAX];
			Q_snprintf(mapname, sizeof(mapname), "%s", gpGlobals->mapname);
			KeyValues *pCurrentMap = parser->m_pMapScript->FindKey(mapname);
			if (pCurrentMap)
			{
				KeyValues *entity = pCurrentMap->FindKey(entityname);
				if (entity)
				{
					newName = (char*)parser->GetEntityNameFromFile(entity);
				}
			}
			if (newName)
			{
				pEntity->PrecacheModel(newName);
				UTIL_SetModel(pEntity, newName);
				return newName;
			}
			return STRING(pEntity->GetModelName());
		}
		return STRING(pEntity->GetModelName());
	}
	return STRING(pEntity->GetModelName());
}

CReplaceModelParser g_ReplaceModelParser("MapScriptParser");

CReplaceModelParser *GetReplaceModelParser()
{
	return &g_ReplaceModelParser;
}

void CReplaceModelParser::LevelInitPostEntity()
{
	char filename[FILENAME_MAX];

	Q_snprintf(filename, sizeof(filename), "scripts/%s.txt", "npc_replace_model");
	ParseScriptFile(filename);
}

void CReplaceModelParser::ParseScriptFile(const char* filename, bool official)
{
	if (!m_pMapScript)
		m_pMapScript = new KeyValues("NPCModelReplaceData");
	if (!m_pMapScript->LoadFromFile(filesystem, filename))
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
		return;
	}
}

void CReplaceModelParser::LevelShutdownPostEntity()
{
	if (m_pMapScript)
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
	}
}

const char* CReplaceModelParser::GetEntityNameFromFile(KeyValues *keyvalues)
{
	FOR_EACH_SUBKEY(keyvalues, pNewEntityModel)
	{
		int chance = pNewEntityModel->GetInt();
		if (RandomInt(1, 100 / chance) == 1)
		{
			return (const char*)pNewEntityModel->GetName();
			Msg("* %s->%s", keyvalues->GetName(), pNewEntityModel->GetName());
		}
		else
		{
			continue;
		}
	}
	return keyvalues->GetName();
}