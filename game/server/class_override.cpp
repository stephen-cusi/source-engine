//========= Copyright Msalinas2877, All rights reserved. ======================//
//
// Purpose: MapAdd Script System
//
//=============================================================================//
#include "cbase.h"
#include "class_override.h"
#include "filesystem.h"

#define DEBUG_MSG TRUE // Change this to "TRUE" to show Debug Info on Release Mode

#if DEBUG_MSG || DEBUG
#define DebugColorMsg(msg) ConColorMsg(Color(0, 124, 255, 255), msg)
#else
#define DebugColorMsg(msg)
#endif

ConVar disable_overrideentity("disable_overrideentity", "0");

//-----------------------------------------------------------------------------
// Class Override implementation.
//-----------------------------------------------------------------------------

const char *ReplaceEntity(const char *entityname)
{
	if (disable_overrideentity.GetBool())
		return entityname;

	CClassOverrideParser *parser = GetClassOverrideParser();
	if (parser)
	{
		if (parser->m_pMapScript)
		{
			char *newClassName = NULL;

			KeyValues *pStandardMap = parser->m_pMapScript->FindKey("default_map");
			if (pStandardMap)
			{
				KeyValues *entity = pStandardMap->FindKey(entityname);
				if (entity)
				{
					newClassName = (char*)parser->GetEntityNameFromFile(entity);
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
					newClassName = (char*)parser->GetEntityNameFromFile(entity);
				}
			}
			if (newClassName)
			{
				return newClassName;
				DevMsg("* %s->%s", entityname, newClassName);
			}
			else
			return entityname;
		}
		return entityname;
	}
	return entityname;
}

CClassOverrideParser g_ClassOverrideParser("MapScriptParser");

CClassOverrideParser *GetClassOverrideParser()
{
	return &g_ClassOverrideParser;
}

void CClassOverrideParser::LevelInitPreEntity()
{
	char filename[FILENAME_MAX];

	Q_snprintf(filename, sizeof(filename), "scripts/%s.txt", "override_class");
	ParseScriptFile(filename);
}

void CClassOverrideParser::ParseScriptFile(const char* filename, bool official)
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
		m_pMapScript = new KeyValues("OverrideClass");
	if (!m_pMapScript->LoadFromFile(filesystem, filename))
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
		return;
	}
}

void CClassOverrideParser::LevelShutdownPostEntity()
{
	if (m_pMapScript)
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
	}
}

const char* CClassOverrideParser::GetEntityNameFromFile(KeyValues *keyvalues)
{
	int totalChance = 0;
	FOR_EACH_SUBKEY(keyvalues, pNewEntityName)
	{
		int chance = pNewEntityName->GetInt();
		totalChance += chance;
		if (RandomInt(1, 100) <= chance)
		{
			return (const char*)pNewEntityName->GetName();
			Msg("* %s->%s", keyvalues->GetName(), pNewEntityName->GetName());
		}
		else
		{
			if (pNewEntityName->GetNextKey() == NULL && totalChance >= 100)
			{
				return (const char*)pNewEntityName->GetName();
				Msg("* %s->%s", keyvalues->GetName(), pNewEntityName->GetName());
			}
			continue;
		}
	}
	return keyvalues->GetName();
}