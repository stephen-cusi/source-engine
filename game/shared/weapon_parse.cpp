//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Weapon data file parsing, shared by game & client dlls.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include <KeyValues.h>
#include <tier0/mem.h>
#include "filesystem.h"
#include "utldict.h"
#include "ammodef.h"
#if defined(CLIENT_DLL)
#include "hl2/c_basehlcombatweapon.h"
#include "smmod/c_weapon_custom.h"
#include "c_baseentity.h"
#else
#include "smmod/weapon_custom.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// The sound categories found in the weapon classname.txt files
// This needs to match the WeaponSound_t enum in weapon_parse.h
#if !defined(_STATIC_LINKED) || defined(CLIENT_DLL)
const char *pWeaponSoundCategories[NUM_SHOOT_SOUND_TYPES] =
{
	"empty",
	"single_shot",
	"single_shot_npc",
	"double_shot",
	"double_shot_npc",
	"burst",
	"reload",
	"reload_npc",
	"melee_miss",
	"melee_hit",
	"melee_hit_world",
	"special1",
	"special2",
	"special3",
	"taunt",
	"deploy"
};
#else
extern const char *pWeaponSoundCategories[NUM_SHOOT_SOUND_TYPES];
#endif

int GetWeaponSoundFromString(const char *pszString)
{
	for (int i = EMPTY; i < NUM_SHOOT_SOUND_TYPES; i++)
	{
		if (!Q_stricmp(pszString, pWeaponSoundCategories[i]))
			return (WeaponSound_t)i;
	}
	return -1;
}

// aaa i am retard
Activity StringToVMActivity(const char *szChar)
{
	if ( 0 != Q_stristr(szChar, "DRAW"))
		return ACT_VM_DRAW;
	else if ( 0 != Q_stristr(szChar, "HOLSTER"))
		return ACT_VM_HOLSTER;
	else if ( 0 != Q_stristr(szChar, "IDLE"))
		return ACT_VM_IDLE;
	else if ( 0 != Q_stristr(szChar, "FIDGET"))
		return ACT_VM_FIDGET;
	else if ( 0 != Q_stristr(szChar, "PULLBACK"))
		return ACT_VM_PULLBACK;
	else if ( 0 != Q_stristr(szChar, "PULLBACK_HIGH"))
		return ACT_VM_PULLBACK_HIGH;
	else if ( 0 != Q_stristr(szChar, "PULLPIN"))
		return ACT_VM_PULLPIN;
	else if ( 0 != Q_stristr(szChar, "PRIMARYATTACK"))
		return ACT_VM_PRIMARYATTACK;
	else if ( 0 != Q_stristr(szChar, "SECONDARYATTACK"))
		return ACT_VM_SECONDARYATTACK;
	else if ( 0 != Q_stristr(szChar, "RELOAD"))
		return ACT_VM_RELOAD;
	else if ( 0 != Q_stristr(szChar, "RELOAD_START"))
		return ACT_VM_RELOAD_START;
	else if ( 0 != Q_stristr(szChar, "RELOAD_FINISH"))
		return ACT_VM_RELOAD_FINISH;
	else if ( 0 != Q_stristr(szChar, "DRYFIRE"))
		return ACT_VM_DRYFIRE;
	else if ( 0 != Q_stristr(szChar, "HITLEFT"))
		return ACT_VM_HITLEFT;
	else if ( 0 != Q_stristr(szChar, "HITLEFT2"))
		return ACT_VM_HITLEFT2;
	else if ( 0 != Q_stristr(szChar, "HITRIGHT"))
		return ACT_VM_HITRIGHT;
	else if ( 0 != Q_stristr(szChar, "HITRIGHT2"))
		return ACT_VM_HITRIGHT2;
	else if ( 0 != Q_stristr(szChar, "HITCENTER"))
		return ACT_VM_HITCENTER;
	else if ( 0 != Q_stristr(szChar, "HITCENTER2"))
		return ACT_VM_HITCENTER2;
	else if ( 0 != Q_stristr(szChar, "MISSLEFT"))
		return ACT_VM_MISSLEFT;
	else if ( 0 != Q_stristr(szChar, "MISSLEFT2"))
		return ACT_VM_MISSLEFT2;
	else if ( 0 != Q_stristr(szChar, "MISSRIGHT"))
		return ACT_VM_MISSRIGHT;
	else if ( 0 != Q_stristr(szChar, "MISSRIGHT2"))
		return ACT_VM_MISSRIGHT2;
	else if ( 0 != Q_stristr(szChar, "MISSCENTER"))
		return ACT_VM_MISSCENTER;
	else if ( 0 != Q_stristr(szChar, "MISSCENTER2"))
		return ACT_VM_MISSCENTER2;
	else if ( 0 != Q_stristr(szChar, "MISSCENTER2"))
		return ACT_VM_MISSCENTER2;
	else if ( 0 != Q_stristr(szChar, "HAULBACK"))
		return ACT_VM_HAULBACK;
	else if ( 0 != Q_stristr(szChar, "SWINGHARD"))
		return ACT_VM_SWINGHARD;
	else if ( 0 != Q_stristr(szChar, "SWINGMISS"))
		return ACT_VM_SWINGMISS;
	else if ( 0 != Q_stristr(szChar, "SWINGHIT"))
		return ACT_VM_SWINGHIT;
	else if ( 0 != Q_stristr(szChar, "IDLE_TO_LOWERED"))
		return ACT_VM_IDLE_TO_LOWERED;
	else if ( 0 != Q_stristr(szChar, "IDLE_LOWERED"))
		return ACT_VM_IDLE_LOWERED;
	else if ( 0 != Q_stristr(szChar, "LOWERED_TO_IDLE"))
		return ACT_VM_LOWERED_TO_IDLE;
	else if ( 0 != Q_stristr(szChar, "RECOIL1"))
		return ACT_VM_RECOIL1;
	else if ( 0 != Q_stristr(szChar, "RECOIL2"))
		return ACT_VM_RECOIL2;
	else if ( 0 != Q_stristr(szChar, "RECOIL3"))
		return ACT_VM_RECOIL3;
	else if ( 0 != Q_stristr(szChar, "PICKUP"))
		return ACT_VM_PICKUP;
	else if ( 0 != Q_stristr(szChar, "RELEASE"))
		return ACT_VM_RELEASE;
	else if ( 0 != Q_stristr(szChar, "ATTACH_SILENCER"))
		return ACT_VM_ATTACH_SILENCER;
	else if ( 0 != Q_stristr(szChar, "DETACH_SILENCER"))
		return ACT_VM_DETACH_SILENCER;
	else if ( 0 != Q_stristr(szChar, "DRYFIRE"))
		return ACT_VM_DRYFIRE;
	else
		return ACT_VM_IDLE;
}


// Item flags that we parse out of the file.
typedef struct
{
	const char *m_pFlagName;
	int m_iFlagValue;
} itemFlags_t;
#if !defined(_STATIC_LINKED) || defined(CLIENT_DLL)
itemFlags_t g_ItemFlags[8] =
{
	{ "ITEM_FLAG_SELECTONEMPTY",	ITEM_FLAG_SELECTONEMPTY },
	{ "ITEM_FLAG_NOAUTORELOAD",		ITEM_FLAG_NOAUTORELOAD },
	{ "ITEM_FLAG_NOAUTOSWITCHEMPTY", ITEM_FLAG_NOAUTOSWITCHEMPTY },
	{ "ITEM_FLAG_LIMITINWORLD",		ITEM_FLAG_LIMITINWORLD },
	{ "ITEM_FLAG_EXHAUSTIBLE",		ITEM_FLAG_EXHAUSTIBLE },
	{ "ITEM_FLAG_DOHITLOCATIONDMG", ITEM_FLAG_DOHITLOCATIONDMG },
	{ "ITEM_FLAG_NOAMMOPICKUPS",	ITEM_FLAG_NOAMMOPICKUPS },
	{ "ITEM_FLAG_NOITEMPICKUP",		ITEM_FLAG_NOITEMPICKUP }
};
#else
extern itemFlags_t g_ItemFlags[7];
#endif


static CUtlDict< FileWeaponInfo_t*, unsigned short > m_WeaponInfoDatabase;

#ifdef _DEBUG
// used to track whether or not two weapons have been mistakenly assigned the wrong slot
bool g_bUsedWeaponSlots[MAX_WEAPON_SLOTS][MAX_WEAPON_POSITIONS] = { 0 };
#endif

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : FileWeaponInfo_t
//-----------------------------------------------------------------------------
static WEAPON_FILE_INFO_HANDLE FindWeaponInfoSlot(const char *name)
{
	// Complain about duplicately defined metaclass names...
	unsigned short lookup = m_WeaponInfoDatabase.Find(name);
	if (lookup != m_WeaponInfoDatabase.InvalidIndex())
	{
		return lookup;
	}

	FileWeaponInfo_t *insert = CreateWeaponInfo();

	lookup = m_WeaponInfoDatabase.Insert(name, insert);
	Assert(lookup != m_WeaponInfoDatabase.InvalidIndex());
	return lookup;
}

// Find a weapon slot, assuming the weapon's data has already been loaded.
WEAPON_FILE_INFO_HANDLE LookupWeaponInfoSlot(const char *name)
{
	return m_WeaponInfoDatabase.Find(name);
}

// FIXME, handle differently?
static FileWeaponInfo_t gNullWeaponInfo;

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : handle - 
// Output : FileWeaponInfo_t
//-----------------------------------------------------------------------------
FileWeaponInfo_t *GetFileWeaponInfoFromHandle(WEAPON_FILE_INFO_HANDLE handle)
{
	if (handle < 0 || handle >= m_WeaponInfoDatabase.Count())
	{
		return &gNullWeaponInfo;
	}

	if (handle == m_WeaponInfoDatabase.InvalidIndex())
	{
		return &gNullWeaponInfo;
	}

	return m_WeaponInfoDatabase[handle];
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : WEAPON_FILE_INFO_HANDLE
//-----------------------------------------------------------------------------
WEAPON_FILE_INFO_HANDLE GetInvalidWeaponInfoHandle(void)
{
	return (WEAPON_FILE_INFO_HANDLE)m_WeaponInfoDatabase.InvalidIndex();
}

#if 0
void ResetFileWeaponInfoDatabase(void)
{
	int c = m_WeaponInfoDatabase.Count();
	for (int i = 0; i < c; ++i)
	{
		delete m_WeaponInfoDatabase[i];
	}
	m_WeaponInfoDatabase.RemoveAll();

#ifdef _DEBUG
	memset(g_bUsedWeaponSlots, 0, sizeof(g_bUsedWeaponSlots));
#endif
}
#endif

#ifdef CLIENT_DLL


#define CWeaponCustom C_WeaponCustom 
static C_BaseEntity *CCHL2MPScriptedWeaponFactory(void)
{
	return static_cast< C_BaseEntity * >(new CWeaponCustom);
};
#endif

#ifndef CLIENT_DLL
static CUtlDict< CEntityFactory<CWeaponCustom>*, unsigned short > m_WeaponFactoryDatabase;
#endif

void RegisterScriptedWeapon(const char *className)
{
#ifdef CLIENT_DLL
	//if (GetClassMap().FindFactory(className))
	//{
	return;
	//}
	GetClassMap().Add(className, "CWeaponCustom", sizeof(CWeaponCustom),
		&CCHL2MPScriptedWeaponFactory);
	//GetClassMap().Add( className, "CWeaponCustom", sizeof( C_HLSelectFireMachineGun));
#else
	if (EntityFactoryDictionary()->FindFactory(className))
	{
		return;
	}

	unsigned short lookup = m_WeaponFactoryDatabase.Find(className);
	if (lookup != m_WeaponFactoryDatabase.InvalidIndex())
	{
		return;
	}

	// Andrew; This fixes months worth of pain and anguish.
	CEntityFactory<CWeaponCustom> *pFactory = new CEntityFactory<CWeaponCustom>(className);

	lookup = m_WeaponFactoryDatabase.Insert(className, pFactory);
	Assert(lookup != m_WeaponFactoryDatabase.InvalidIndex());
#endif
	// BUGBUG: When attempting to precache weapons registered during runtime,
	// they don't appear as valid registered entities.
	// static CPrecacheRegister precache_weapon_(&CPrecacheRegister::PrecacheFn_Other, className);
}
void InitCustomWeapon()
{
	FileFindHandle_t findHandle; // note: FileFINDHandle

	const char *pFilename = filesystem->FindFirstEx("scripts/weapon_custom/*.txt", "MOD", &findHandle);
	while (pFilename)
	{
		Msg("%s added to custom weapons!\n", pFilename);

#if !defined(CLIENT_DLL)
		//	CEntityFactory<CWeaponCustom> weapon_custom( pFilename );
		//	UTIL_PrecacheOther(pFilename);
#endif
		char fileBase[512] = "";
		Q_FileBase(pFilename, fileBase, sizeof(fileBase));
		RegisterScriptedWeapon(fileBase);
		//CEntityFactory<CWeaponCustom>(CEntityFactory<CWeaponCustom> &);
		//LINK_ENTITY_TO_CLASS2(pFilename,CWeaponCustom);

		WEAPON_FILE_INFO_HANDLE tmp;
#ifdef CLIENT_DLL
		if (ReadWeaponDataFromFileForSlot(filesystem, fileBase, &tmp))
		{
			gWR.LoadWeaponSprites(tmp);
		}
#else
		ReadWeaponDataFromFileForSlot(filesystem, fileBase, &tmp);
#endif


		pFilename = filesystem->FindNext(findHandle);
	}

	filesystem->FindClose(findHandle);

}
void PrecacheFileWeaponInfoDatabase(IFileSystem *filesystem, const unsigned char *pICEKey)
{

	if (m_WeaponInfoDatabase.Count())
		return;
	//InitCustomWeapon();
	KeyValues *manifest = new KeyValues("weaponscripts");
	if (manifest->LoadFromFile(filesystem, "scripts/weapon_manifest.txt", "GAME"))
	{
		for (KeyValues *sub = manifest->GetFirstSubKey(); sub != NULL; sub = sub->GetNextKey())
		{
			if (!Q_stricmp(sub->GetName(), "file"))
			{
				char fileBase[512];
				Q_FileBase(sub->GetString(), fileBase, sizeof(fileBase));
				WEAPON_FILE_INFO_HANDLE tmp;
#ifdef CLIENT_DLL
				if (ReadWeaponDataFromFileForSlot(filesystem, fileBase, &tmp, pICEKey))
				{
					gWR.LoadWeaponSprites(tmp);
				}
#else
				ReadWeaponDataFromFileForSlot(filesystem, fileBase, &tmp, pICEKey);
#endif
			}
			else
			{
				Error("Expecting 'file', got %s\n", sub->GetName());
			}
		}
	}
	manifest->deleteThis();
}

KeyValues* ReadEncryptedKVFile(IFileSystem *filesystem, const char *szFilenameWithoutExtension, const unsigned char *pICEKey, bool bForceReadEncryptedFile /*= false*/)
{
	Assert(strchr(szFilenameWithoutExtension, '.') == NULL);
	char szFullName[512];

	const char *pSearchPath = "MOD";

	if (pICEKey == NULL)
	{
		pSearchPath = "GAME";
	}

	// Open the weapon data file, and abort if we can't
	KeyValues *pKV = new KeyValues("WeaponDatafile");

	Q_snprintf(szFullName, sizeof(szFullName), "%s.txt", szFilenameWithoutExtension);

	if (bForceReadEncryptedFile || !pKV->LoadFromFile(filesystem, szFullName, pSearchPath)) // try to load the normal .txt file first
	{
#ifndef _XBOX
		if (pICEKey)
		{
			Q_snprintf(szFullName, sizeof(szFullName), "%s.ctx", szFilenameWithoutExtension); // fall back to the .ctx file

			FileHandle_t f = filesystem->Open(szFullName, "rb", pSearchPath);

			if (!f)
			{
				pKV->deleteThis();
				return NULL;
			}
			// load file into a null-terminated buffer
			int fileSize = filesystem->Size(f);
			char *buffer = (char*)MemAllocScratch(fileSize + 1);

			Assert(buffer);

			filesystem->Read(buffer, fileSize, f); // read into local buffer
			buffer[fileSize] = 0; // null terminate file as EOF
			filesystem->Close(f);	// close file after reading

			UTIL_DecodeICE((unsigned char*)buffer, fileSize, pICEKey);

			bool retOK = pKV->LoadFromBuffer(szFullName, buffer, filesystem);

			MemFreeScratch();

			if (!retOK)
			{
				pKV->deleteThis();
				return NULL;
			}
		}
		else
		{
			pKV->deleteThis();
			return NULL;
		}
#else
		pKV->deleteThis();
		return NULL;
#endif
	}

	return pKV;
}


//-----------------------------------------------------------------------------
// Purpose: Read data on weapon from script file
// Output:  true  - if data2 successfully read
//			false - if data load fails
//-----------------------------------------------------------------------------

bool ReadWeaponDataFromFileForSlot(IFileSystem* filesystem, const char *szWeaponName, WEAPON_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey)
{
	if (!phandle)
	{
		Assert(0);
		return false;
	}

	*phandle = FindWeaponInfoSlot(szWeaponName);
	FileWeaponInfo_t *pFileInfo = GetFileWeaponInfoFromHandle(*phandle);
	Assert(pFileInfo);

	if (pFileInfo->bParsedScript)
		return true;

	char sz[128];
	Q_snprintf(sz, sizeof(sz), "scripts/%s", szWeaponName);

	KeyValues *pKV = ReadEncryptedKVFile(filesystem, sz, pICEKey,
#if defined( DOD_DLL )
		true			// Only read .ctx files!
#else
		false
#endif
	);
	if (!pKV) //If it failed even after the custom weapon check, then don't read it
		return false;

	pFileInfo->Parse(pKV, szWeaponName);

	pKV->deleteThis();

	return true;
}


//-----------------------------------------------------------------------------
// FileWeaponInfo_t implementation.
//-----------------------------------------------------------------------------

FileWeaponInfo_t::FileWeaponInfo_t()
{
	bParsedScript = false;
	bLoadedHudElements = false;
	szClassName[0] = 0;
	szPrintName[0] = 0;

	szViewModel[0] = 0;
	szWorldModel[0] = 0;
	szAnimationPrefix[0] = 0;
	iSlot = 0;
	iPosition = 0;
	iMaxClip1 = 0;
	iMaxClip2 = 0;
	iDefaultClip1 = 0;
	iDefaultClip2 = 0;
	iWeight = 0;
	iRumbleEffect = -1;
	bAutoSwitchTo = false;
	bAutoSwitchFrom = false;
	iFlags = 0;
	szAmmo1[0] = 0;
	szAmmo2[0] = 0;
	memset(aShootSounds, 0, sizeof(aShootSounds));
	iAmmoType = 0;
	iAmmo2Type = 0;
	m_bMeleeWeapon = false;
	iSpriteCount = 0;
	iconActive = 0;
	iconInactive = 0;
	iconAmmo = 0;
	iconAmmo2 = 0;
	iconCrosshair = 0;
	iconAutoaim = 0;
	iconZoomedCrosshair = 0;
	iconZoomedAutoaim = 0;
	bShowUsageHint = false;
	m_bAllowFlipping = true;
	m_bBuiltRightHanded = true;

	// GSTRINGMIGRATION
	flCameraMovementScale = 1.0f;
	*szCameraAttachmentName = 0;
	*szCameraBoneName = 0;
	angCameraMovementOrientation = vec3_angle;
	angCameraMovementOffset = vec3_angle;
	flCameraMovementReferenceCycle = 0.0f;
	*szFlashlightTexture = 0;
	// END GSTRINGMIGRATION
}

#ifdef CLIENT_DLL
extern ConVar hud_fastswitch;
#endif

void FileWeaponInfo_t::Parse(KeyValues *pKeyValuesData, const char *szWeaponName)
{
	// Okay, we tried at least once to look this up...
	bParsedScript = true;

	// Classname
	Q_strncpy(szClassName, szWeaponName, MAX_WEAPON_STRING);
	// Printable name
	Q_strncpy(szPrintName, pKeyValuesData->GetString("printname", WEAPON_PRINTNAME_MISSING), MAX_WEAPON_STRING);
	// View model & world model
	Q_strncpy(szViewModel, pKeyValuesData->GetString("viewmodel"), MAX_WEAPON_STRING);
	Q_strncpy(szWorldModel, pKeyValuesData->GetString("playermodel"), MAX_WEAPON_STRING);
	Q_strncpy(szAnimationPrefix, pKeyValuesData->GetString("anim_prefix"), MAX_WEAPON_PREFIX);
	iPosition = pKeyValuesData->GetInt("bucket_position", 0);

	const char *pBucket = pKeyValuesData->GetString("bucket", NULL);
	if (pBucket)
	{
		if (Q_stricmp(pBucket, "melee") == 0)
		{
			iSlot = 0;
		}
		else if (Q_stricmp(pBucket, "pistol") == 0)
		{
			iSlot = 1;
		}
		else if (Q_stricmp(pBucket, "light") == 0)
		{
			iSlot = 2;
		}
		else if (Q_stricmp(pBucket, "heavy") == 0)
		{
			iSlot = 3;
		}
		else if (Q_stricmp(pBucket, "grenades") == 0)
		{
			iSlot = 4;
		}
		else if (Q_stricmp(pBucket, "special") == 0)
		{
			iSlot = 5;
		}
		else if (Q_stricmp(pBucket, "tool") == 0)
		{
			iSlot = 6;
		}
		else if (Q_stricmp(pBucket, "disabled") == 0)
		{
			iSlot = 25;
		}
		else
		{
			iSlot = pKeyValuesData->GetInt("bucket", 0);
			Msg("Weapon %s (%s) is using old-style buckets, should really be using new ones.\n", szPrintName, szClassName);
		}
	}
	else
	{
		iSlot = pKeyValuesData->GetInt("bucket", 17);  //Default this to out-of-bounds so they don't cover up the crowbar...
		Warning("WARNING: Weapon %s (%s) is missing bucket info!!\n", szPrintName, szClassName);
	}

	// Use the console (X360) buckets if hud_fastswitch is set to 2.
	//SMOD: Our new hud does not support X360-style hudbuckets
	//#ifdef CLIENT_DLL
	//	if ( hud_fastswitch.GetInt() == 2 )
	//#else
	//	if ( IsX360() )
	//#endif
	//	{
	//		iSlot = pKeyValuesData->GetInt( "bucket_360", iSlot );
	//		iPosition = pKeyValuesData->GetInt( "bucket_position_360", iPosition );
	//	}

	// Use the console (X360) buckets if hud_fastswitch is set to 2.
#ifdef CLIENT_DLL
	if (hud_fastswitch.GetInt() == 2)
#else
	if (IsX360())
#endif
	{
		iSlot = pKeyValuesData->GetInt("bucket_360", iSlot);
		iPosition = pKeyValuesData->GetInt("bucket_position_360", iPosition);
	}
	iMaxClip1 = pKeyValuesData->GetInt("clip_size", WEAPON_NOCLIP);					// Max primary clips gun can hold (assume they don't use clips by default)
	iMaxClip2 = pKeyValuesData->GetInt("clip2_size", WEAPON_NOCLIP);					// Max secondary clips gun can hold (assume they don't use clips by default)
	iDefaultClip1 = pKeyValuesData->GetInt("default_clip", iMaxClip1);		// amount of primary ammo placed in the primary clip when it's picked up
	iDefaultClip2 = pKeyValuesData->GetInt("default_clip2", iMaxClip2);		// amount of secondary ammo placed in the secondary clip when it's picked up
	iWeight = pKeyValuesData->GetInt("weight", 0);

	iRumbleEffect = pKeyValuesData->GetInt("rumble", -1);

	// LAME old way to specify item flags.
	// Weapon scripts should use the flag names.
	iFlags = pKeyValuesData->GetInt("item_flags", ITEM_FLAG_LIMITINWORLD);

	for (int i = 0; i < ARRAYSIZE(g_ItemFlags); i++)
	{
		int iVal = pKeyValuesData->GetInt(g_ItemFlags[i].m_pFlagName, -1);
		if (iVal == 0)
		{
			iFlags &= ~g_ItemFlags[i].m_iFlagValue;
		}
		else if (iVal == 1)
		{
			iFlags |= g_ItemFlags[i].m_iFlagValue;
		}
	}


	bShowUsageHint = (pKeyValuesData->GetInt("showusagehint", 0) != 0) ? true : false;
	bAutoSwitchTo = (pKeyValuesData->GetInt("autoswitchto", 1) != 0) ? true : false;
	bAutoSwitchFrom = (pKeyValuesData->GetInt("autoswitchfrom", 1) != 0) ? true : false;
	m_bBuiltRightHanded = (pKeyValuesData->GetInt("BuiltRightHanded", 1) != 0) ? true : false;
	m_bAllowFlipping = (pKeyValuesData->GetInt("AllowFlipping", 1) != 0) ? true : false;
	m_bMeleeWeapon = (pKeyValuesData->GetInt("MeleeWeapon", 0) != 0) ? true : false;

#if defined(_DEBUG) && defined(HL2_CLIENT_DLL)
	// make sure two weapons aren't in the same slot & position
	if (iSlot >= MAX_WEAPON_SLOTS ||
		iPosition >= MAX_WEAPON_POSITIONS)
	{
		Warning("Invalid weapon slot or position [slot %d/%d max], pos[%d/%d max]\n",
			iSlot, MAX_WEAPON_SLOTS - 1, iPosition, MAX_WEAPON_POSITIONS - 1);
	}
	else
	{
		if (g_bUsedWeaponSlots[iSlot][iPosition])
		{
			Warning("Duplicately assigned weapon slots in selection hud:  %s (%d, %d)\n", szPrintName, iSlot, iPosition);
		}
		g_bUsedWeaponSlots[iSlot][iPosition] = true;
	}
#endif

	// Primary ammo used
	const char *pAmmo = pKeyValuesData->GetString("primary_ammo", "None");
	if (strcmp("None", pAmmo) == 0)
		Q_strncpy(szAmmo1, "", sizeof(szAmmo1));
	else
		Q_strncpy(szAmmo1, pAmmo, sizeof(szAmmo1));
	iAmmoType = GetAmmoDef()->Index(szAmmo1);

	// Secondary ammo used
	pAmmo = pKeyValuesData->GetString("secondary_ammo", "None");
	if (strcmp("None", pAmmo) == 0)
		Q_strncpy(szAmmo2, "", sizeof(szAmmo2));
	else
		Q_strncpy(szAmmo2, pAmmo, sizeof(szAmmo2));
	iAmmo2Type = GetAmmoDef()->Index(szAmmo2);

	// Now read the weapon sounds
	memset(aShootSounds, 0, sizeof(aShootSounds));
	KeyValues *pSoundData = pKeyValuesData->FindKey("SoundData");
	if (pSoundData)
	{
		for (int i = EMPTY; i < NUM_SHOOT_SOUND_TYPES; i++)
		{
			const char *soundname = pSoundData->GetString(pWeaponSoundCategories[i]);
			if (soundname && soundname[0])
			{
				Q_strncpy(aShootSounds[i], soundname, MAX_WEAPON_STRING);
			}
		}
	}

	m_flAddFOV = pKeyValuesData->GetFloat("addfov", 0.0f);
	m_flLagScale = pKeyValuesData->GetFloat("LagScale", 1.0f);

	KeyValues *pSights = pKeyValuesData->FindKey("IronSight");
	if (pSights)
	{
		bHasIronsight = true;
		vecIronsightPosOffset.x = pSights->GetFloat("forward", 0.0f);
		vecIronsightPosOffset.y = pSights->GetFloat("right", 0.0f);
		vecIronsightPosOffset.z = pSights->GetFloat("up", 0.0f);

		angIronsightAngOffset[PITCH] = pSights->GetFloat("pitch", 0.0f);
		angIronsightAngOffset[YAW] = pSights->GetFloat("yaw", 0.0f);
		angIronsightAngOffset[ROLL] = pSights->GetFloat("roll", 0.0f);

		flIronsightFOVOffset = pSights->GetFloat("fov", 0.0f);
	}
	else
	{
		bHasIronsight = false;
		//note: you can set a bool here if you'd like to disable ironsights for weapons with no IronSight-key
		vecIronsightPosOffset = vec3_origin;
		angIronsightAngOffset.Init();
		flIronsightFOVOffset = 0.0f;
	}

	KeyValues *pAdjust = pKeyValuesData->FindKey("Adjust");
	if (pAdjust)
	{
		vecAdjustPosOffset.x = -pAdjust->GetFloat("forward", 0.0f);
		vecAdjustPosOffset.y = -pAdjust->GetFloat("right", 0.0f);
		vecAdjustPosOffset.z = -pAdjust->GetFloat("up", 0.0f);

		angAdjustAngOffset[PITCH] = pAdjust->GetFloat("pitch", 0.0f);
		angAdjustAngOffset[YAW] = pAdjust->GetFloat("yaw", 0.0f);
		angAdjustAngOffset[ROLL] = pAdjust->GetFloat("roll", 0.0f);
	}
	else
	{
		//note: you can set a bool here if you'd like to disable ironsights for weapons with no IronSight-key
		vecAdjustPosOffset = vec3_origin;
		angAdjustAngOffset.Init();
	}

	KeyValues *pLower = pKeyValuesData->FindKey("WeaponLower");
	if (pLower)
	{
		vecLowerPosOffset.x = pLower->GetFloat("forward", 0.0f);
		vecLowerPosOffset.y = pLower->GetFloat("right", 0.0f);
		vecLowerPosOffset.z = pLower->GetFloat("up", 0.0f);

		angLowerAngOffset[PITCH] = pLower->GetFloat("pitch", 0.0f);
		angLowerAngOffset[YAW] = pLower->GetFloat("yaw", 0.0f);
		angLowerAngOffset[ROLL] = pLower->GetFloat("roll", 0.0f);
	}
	else
	{
		//note: you can set a bool here if you'd like to disable ironsights for weapons with no IronSight-key
		vecLowerPosOffset = vec3_origin;
		angLowerAngOffset.Init();
	}

	// GSTRINGMIGRATION
	const char *pszCameraAttachment = pKeyValuesData->GetString("camera_attachment", "muzzle");
	Q_strncpy(szCameraAttachmentName, pszCameraAttachment, sizeof(szCameraAttachmentName));

	const char *pszCameraBone = pKeyValuesData->GetString("camera_bone");
	Q_strncpy(szCameraBoneName, pszCameraBone, sizeof(szCameraBoneName));

	flCameraMovementScale = pKeyValuesData->GetFloat("camera_movement_scale", 1.0f);
	flCameraMovementReferenceCycle = pKeyValuesData->GetFloat("camera_reference_cycle", 0.0f);

	UTIL_StringToVector(angCameraMovementOrientation.Base(), pKeyValuesData->GetString("camera_orientation", "0 0 0"));
	UTIL_StringToVector(angCameraMovementOffset.Base(), pKeyValuesData->GetString("camera_offset_angle", "0 0 0"));

	Q_strncpy(szFlashlightTexture, pKeyValuesData->GetString("flashlight_texture"), sizeof(szFlashlightTexture));
	// END GSTRINGMIGRATION

	m_flDefaultSpread = 2;
	m_flMaxSpread = 2;
	m_flRunSpread = 5;
	m_flRunSpeedSpread = 10;
	m_flCrouchSpread = 0.5f;
	m_flFireSpread = 2;

	KeyValues *pWeaponSpec = pKeyValuesData->FindKey("WeaponSpec");
	if (pWeaponSpec)
	{
		m_iWeaponType = pWeaponSpec->GetInt("WeaponType", 0);
		m_sPrimaryFireRate = pWeaponSpec->GetFloat("FireRate", 1.0f);
		bIsAkimbo = (pWeaponSpec->GetInt("Akimbo", 0) != 0) ? true : false;

		KeyValues *pOptions = pWeaponSpec->FindKey("Options");
		if (pOptions)
		{
			m_iCrosshairMinDistance = pOptions->GetFloat("CrossHairMinSize", 1.0f);
			m_iCrosshairDeltaDistance = pOptions->GetFloat("CrossHairMove", 0.1f);
		}
		else
		{
			m_iCrosshairMinDistance = 1.0f;
			m_iCrosshairDeltaDistance = 0.1f;
		}

		KeyValues *pBullet1 = pWeaponSpec->FindKey("Bullet");
		if (pBullet1)
		{
			m_sPrimaryBulletEnabled = true;
			m_sPrimaryDamage = pBullet1->GetFloat("Damage", 0);
			m_sPrimaryShotCount = pBullet1->GetInt("ShotCount", 0);
			m_iPrimaryPenetrateCount = pBullet1->GetInt("PenetrationMax", 0);
			m_flPrimaryPenetrateDepth = pBullet1->GetFloat("PenetrationDepth", 0);
		}

		KeyValues *pActs = pWeaponSpec->FindKey("Activities");
		//KeyValues *pActsw = pWeaponSpec->FindKey("Activities!");
		if (pActs/* || pActsw*/)
		{
			m_actPrimary = StringToVMActivity(pActs->GetString("PrimaryFire", "ACT_VM_PRIMARYATTACK"));
			m_actSecondary = StringToVMActivity(pActs->GetString("SecondaryFire", "ACT_VM_SECONDARYATTACK"));
			m_actSpecial = StringToVMActivity(pActs->GetString("Special", "ACT_VM_DRYFIRE"));
		}
		else
		{
			m_actPrimary = StringToVMActivity("ACT_VM_PRIMARYATTACK");
			m_actSecondary = StringToVMActivity("ACT_VM_SECONDARYATTACK");
			m_actSpecial = StringToVMActivity("ACT_VM_DRYFIRE");
		}

		KeyValues *pMelee = pWeaponSpec->FindKey("Melee");
		if (pMelee)
		{
			m_sPrimaryMeleeEnabled = false;
			m_sSecondaryMeleeEnabled = false;
			m_bMeleeWeapon = true;

			KeyValues *pPrimaryMelee = pMelee->FindKey("Primary");
			if (pPrimaryMelee)
			{
				m_sPrimaryMeleeEnabled = true;
				//Q_strncpy(m_sPrimaryMeleeDamageType, pPrimaryMelee->GetString("DamageType", "DMG_CLUB"), MAX_WEAPON_STRING);
				m_sPrimaryMeleeDamage = pPrimaryMelee->GetFloat("Damage", 30.0f);
				m_sPrimaryMeleeRange = pPrimaryMelee->GetFloat("Range", 64.0f);
				m_sNextPrimaryMeleeAttack = pPrimaryMelee->GetFloat("NextAttack", 10.0f) / 10;

			}
			KeyValues *pSecondaryMelee = pMelee->FindKey("Secondary");
			if (pSecondaryMelee)
			{
				m_sSecondaryMeleeEnabled = true;
				//m_sSecondaryMeleeDamageType = pSecondaryMelee->GetString("DamageType", "DMG_CLUB");
				m_sSecondaryMeleeDamage = pSecondaryMelee->GetFloat("Damage", 30.0f);
				m_sSecondaryMeleeRange = pSecondaryMelee->GetFloat("Range", 64.0f);
				m_sNextSecondaryMeleeAttack = pSecondaryMelee->GetFloat("NextAttack", 10.0f) / 10;
			}
		}
		else
		{
			m_bMeleeWeapon = false;
			m_sPrimaryMeleeEnabled = false;
			m_sSecondaryMeleeEnabled = false;
		}

		KeyValues *pRecoil = pWeaponSpec->FindKey("Recoil");
		if (pRecoil)
		{
			UTIL_StringToVector(m_vRecoilPunchPitch.Base(), pRecoil->GetString("PunchPitch", "0 0"));
			UTIL_StringToVector(m_vRecoilPunchYaw.Base(), pRecoil->GetString("PunchYaw", "0 0"));
			m_flRecoilCrouch = pRecoil->GetFloat("Crouch", 0.5f);
			m_flRecoilAmp = pRecoil->GetFloat("Amp", 1);
			m_flPunchLimit = pRecoil->GetFloat("PunchLimit", 2);
		}

		KeyValues *pLaser = pWeaponSpec->FindKey("LaserPointer");
		if (pLaser)
		{
			bLaser = true;
			bLaserUseButton = (pLaser->GetInt("UseButton", 0) != 0) ? true : false;
			bLaserUseIronsight = (pLaser->GetInt("UseIronsight", 0) != 0) ? true : false;
			bLaserDeactivateinReload = (pLaser->GetInt("DeactivateinReload", 1) != 0) ? true : false;
			Q_strncpy(sLaserMaterial, pLaser->GetString("Material"), sizeof(sLaserMaterial));
			flLaserDrawRange = pLaser->GetFloat("DrawRange", 512.0f);
			flLaserTraceLength = pLaser->GetFloat("TraceLength", 64.0f);
			bLaserFixSize = (pLaser->GetInt("FixSize", 1) != 0) ? true : false;
			flLaserPointerSize = pLaser->GetFloat("TraceLength", 0.5f);
		}
		else
		{
			bLaser = false;
		}

		KeyValues *pSpread1 = pWeaponSpec->FindKey("Spread");
		if (pSpread1)
		{
			m_flDefaultSpread = pSpread1->GetFloat("Default", 1.0f);
			m_flMaxSpread = pSpread1->GetFloat("MaxSpread", 50);
			m_flRunSpread = pSpread1->GetFloat("Run", 5);
			m_flRunSpeedSpread = pSpread1->GetFloat("RunSpeed", 10);
			m_flCrouchSpread = pSpread1->GetFloat("Crouch", 0.5f);
			m_flFireSpread = pSpread1->GetFloat("Fire", 2);
			/*m_vPrimarySpread.x = RandomFloat(-sin(m_flDefaultSpread / 2), sin(m_flMaxSpread / 2));
			m_vPrimarySpread.y = RandomFloat(-sin(m_flDefaultSpread / 2), sin(m_flMaxSpread / 2));
			m_vPrimarySpread.z = RandomFloat(-sin(m_flDefaultSpread / 2), sin(m_flMaxSpread / 2));*/
		}
		/*
		else
		{
		m_sPrimaryDamage = 0.0f;
		m_sSecondaryShotCount = 0;
		m_sPrimaryBulletEnabled = false;
		}*/

		KeyValues *pMissle1 = pWeaponSpec->FindKey("Missle");
		if (pMissle1) //No params yet, but setting this will enable missles
		{
			m_sPrimaryMissleEnabled = true;
		}
		else
		{
			m_sPrimaryMissleEnabled = false;
		}

		KeyValues *pSecondaryFire = pWeaponSpec->FindKey("SecondaryFire");
		if (pSecondaryFire)
		{
			m_sSecondaryFireRate = pSecondaryFire->GetFloat("FireRate", 1.0f);
			m_sUsePrimaryAmmo = (pSecondaryFire->GetInt("UsePrimaryAmmo", 0) != 0) ? true : false;
			KeyValues *pBullet2 = pSecondaryFire->FindKey("Bullet");
			if (pBullet2)
			{
				m_sSecondaryBulletEnabled = true;
				m_sSecondaryDamage = pBullet2->GetFloat("Damage", 0);
				m_sSecondaryShotCount = pBullet2->GetInt("ShotCount", 0);
				m_iSecondaryPenetrateCount = pBullet2->GetInt("PenetrationMax", 0);
				m_flSecondaryPenetrateDepth = pBullet2->GetFloat("PenetrationDepth", 0);


				KeyValues *pSpread2 = pBullet2->FindKey("Spread");
				if (pSpread2)
				{
					m_vSecondarySpread.x = sin(pSpread2->GetFloat("x", 0.0f) / 2.0f);
					m_vSecondarySpread.y = sin(pSpread2->GetFloat("y", 0.0f) / 2.0f);
					m_vSecondarySpread.z = sin(pSpread2->GetFloat("z", 0.0f) / 2.0f);
				}
				else
				{
					m_vSecondarySpread.x = 0.0f;
					m_vSecondarySpread.y = 0.0f;
					m_vSecondarySpread.z = 0.0f;
				}
			}
			else
			{
				m_sSecondaryDamage = 0.0f;
				m_sSecondaryShotCount = 0;
				m_sSecondaryBulletEnabled = false;
			}
			KeyValues *pMissle2 = pSecondaryFire->FindKey("Missle");
			if (pMissle2) //No params yet, but setting this will enable missles
			{
				m_sSecondaryMissleEnabled = true;
			}
			else
			{
				m_sSecondaryMissleEnabled = false;
			}
		}

		KeyValues *pNpc = pWeaponSpec->FindKey("Npc");
		if (pNpc)
		{
			char tmpstr[16];
			Q_strcpy(tmpstr, pNpc->GetString("AnimType", "smg1"));
			Q_strlower(tmpstr);
			Q_strcpy(sNpcAnimType, tmpstr);
			flNpcFireRate = pNpc->GetFloat("Firerate", 1.0f);
			iNpcBurstMax = pNpc->GetInt("BurstMax", 2);
			iNpcBurstMin = pNpc->GetInt("BurstMin", 2);
		}
	}
}
