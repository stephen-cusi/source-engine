//========= Copyright Msalinas2877, All rights reserved. ======================//
//
// Purpose: MapAdd Script System
//
//=============================================================================//
#include "cbase.h"
#include "smod_weapon_randomize.h"
#include "filesystem.h"

#define DEBUG_MSG TRUE // Change this to "TRUE" to show Debug Info on Release Mode

#if DEBUG_MSG || DEBUG
#define DebugColorMsg(msg) ConColorMsg(Color(0, 124, 255, 255), msg)
#else
#define DebugColorMsg(msg)
#endif

ConVar npc_randomize_weapon("npc_randomize_weapon", "0");

//-----------------------------------------------------------------------------
// npc_weapon_randomize implementation.
//-----------------------------------------------------------------------------

const char *GiveRandomWeapon(CAI_BaseNPC *pNPC)
{
	//if (!npc_randomize_weapon.GetBool())
		return pNPC->GetActiveWeapon() ? pNPC->GetActiveWeapon()->GetClassname() : "0";

	CWeaponRandomize *parser = GetWeaponRandomizeParser();
	if (parser)
	{
		if (parser->m_pMapScript)
		{
			char *entityname = (char*)pNPC->GetClassname();
			const char *newWeapon = (char*)pNPC->GetActiveWeapon() ? pNPC->GetActiveWeapon()->GetClassname() : "0";

			KeyValues *pStandardMap = parser->m_pMapScript->FindKey("default_map");
			if (pStandardMap)
			{
				KeyValues *pStandardNPC = parser->m_pMapScript->FindKey("default_npc");
				if (pStandardNPC)
				{
					newWeapon = (char*)parser->GetEntityNameFromFile(pStandardNPC);
				}
				KeyValues *entity = pStandardMap->FindKey(entityname);
				if (entity)
				{
					newWeapon = (char*)parser->GetEntityNameFromFile(entity);
				}
			}

			char mapname[FILENAME_MAX];
			Q_snprintf(mapname, sizeof(mapname), "%s", gpGlobals->mapname);
			KeyValues *pCurrentMap = parser->m_pMapScript->FindKey(mapname);
			if (pCurrentMap)
			{
				KeyValues *pStandardNPC = parser->m_pMapScript->FindKey("default_npc");
				if (pStandardNPC)
				{
					newWeapon = (char*)parser->GetEntityNameFromFile(pStandardNPC);
				}
				KeyValues *entity = pCurrentMap->FindKey(entityname);
				if (entity)
				{
					newWeapon = (char*)parser->GetEntityNameFromFile(entity);
				}
			}
			if (newWeapon)
			{
				pNPC->KeyValue("additionalequipment", newWeapon);
				DevMsg("* %s->%s", pNPC->GetActiveWeapon() ? pNPC->GetActiveWeapon()->GetClassname() : "0", newWeapon);
				return newWeapon;
			}
			else
				return pNPC->GetActiveWeapon() ? pNPC->GetActiveWeapon()->GetClassname() : "0";
		}
		return pNPC->GetActiveWeapon() ? pNPC->GetActiveWeapon()->GetClassname() : "0";
	}
	return pNPC->GetActiveWeapon() ? pNPC->GetActiveWeapon()->GetClassname() : "0";
}

CWeaponRandomize g_WeaponRandomizeParser("MapScriptParser");

CWeaponRandomize *GetWeaponRandomizeParser()
{
	return &g_WeaponRandomizeParser;
}

void CWeaponRandomize::LevelInitPreEntity()
{
	char filename[FILENAME_MAX];


	Q_snprintf(filename, sizeof(filename), "scripts/%s.txt", "npc_weapon_randomize");
	ParseScriptFile(filename);
}

void CWeaponRandomize::ParseScriptFile(const char* filename, bool official)
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
		m_pMapScript = new KeyValues("WeaponRandomizeData");
	if (!m_pMapScript->LoadFromFile(filesystem, filename))
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
		return;
	}
}

void CWeaponRandomize::LevelShutdownPostEntity()
{
	if (m_pMapScript)
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
	}
}

const char* CWeaponRandomize::GetEntityNameFromFile(KeyValues *keyvalues)
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