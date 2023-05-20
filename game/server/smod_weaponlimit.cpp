//========= Copyright Msalinas2877, All rights reserved. ======================//
//
// Purpose: MapAdd Script System
//
//=============================================================================//
#include "cbase.h"
#include "smod_weaponlimit.h"
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

CWeaponLimit g_WeaponLimitParser("MapScriptParser");

CWeaponLimit *GetWeaponLimitParser()
{
	return &g_WeaponLimitParser;
}

bool CanPickUpWeapon(CHL2_Player *pPlayer, CBaseCombatWeapon *pWeapon)
{

	CWeaponLimit *parser = GetWeaponLimitParser();
	if (parser)
	{
		if (parser->m_pMapScript)
		{
			if (KeyValues *pCantCarry = parser->m_pMapScript->FindKey("CannotCarry"))
			{
				FOR_EACH_SUBKEY(pCantCarry, pWeaponInSlot)
				{
					for (int i = 0; i < pPlayer->WeaponCount(); i++)
					{
						const char *bumpWeapon = pWeapon->GetClassname();

						if (FStrEq(bumpWeapon, pWeaponInSlot->GetName()))
							return false;
					}
				}
			}
			FOR_EACH_SUBKEY(parser->m_pMapScript, pCategory)
			{
				if (FStrEq(pCategory->GetName(), "CannotCarry")) continue;

				FOR_EACH_SUBKEY(pCategory, pWeaponInSlot)
				{
					for (int i = 0; i < pPlayer->WeaponCount(); i++)
					{
						const char *bumpWeapon = pWeapon->GetClassname();
						const char *curWeapon = pPlayer->GetWeapon(i)->GetClassname();


					}
				}

			}
			return true;
		}
		return true;
	}
}

void CWeaponLimit::LevelInitPostEntity()
{
	char filename[FILENAME_MAX];

	Q_snprintf(filename, sizeof(filename), "scripts/%s.txt", "weapon_category");
	ParseScriptFile(filename);
}

void CWeaponLimit::ParseScriptFile(const char* filename, bool official)
{
	if (!m_pMapScript)
		m_pMapScript = new KeyValues("WeaponCategory");
	if (!m_pMapScript->LoadFromFile(filesystem, filename))
	{
		DebugColorMsg("Error: Could not load weapon_category file \n");
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
		return;
	}
}

void CWeaponLimit::LevelShutdownPostEntity()
{
	if (m_pMapScript)
	{
		m_pMapScript->deleteThis();
		m_pMapScript = nullptr;
	}
}

const char* CWeaponLimit::GetEntityNameFromFile(KeyValues *keyvalues)
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