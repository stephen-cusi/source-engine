//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "grenade_ar2.h"
#include "weapon_rpg.h"
#include "triggers.h"
#include "ai_memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "filesystem.h"
#include "weapon_custom.h"
#include "hl2_player.h"
#include "ammodef.h"
#include "bullet_9mm.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_auto_reload_time;

extern ConVar bullettimesim;

extern ConVar disable_bullettime;

IMPLEMENT_SERVERCLASS_ST(CWeaponCustom, DT_WeaponCustom)
END_SEND_TABLE()

BEGIN_DATADESC(CWeaponCustom)
DEFINE_FIELD(m_hMissile, FIELD_EHANDLE),
END_DATADESC()

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

//#define	PISTOL_FASTEST_REFIRE_TIME		0.1f
//#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

//=========================================================
CWeaponCustom::CWeaponCustom()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 1400;

	m_flNextMeleeAttack = 0;

	m_bAltFiresUnderwater = false;
	m_bSighting = false;
	m_bIsLeftShoot = false;
}

acttable_t	CWeaponCustom::m_acttableSMG[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SMG1,			true },
{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

{ ACT_WALK,						ACT_WALK_RIFLE,					true },
{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },

// Readiness activities (not aiming)
{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

																		// Readiness activities (aiming)
{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
																		//End readiness activities

{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
{ ACT_RUN,						ACT_RUN_RIFLE,					true },
{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

acttable_t	CWeaponCustom::m_acttablePISTOL[] =
{
	{ ACT_IDLE,						ACT_IDLE_PISTOL,				true },
{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_PISTOL,			true },
{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_PISTOL,		true },
{ ACT_RELOAD,					ACT_RELOAD_PISTOL,				true },
{ ACT_WALK_AIM,					ACT_WALK_AIM_PISTOL,			true },
{ ACT_RUN_AIM,					ACT_RUN_AIM_PISTOL,				true },
{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_PISTOL,true },
{ ACT_RELOAD_LOW,				ACT_RELOAD_PISTOL_LOW,			false },
{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_PISTOL_LOW,	false },
{ ACT_COVER_LOW,				ACT_COVER_PISTOL_LOW,			false },
{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_PISTOL_LOW,		false },
{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_PISTOL,		false },
{ ACT_WALK,						ACT_WALK_PISTOL,				false },
{ ACT_RUN,						ACT_RUN_PISTOL,					false },
};

acttable_t	CWeaponCustom::m_acttableRIFLE[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
{ ACT_IDLE,						ACT_IDLE_SMG1,					true },		// FIXME: hook to AR2 unique
{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },		// FIXME: hook to AR2 unique

{ ACT_WALK,						ACT_WALK_RIFLE,					true },

// Readiness activities (not aiming)
{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

																		// Readiness activities (aiming)
{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
																		//End readiness activities

{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
{ ACT_RUN,						ACT_RUN_RIFLE,					true },
{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },		// FIXME: hook to AR2 unique
{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },		// FIXME: hook to AR2 unique
{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
};

acttable_t	CWeaponCustom::m_acttableSHOTGUN[] =
{
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },	// FIXME: hook to shotgun unique

{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SHOTGUN,			true },
{ ACT_RELOAD,					ACT_RELOAD_SHOTGUN,					false },
{ ACT_WALK,						ACT_WALK_RIFLE,						true },
{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SHOTGUN,				true },

// Readiness activities (not aiming)
{ ACT_IDLE_RELAXED,				ACT_IDLE_SHOTGUN_RELAXED,		false },//never aims
{ ACT_IDLE_STIMULATED,			ACT_IDLE_SHOTGUN_STIMULATED,	false },
{ ACT_IDLE_AGITATED,			ACT_IDLE_SHOTGUN_AGITATED,		false },//always aims

{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

																		// Readiness activities (aiming)
{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
																		//End readiness activities

{ ACT_WALK_AIM,					ACT_WALK_AIM_SHOTGUN,				true },
{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,				true },
{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,			true },
{ ACT_RUN,						ACT_RUN_RIFLE,						true },
{ ACT_RUN_AIM,					ACT_RUN_AIM_SHOTGUN,				true },
{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,				true },
{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,			true },
{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SHOTGUN,	true },
{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SHOTGUN_LOW,		true },
{ ACT_RELOAD_LOW,				ACT_RELOAD_SHOTGUN_LOW,				false },
{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SHOTGUN,			false },
};

acttable_t	CWeaponCustom::m_acttableCROWBAR[] =
{
{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
};

acttable_t *CWeaponCustom::ActivityList(void) {
	if (!Q_strcmp(GetWpnData().sNpcAnimType, "smg1"))
		return m_acttableSMG;
	else if (!Q_strcmp(GetWpnData().sNpcAnimType, "pistol"))
		return m_acttablePISTOL;
	else if (!Q_strcmp(GetWpnData().sNpcAnimType, "ar2"))
		return m_acttableRIFLE;
	else if (!Q_strcmp(GetWpnData().sNpcAnimType, "shotgun"))
		return m_acttableSHOTGUN;
	else if (!Q_strcmp(GetWpnData().sNpcAnimType, "crowbar"))
		return m_acttableCROWBAR;
	else
		return m_acttableSMG;
}

int CWeaponCustom::ActivityListCount(void) { 
	if (!Q_strcmp(GetWpnData().sNpcAnimType, "smg1"))
		return ARRAYSIZE(m_acttableSMG);
	else if (!Q_strcmp(GetWpnData().sNpcAnimType, "pistol"))
		return ARRAYSIZE(m_acttablePISTOL);
	else if (!Q_strcmp(GetWpnData().sNpcAnimType, "ar2"))
		return ARRAYSIZE(m_acttableRIFLE);
	else if (!Q_strcmp(GetWpnData().sNpcAnimType, "shotgun"))
		return ARRAYSIZE(m_acttableSHOTGUN);
	else if (!Q_strcmp(GetWpnData().sNpcAnimType, "crowbar"))
		return ARRAYSIZE(m_acttableCROWBAR);
	else
		return ARRAYSIZE(m_acttableSMG);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::Precache(void)
{
	m_iPrimaryAmmoType = m_iSecondaryAmmoType = -1;

	// Add this weapon to the weapon registry, and get our index into it
	// Get weapon data from script file
	if (ReadWeaponDataFromFileForSlot(filesystem, GetClassname(), &m_hWeaponFileInfo, GetEncryptionKey()))
	{
		// Get the ammo indexes for the ammo's specified in the data file
		if (GetWpnData().szAmmo1[0])
		{
			m_iPrimaryAmmoType = GetAmmoDef()->Index(GetWpnData().szAmmo1);
			if (m_iPrimaryAmmoType == -1)
			{
				Msg("ERROR: Weapon (%s) using undefined primary ammo type (%s)\n", GetClassname(), GetWpnData().szAmmo1);
			}
		}
		if (GetWpnData().szAmmo2[0])
		{
			m_iSecondaryAmmoType = GetAmmoDef()->Index(GetWpnData().szAmmo2);
			if (m_iSecondaryAmmoType == -1)
			{
				Msg("ERROR: Weapon (%s) using undefined secondary ammo type (%s)\n", GetClassname(), GetWpnData().szAmmo2);
			}

		}
		// Precache models (preload to avoid hitch)
		m_iViewModelIndex = 0;
		m_iWorldModelIndex = 0;
		if (GetViewModel() && GetViewModel()[0])
		{
			m_iViewModelIndex = CBaseEntity::PrecacheModel(GetViewModel());
		}
		if (GetWorldModel() && GetWorldModel()[0])
		{
			m_iWorldModelIndex = CBaseEntity::PrecacheModel(GetWorldModel());
		}

		// Precache sounds, too
		for (int i = 0; i < NUM_SHOOT_SOUND_TYPES; ++i)
		{
			const char *shootsound = GetShootSound(i);
			if (shootsound && shootsound[0])
			{
				CBaseEntity::PrecacheScriptSound(shootsound);
			}
		}
	}
	else
	{
		// Couldn't read data file, remove myself
		Warning("Error reading weapon data file for: %s\n", GetClassname());
		//	Remove( );	//don't remove, this gets released soon!
	}

	UTIL_PrecacheOther( "bullet_9mm" );
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponCustom::Equip(CBaseCombatCharacter *pOwner)
{
	if (pOwner->Classify() == CLASS_PLAYER_ALLY)
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}

	BaseClass::Equip(pOwner);
}
#if 1
void CWeaponCustom::ItemPostFrame(void)
{
	CHL2_Player *pOwner = (CHL2_Player *)ToBasePlayer(GetOwner());
	if (!pOwner)
		return;
	if (GetWpnData().m_iWeaponType == 2 || GetWpnData().m_iWeaponType == 3)
	{
		ShotgunPostFrame();
		return;
	}
	if (GetWpnData().m_iWeaponType != 0)
	{

		//Track the duration of the fire
		//FIXME: Check for IN_ATTACK2 as well?
		//FIXME: What if we're calling ItemBusyFrame?
		m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

		if (UsesClipsForAmmo1())
		{
			CheckReload();
		}
		if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			// Clip empty? Or out of ammo on a no-clip weapon?
			if (!IsMeleeWeapon() &&
				((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
			{
				HandleFireOnEmpty();
			}
			else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
			{
				// This weapon doesn't fire underwater
				WeaponSound(EMPTY);
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
				return;
			}
			else
			{
				//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
				//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
				//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
				//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
				//			first shot.  Right now that's too much of an architecture change -- jdw

				// If the firing button was just pressed, or the alt-fire just released, reset the firing time
				if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
				{
					m_flNextPrimaryAttack = gpGlobals->curtime;
				}

				SecondaryAttack();
			}
		}
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
		{
			// Clip empty? Or out of ammo on a no-clip weapon?
			if (!IsMeleeWeapon() &&
				((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
			{
				HandleFireOnEmpty();
			}
			else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
			{
				// This weapon doesn't fire underwater
				WeaponSound(EMPTY);
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
				return;
			}
			else
			{
				//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
				//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
				//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
				//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
				//			first shot.  Right now that's too much of an architecture change -- jdw

				// If the firing button was just pressed, or the alt-fire just released, reset the firing time
				if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
				{
					m_flNextPrimaryAttack = gpGlobals->curtime;
				}
				if (GetWpnData().m_iWeaponType == 0)
					SetWeaponIdleTime(gpGlobals->curtime + 3.0f);
				else
					SetWeaponIdleTime(gpGlobals->curtime + GetFireRate());
				PrimaryAttack();
			}
		}

		// -----------------------
		//  Reload pressed / Clip Empty
		// -----------------------
		if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
		{
			// reload when reload is pressed, or if no buttons are down and weapon is empty.
			Reload();
			m_fFireDuration = 0.0f;
		}

		// -----------------------
		//  No buttons down
		// -----------------------
		if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (pOwner->m_nButtons & IN_RELOAD)))
		{
			// no fire buttons down or reloading
			if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
			{
				WeaponIdle();
			}
		}
	}
	else
	{
		BaseClass::ItemPostFrame();

		if (m_bInReload)
			return;

		//Allow a refire as fast as the player can click
		if (((pOwner->m_nButtons & IN_ATTACK) == false) && (m_flSoonestPrimaryAttack < gpGlobals->curtime))
		{
			m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
		}
		else if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack < gpGlobals->curtime) && (m_iClip1 <= 0))
		{
			//	DryFire();
		}

		UpdatePenaltyTime();
	}

	//Move the laser
	if (GetWpnData().bLaser)
		UpdateLaserPosition();
}
#endif

void CWeaponCustom::ItemHolsterFrame()
{
	if (GetWpnData().m_iWeaponType == 2 || GetWpnData().m_iWeaponType == 3)
	{
		ShotgunHolsterFrame();
	}
}

void CWeaponCustom::ShotgunHolsterFrame(void)
{
	// Must be player held
	if (GetOwner() && GetOwner()->IsPlayer() == false)
		return;

	// We can't be active
	if (GetOwner()->GetActiveWeapon() == this)
		return;

	// If it's been longer than three seconds, reload
	if ((gpGlobals->curtime - m_flHolsterTime) > sk_auto_reload_time.GetFloat())
	{
		// Reset the timer
		m_flHolsterTime = gpGlobals->curtime;

		if (GetOwner() == NULL)
			return;

		if (m_iClip1 == GetMaxClip1())
			return;

		// Just load the clip with no animations
		int ammoFill = MIN((GetMaxClip1() - m_iClip1), GetOwner()->GetAmmoCount(GetPrimaryAmmoType()));

		GetOwner()->RemoveAmmo(ammoFill, GetPrimaryAmmoType());
		m_iClip1 += ammoFill;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponCustom::ShotgunStartReload(void)
{
	DisableIronsights();

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	// If shotgun totally emptied then a pump animation is needed

	//NOTENOTE: This is kinda lame because the player doesn't get strong feedback on when the reload has finished,
	//			without the pump.  Technically, it's incorrect, but it's good for feedback...

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

	// Make shotgun shell visible
	SetBodygroup(1, 0);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponCustom::ShotgunReload(void)
{
	// Check that StartReload was called first
	if (!m_bInReload)
	{
		Warning("ERROR: Shotgun Reload called incorrectly!\n");
	}

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	ShotgunFillClip();
	// Play reload on different channel as otherwise steals channel away from fire sound
	WeaponSound(RELOAD);
	SendWeaponAnim(ACT_VM_RELOAD);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponCustom::ShotgunFinishReload(void)
{
	// Make shotgun shell invisible
	//SetBodygroup(1, 1);

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bInReload = false;

	// Finish reload animation
	SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponCustom::ShotgunFillClip(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	// Add them to the clip
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
	{
		if (Clip1() < GetMaxClip1())
		{
			m_iClip1++;
			pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do mulitple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponCustom::ShotgunPostFrame(void)
{
	CHL2_Player *pOwner = (CHL2_Player*)ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	if (m_bInReload)
	{
		// If I'm primary firing and have one round stop reloading and fire
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_iClip1 >= 1))
		{
			m_bInReload = false;
		}
		// If I'm secondary firing and have one round stop reloading and fire
		else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_iClip1 >= 2))
		{
			m_bInReload = false;
		}
		else if (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				ShotgunFinishReload();
				return;
			}
			// If clip not full reload again
			if (m_iClip1 < GetMaxClip1())
			{
				ShotgunReload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				ShotgunFinishReload();
				return;
			}
		}
	}
	else
	{
		// Make shotgun shell invisible
		SetBodygroup(1, 1);
	}

	if ((pOwner->m_nButtons & IN_ATTACK) && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		if ((m_iClip1 <= 0 && UsesClipsForAmmo1()) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		{
			if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
			{
				// why i dont have this?
				//DryFire();
				return;
			}
			else
			{
				ShotgunStartReload();
			}
		}
		// Fire underwater?
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			// If the firing button was just pressed, reset the firing time
			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
			if (pPlayer && pPlayer->m_afButtonPressed & IN_ATTACK)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}
			PrimaryAttack();
		}
	}

	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		ShotgunStartReload();
	}
	else
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if (!HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			// weapon isn't useable, switch.
			if (!(GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && pOwner->SwitchToNextBestWeapon(this))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if (m_iClip1 <= 0 && !(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->curtime)
			{
				if (ShotgunStartReload())
				{
					// if we've successfully started to reload, we're done
					return;
				}
			}
		}

		WeaponIdle();
		return;
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::UpdatePenaltyTime(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Check our penalty time decay
	if (((pOwner->m_nButtons & IN_ATTACK) == false) && (m_flSoonestPrimaryAttack < gpGlobals->curtime))
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp(m_flAccuracyPenalty, 0.0f, PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME);
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	CHL2_Player *pPlayer = dynamic_cast < CHL2_Player* >(UTIL_PlayerByIndex(1));

	if (!pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0)
	{
		// FIXME: use the returned number of bullets to account for >10hz firerate
		//WeaponSoundRealtime(SINGLE_NPC);

		WeaponSound(SINGLE_NPC);

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();

		Vector vecShootOrigin, vecShootDir;
		if (bUseWeaponAngles)
		{
			QAngle	angShootDir;
			GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
			AngleVectors(angShootDir, &vecShootDir);
		}
		else
		{
			vecShootOrigin = pOperator->Weapon_ShootPosition();
			vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
		}

		CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
		if (IsPrimaryBullet() == true)
		{
			//pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
			//	MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);
			FireBulletsInfo_t info;
			info.m_iShots = this->GetWpnData().m_sPrimaryShotCount;
			info.m_flDamage = this->GetWpnData().m_sPrimaryDamage;
			info.m_iPlayerDamage = this->GetWpnData().m_sPrimaryDamage;
			info.m_vecSrc = vecShootOrigin;
			info.m_vecDirShooting = vecShootDir;
			info.m_vecSpread = GetBulletSpreadPrimary();
			info.m_flDistance = MAX_TRACE_LENGTH;
			info.m_iAmmoType = m_iPrimaryAmmoType;
			info.m_iTracerFreq = 2;
			info.m_iPenetrationCount = this->GetWpnData().m_iPrimaryPenetrateCount;
			FireBullets(info);
			pOperator->DoMuzzleFlash();
		}
		m_iClip1 = m_iClip1 - 1;
	}
	else if (pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0)
	{
		Fire9MMBullet();
	}

	if (bullettimesim.GetInt() == 1 && disable_bullettime.GetInt() == 1)
	{
		Fire9MMBullet();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;
	FireNPCPrimaryAttack(pOperator, true);
}

void CWeaponCustom::Fire9MMBullet( void )
{
	// m_vecEnemyLKP should be center of enemy body
	Vector vecArmPos;
	QAngle angArmDir;
	Vector vecDirToEnemy;
	QAngle angDir;

	if ( GetEnemy() )
	{
		Vector vecEnemyLKP = GetEnemy()->GetAbsOrigin();

		vecDirToEnemy = ( ( vecEnemyLKP ) - GetAbsOrigin() );
		VectorAngles( vecDirToEnemy, angDir );
		VectorNormalize( vecDirToEnemy );
	}
	else
	{
		angDir = GetAbsAngles();
		angDir.x = -angDir.x;

		Vector vForward;
		AngleVectors( angDir, &vForward );
		vecDirToEnemy = vForward;
	}

	DoMuzzleFlash();

	// make angles +-180
	if (angDir.x > 180)
	{
		angDir.x = angDir.x - 360;
	}

	VectorAngles( vecDirToEnemy, angDir );

	float RandomAngle = (rand() % 55960);
	float RandMagnitudeX = ((rand() % 70375) / 3800.0);
	float RandMagnitudeY = ((rand() % 70375) / 3800.0);
	angDir.x += (RandMagnitudeX)*cos(RandomAngle);
	angDir.y += (RandMagnitudeY)*sin(RandomAngle);

	AngleVectors(angDir, &vecDirToEnemy);

	GetAttachment( "muzzle", vecArmPos, angArmDir );

	vecArmPos = vecArmPos + vecDirToEnemy * 32;

	CBaseEntity *pBullet = CBaseEntity::Create( "bullet_9mm", vecArmPos, QAngle( 0, 0, 0 ), this );

	Vector vForward;
	AngleVectors( angDir, &vForward );
	
	pBullet->SetAbsVelocity( vForward * 1000 );
	pBullet->SetOwnerEntity( this );
			
	CSoundEnt::InsertSound( SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, this, SOUNDENT_CHANNEL_WEAPON, GetEnemy() );

	WeaponSound( SINGLE_NPC );

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		FireNPCPrimaryAttack(pOperator, true);
		break;
	}
	case EVENT_WEAPON_PISTOL_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, true);
		break;
	}
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
		FireNPCPrimaryAttack(pOperator, true);
		break;
	}
	case EVENT_WEAPON_AR2:
	{
		FireNPCPrimaryAttack(pOperator, true);
		break;
	}

	/*//FIXME: Re-enable
	case EVENT_WEAPON_AR2_GRENADE:
	{
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();
	vecShootDir = npc->GetShootEnemyDir( vecShootOrigin );

	Vector vecThrow = m_vecTossVelocity;

	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecShootOrigin, vec3_angle, npc );
	pGrenade->SetAbsVelocity( vecThrow );
	pGrenade->SetLocalAngularVelocity( QAngle( 0, 400, 0 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY );
	pGrenade->m_hOwner			= npc;
	pGrenade->m_pMyWeaponAR2	= this;
	pGrenade->SetDamage(sk_npc_dmg_ar2_grenade.GetFloat());

	// FIXME: arrgg ,this is hard coded into the weapon???
	m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.

	m_iClip2--;
	}
	break;
	*/

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
void CWeaponCustom::ShootBullets(bool isPrimary, bool usePrimaryAmmo)
{
	// Only the player fires this way so we can cast
	CHL2_Player *pPlayer = (CHL2_Player *)ToBasePlayer(GetOwner());
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (!pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0)
	{
		if (!pPlayer)
		{
			return;
		}

		if (m_iClip1 <= 0)
		{
			if (!m_bFireOnEmpty)
			{
				Reload();
			}
			else
			{
				WeaponSound(EMPTY);
				m_flNextPrimaryAttack = 0.15;
			}

			return;
		}

		pPlayer->m_iShotsFired++;
		pPlayer->m_flNextShotsClear = gpGlobals->curtime + 0.5;

		//if (GetWpnData().m_iWeaponType != 0)
		{
			m_iPrimaryAttacks++;
			gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

			WeaponSound(SINGLE);
			pPlayer->DoMuzzleFlash();

			SendWeaponAnim(!m_bIsLeftShoot ? ACT_VM_PRIMARYATTACK : ACT_VM_SECONDARYATTACK);
			pPlayer->SetAnimation(PLAYER_ATTACK1);

			if (GetWpnData().m_iWeaponType == 2)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
				m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			}
			else
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
				m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;
			}
			m_iClip1--;

			//DevMsg(CFmtStr("sin( %f / 2 ) = %f? \n", GetWpnData().m_flDefaultSpread, GetBulletSpreadPrimary().x));

			// Fire the bullets
			FireBulletsInfo_t info;
			if (isPrimary)
			{
				info.m_iShots = this->GetWpnData().m_sPrimaryShotCount;
				info.m_flDamage = this->GetWpnData().m_sPrimaryDamage;
				info.m_iPlayerDamage = this->GetWpnData().m_sPrimaryDamage;
				info.m_vecSpread = GetBulletSpreadPrimary();
				info.m_iPenetrationCount = this->GetWpnData().m_iPrimaryPenetrateCount;
				info.m_flPenetrationForce = this->GetWpnData().m_flPrimaryPenetrateDepth;
			}
			else
			{
				info.m_iShots = this->GetWpnData().m_sSecondaryShotCount;
				info.m_flDamage = this->GetWpnData().m_sSecondaryDamage;
				info.m_iPlayerDamage = this->GetWpnData().m_sSecondaryDamage;
				info.m_vecSpread = GetBulletSpreadSecondary();
				info.m_iPenetrationCount = this->GetWpnData().m_iSecondaryPenetrateCount;
				info.m_flPenetrationForce = this->GetWpnData().m_flSecondaryPenetrateDepth;
			}
			info.m_vecSrc = pPlayer->Weapon_ShootPosition();
			info.m_vecDirShooting = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
			info.m_flDistance = MAX_TRACE_LENGTH;
			info.m_iAmmoType = m_iPrimaryAmmoType;
			info.m_iTracerFreq = 2;
			FireBullets(info);

			pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

			if (pPlayer)
			{
				if (pPlayer->GetFlags() & FL_DUCKING)
				{
					pPlayer->KickBack(-RandomFloat(-GetWpnData().m_vRecoilPunchPitch.x, GetWpnData().m_vRecoilPunchPitch.y), RandomFloat(GetWpnData().m_vRecoilPunchYaw.x, GetWpnData().m_vRecoilPunchYaw.y), GetWpnData().m_flRecoilCrouch, GetWpnData().m_flRecoilCrouch, GetWpnData().m_flPunchLimit, GetWpnData().m_flPunchLimit, 0);
					//CalcRecoil(GetWpnData().m_flRecoilAmp * GetWpnData().m_flRecoilCrouch);
				}
				else
				{
					pPlayer->KickBack(-RandomFloat(GetWpnData().m_vRecoilPunchPitch.x, GetWpnData().m_vRecoilPunchPitch.y), RandomFloat(GetWpnData().m_vRecoilPunchYaw.x, GetWpnData().m_vRecoilPunchYaw.y), GetWpnData().m_flRecoilAmp, GetWpnData().m_flRecoilAmp, GetWpnData().m_flPunchLimit, GetWpnData().m_flPunchLimit, 0);
					//CalcRecoil(GetWpnData().m_flRecoilAmp);
				}
			}

			//pPlayer->ViewPunch(QAngle(-8, random->RandomFloat(-2, 2), 0));

			CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner());

			if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				// HEV suit - indicate out of ammo condition
				pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
			}
		}
	}
	else if (pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0)
	{
		if (!pPlayer)
		{
			return;
		}

		if (m_iClip1 <= 0)
		{
			if (!m_bFireOnEmpty)
			{
				Reload();
			}
			else
			{
				WeaponSound(EMPTY);
				m_flNextPrimaryAttack = 0.15;
			}

			return;
		}

		pPlayer->m_iShotsFired++;
		pPlayer->m_flNextShotsClear = gpGlobals->curtime + 0.5;

		//if (GetWpnData().m_iWeaponType != 0)
		{
			m_iPrimaryAttacks++;
			gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

			WeaponSound(SINGLE);
			pPlayer->DoMuzzleFlash();

			SendWeaponAnim(!m_bIsLeftShoot ? ACT_VM_PRIMARYATTACK : ACT_VM_SECONDARYATTACK);
			pPlayer->SetAnimation(PLAYER_ATTACK1);

			if (GetWpnData().m_iWeaponType == 2)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
				m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			}
			else
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
				m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;
			}
			m_iClip1--;

			//DevMsg(CFmtStr("sin( %f / 2 ) = %f? \n", GetWpnData().m_flDefaultSpread, GetBulletSpreadPrimary().x));

			// Fire the bullets
			/*
			FireBulletsInfo_t info;
			if (isPrimary)
			{
				info.m_iShots = this->GetWpnData().m_sPrimaryShotCount;
				info.m_flDamage = this->GetWpnData().m_sPrimaryDamage;
				info.m_iPlayerDamage = this->GetWpnData().m_sPrimaryDamage;
				info.m_vecSpread = GetBulletSpreadPrimary();
				info.m_iPenetrationCount = this->GetWpnData().m_iPrimaryPenetrateCount;
				info.m_flPenetrationForce = this->GetWpnData().m_flPrimaryPenetrateDepth;
			}
			else
			{
				info.m_iShots = this->GetWpnData().m_sSecondaryShotCount;
				info.m_flDamage = this->GetWpnData().m_sSecondaryDamage;
				info.m_iPlayerDamage = this->GetWpnData().m_sSecondaryDamage;
				info.m_vecSpread = GetBulletSpreadSecondary();
				info.m_iPenetrationCount = this->GetWpnData().m_iSecondaryPenetrateCount;
				info.m_flPenetrationForce = this->GetWpnData().m_flSecondaryPenetrateDepth;
			}
			info.m_vecSrc = pPlayer->Weapon_ShootPosition();
			info.m_vecDirShooting = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
			info.m_flDistance = MAX_TRACE_LENGTH;
			info.m_iAmmoType = m_iPrimaryAmmoType;
			info.m_iTracerFreq = 2;
			FireBullets(info);
			*/

			Vector vecAiming;
			vecAiming = pOwner->GetAutoaimVector(0);
			Vector vecSrc = pPlayer->Weapon_ShootPosition();

			QAngle angAiming;
			VectorAngles(vecAiming, angAiming);

			if (isPrimary)
			{
				CBullet9MM *pBullet = CBullet9MM::BulletCreate(vecSrc, angAiming, pPlayer);

				if (pPlayer->GetWaterLevel() == 3)
				{
					pBullet->SetAbsVelocity(vecAiming * 2000);
				}
				else
				{
					pBullet->SetAbsVelocity(vecAiming * 2000);
				}
			}
			else
			{
				CBullet9MM *pBullet = CBullet9MM::BulletCreate(vecSrc, angAiming, pPlayer);

				if (pPlayer->GetWaterLevel() == 3)
				{
					pBullet->SetAbsVelocity(vecAiming * 2000);
				}
				else
				{
					pBullet->SetAbsVelocity(vecAiming * 2000);
				}
			}

			pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

			if (pPlayer)
			{
				if (pPlayer->GetFlags() & FL_DUCKING)
				{
					pPlayer->KickBack(-RandomFloat(-GetWpnData().m_vRecoilPunchPitch.x, GetWpnData().m_vRecoilPunchPitch.y), RandomFloat(GetWpnData().m_vRecoilPunchYaw.x, GetWpnData().m_vRecoilPunchYaw.y), GetWpnData().m_flRecoilCrouch, GetWpnData().m_flRecoilCrouch, GetWpnData().m_flPunchLimit, GetWpnData().m_flPunchLimit, 0);
					//CalcRecoil(GetWpnData().m_flRecoilAmp * GetWpnData().m_flRecoilCrouch);
				}
				else
				{
					pPlayer->KickBack(-RandomFloat(GetWpnData().m_vRecoilPunchPitch.x, GetWpnData().m_vRecoilPunchPitch.y), RandomFloat(GetWpnData().m_vRecoilPunchYaw.x, GetWpnData().m_vRecoilPunchYaw.y), GetWpnData().m_flRecoilAmp, GetWpnData().m_flRecoilAmp, GetWpnData().m_flPunchLimit, GetWpnData().m_flPunchLimit, 0);
					//CalcRecoil(GetWpnData().m_flRecoilAmp);
				}
			}

			//pPlayer->ViewPunch(QAngle(-8, random->RandomFloat(-2, 2), 0));

			CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner());

			if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				// HEV suit - indicate out of ammo condition
				pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
			}
		}
	}

	if (bullettimesim.GetInt() == 1 && disable_bullettime.GetInt() == 1)
	{
		if (!pPlayer)
		{
			return;
		}

		if (m_iClip1 <= 0)
		{
			if (!m_bFireOnEmpty)
			{
				Reload();
			}
			else
			{
				WeaponSound(EMPTY);
				m_flNextPrimaryAttack = 0.15;
			}

			return;
		}

		pPlayer->m_iShotsFired++;
		pPlayer->m_flNextShotsClear = gpGlobals->curtime + 0.5;

		//if (GetWpnData().m_iWeaponType != 0)
		{
			m_iPrimaryAttacks++;
			gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

			WeaponSound(SINGLE);
			pPlayer->DoMuzzleFlash();

			SendWeaponAnim(!m_bIsLeftShoot ? ACT_VM_PRIMARYATTACK : ACT_VM_SECONDARYATTACK);
			pPlayer->SetAnimation(PLAYER_ATTACK1);

			if (GetWpnData().m_iWeaponType == 2)
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
				m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
			}
			else
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();
				m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;
			}
			m_iClip1--;

			//DevMsg(CFmtStr("sin( %f / 2 ) = %f? \n", GetWpnData().m_flDefaultSpread, GetBulletSpreadPrimary().x));

			// Fire the bullets
			/*
			FireBulletsInfo_t info;
			if (isPrimary)
			{
			info.m_iShots = this->GetWpnData().m_sPrimaryShotCount;
			info.m_flDamage = this->GetWpnData().m_sPrimaryDamage;
			info.m_iPlayerDamage = this->GetWpnData().m_sPrimaryDamage;
			info.m_vecSpread = GetBulletSpreadPrimary();
			info.m_iPenetrationCount = this->GetWpnData().m_iPrimaryPenetrateCount;
			info.m_flPenetrationForce = this->GetWpnData().m_flPrimaryPenetrateDepth;
			}
			else
			{
			info.m_iShots = this->GetWpnData().m_sSecondaryShotCount;
			info.m_flDamage = this->GetWpnData().m_sSecondaryDamage;
			info.m_iPlayerDamage = this->GetWpnData().m_sSecondaryDamage;
			info.m_vecSpread = GetBulletSpreadSecondary();
			info.m_iPenetrationCount = this->GetWpnData().m_iSecondaryPenetrateCount;
			info.m_flPenetrationForce = this->GetWpnData().m_flSecondaryPenetrateDepth;
			}
			info.m_vecSrc = pPlayer->Weapon_ShootPosition();
			info.m_vecDirShooting = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
			info.m_flDistance = MAX_TRACE_LENGTH;
			info.m_iAmmoType = m_iPrimaryAmmoType;
			info.m_iTracerFreq = 2;
			FireBullets(info);
			*/

			Vector vecAiming;
			vecAiming = pOwner->GetAutoaimVector(0);
			Vector vecSrc = pPlayer->Weapon_ShootPosition();

			QAngle angAiming;
			VectorAngles(vecAiming, angAiming);

			if (isPrimary)
			{
				CBullet9MM *pBullet = CBullet9MM::BulletCreate(vecSrc, angAiming, pPlayer);

				if (pPlayer->GetWaterLevel() == 3)
				{
					pBullet->SetAbsVelocity(vecAiming * 2000);
				}
				else
				{
					pBullet->SetAbsVelocity(vecAiming * 2000);
				}
			}
			else
			{
				CBullet9MM *pBullet = CBullet9MM::BulletCreate(vecSrc, angAiming, pPlayer);

				if (pPlayer->GetWaterLevel() == 3)
				{
					pBullet->SetAbsVelocity(vecAiming * 2000);
				}
				else
				{
					pBullet->SetAbsVelocity(vecAiming * 2000);
				}
			}

			pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

			if (pPlayer)
			{
				if (pPlayer->GetFlags() & FL_DUCKING)
				{
					pPlayer->KickBack(-RandomFloat(-GetWpnData().m_vRecoilPunchPitch.x, GetWpnData().m_vRecoilPunchPitch.y), RandomFloat(GetWpnData().m_vRecoilPunchYaw.x, GetWpnData().m_vRecoilPunchYaw.y), GetWpnData().m_flRecoilCrouch, GetWpnData().m_flRecoilCrouch, GetWpnData().m_flPunchLimit, GetWpnData().m_flPunchLimit, 0);
					//CalcRecoil(GetWpnData().m_flRecoilAmp * GetWpnData().m_flRecoilCrouch);
				}
				else
				{
					pPlayer->KickBack(-RandomFloat(GetWpnData().m_vRecoilPunchPitch.x, GetWpnData().m_vRecoilPunchPitch.y), RandomFloat(GetWpnData().m_vRecoilPunchYaw.x, GetWpnData().m_vRecoilPunchYaw.y), GetWpnData().m_flRecoilAmp, GetWpnData().m_flRecoilAmp, GetWpnData().m_flPunchLimit, GetWpnData().m_flPunchLimit, 0);
					//CalcRecoil(GetWpnData().m_flRecoilAmp);
				}
			}

			//pPlayer->ViewPunch(QAngle(-8, random->RandomFloat(-2, 2), 0));

			CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner());

			if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				// HEV suit - indicate out of ammo condition
				pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
			}
		}
	}
}

#ifdef HL2_DLL
extern int g_interactionPlayerLaunchedRPG;
#endif

void CWeaponCustom::ShootProjectile(bool isPrimary, bool usePrimaryAmmo)
{

	// Can't have an active missile out
	//if (m_hMissile != NULL)
	//	return;

	// Can't be reloading
	if (GetActivity() == ACT_VM_RELOAD)
		return;

	Vector vecOrigin;
	Vector vecForward;

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	Vector	vForward, vRight, vUp;

	pOwner->EyeVectors(&vForward, &vRight, &vUp);

	Vector	muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;

	QAngle vecAngles;
	VectorAngles(vForward, vecAngles);
	m_hMissile = CMissile::Create(muzzlePoint, vecAngles, GetOwner()->edict());

	//	m_hMissile->m_hOwner = this;

	// If the shot is clear to the player, give the missile a grace period
	trace_t	tr;
	Vector vecEye = pOwner->EyePosition();
	UTIL_TraceLine(vecEye, vecEye + vForward * 128, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction == 1.0)
	{
		m_hMissile->SetGracePeriod(0.3);
	}


	// Register a muzzleflash for the AI
	pOwner->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	SendWeaponAnim(!m_bIsLeftShoot ? ACT_VM_PRIMARYATTACK  : ACT_VM_SECONDARYATTACK);
	WeaponSound(SINGLE);

	pOwner->RumbleEffect(RUMBLE_SHOTGUN_SINGLE, 0, RUMBLE_FLAG_RESTART);

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pOwner, true, GetClassname());

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

	// Check to see if we should trigger any RPG firing triggers
	//Yes, these now work.
	int iCount = g_hWeaponFireTriggers.Count();
	for (int i = 0; i < iCount; i++)
	{
		if (g_hWeaponFireTriggers[i]->IsTouching(pOwner))
		{
			if (FClassnameIs(g_hWeaponFireTriggers[i], "trigger_rpgfire"))
			{
				g_hWeaponFireTriggers[i]->ActivateMultiTrigger(pOwner);
			}
		}
	}

	if (hl2_episodic.GetBool())
	{
		CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
		int nAIs = g_AI_Manager.NumAIs();

		string_t iszStriderClassname = AllocPooledString("npc_strider");

		for (int i = 0; i < nAIs; i++)
		{
			if (ppAIs[i]->m_iClassname == iszStriderClassname)
			{
				ppAIs[i]->DispatchInteraction(g_interactionPlayerLaunchedRPG, NULL, m_hMissile);
			}
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponCustom::GetPrimaryAttackActivity(void)
{
	//else
	return m_bIsLeftShoot ? ACT_VM_PRIMARYATTACK : ACT_VM_SECONDARYATTACK;
}

//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CWeaponCustom::Swing()
{
	bool isSecondary = false;
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetMeleeRange(isSecondary));

	Vector swingEnd = swingStart + forward * GetMeleeRange(isSecondary);
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = GetWpnData().m_actPrimary;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetMeleeDamage(isSecondary), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

																// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = GetWpnData().m_actPrimary;
			}
		}
	}

	if (!isSecondary)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !isSecondary, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = GetWpnData().m_actPrimary;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetMeleeRange(isSecondary);

		// See if we happened to hit water
		//ImpactWater(swingStart, testEnd);
	}
	else
	{
		Hit(traceHit, nHitActivity, isSecondary);
	}

	// Send the anim
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetMeleeFireRate(isSecondary);
	m_flNextSecondaryAttack = gpGlobals->curtime + GetMeleeFireRate(isSecondary);

	//Play swing sound
	WeaponSound(SPECIAL1);
}

//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CWeaponCustom::SwingSecondary()
{
	bool isSecondary = true;
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetMeleeRange(isSecondary));

	Vector swingEnd = swingStart + forward * GetMeleeRange(isSecondary);
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = GetWpnData().m_actSecondary;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetMeleeDamage(isSecondary), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

																// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = GetWpnData().m_actSecondary;
			}
		}
	}

	if (!isSecondary)
	{
		m_iPrimaryAttacks++;
	}
	else
	{
		m_iSecondaryAttacks++;
	}

	gamestats->Event_WeaponFired(pOwner, !isSecondary, GetClassname());

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = GetWpnData().m_actSecondary;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetMeleeRange(isSecondary);

		// See if we happened to hit water
		//ImpactWater(swingStart, testEnd);
	}
	else
	{
		Hit(traceHit, nHitActivity, isSecondary);
	}

	// Send the anim
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetMeleeFireRate(isSecondary);
	m_flNextSecondaryAttack = gpGlobals->curtime + GetMeleeFireRate(isSecondary);

	//Play swing sound
	WeaponSound(SPECIAL1);
}

void CWeaponCustom::Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	//Do view kick
	AddViewKick();

	//Make sound for the AI
	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);

	// This isn't great, but it's something for when the crowbar hits.
	pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);

	CBaseEntity	*pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if (pHitEntity != NULL)
	{
		Vector hitDirection;
		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
		VectorNormalize(hitDirection);

		CTakeDamageInfo info(GetOwner(), GetOwner(), GetMeleeDamage(bIsSecondary), DMG_CLUB);

		if (pPlayer && pHitEntity->IsNPC())
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce(&info, hitDirection, traceHit.endpos);

		pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit);
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers(info, traceHit.startpos, traceHit.endpos, hitDirection);

		if (ToBaseCombatCharacter(pHitEntity))
		{
			gamestats->Event_WeaponHit(pPlayer, !bIsSecondary, GetClassname(), info);
		}
	}

	UTIL_ImpactTrace(&traceHit, GetMeleeDamage(bIsSecondary));
}

void CWeaponCustom::PrimaryAttack(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (GetWpnData().m_iWeaponType  == 0)
	{
		if (pOwner->m_afButtonPressed & IN_ATTACK)
		{
			if (IsPrimaryBullet())
				ShootBullets(true, true);
			else
				ShootProjectile(true, true);
		}
	}
	else if (GetWpnData().m_sPrimaryMeleeEnabled)
	{
		if (pOwner->m_afButtonPressed & IN_ATTACK)
		{
			Swing();
		}
	}
	else
	{
		if (this->GetWpnData().m_sPrimaryMissleEnabled)
			ShootProjectile(true, true);
		else if (IsPrimaryBullet() == true && pOwner->m_nButtons & IN_ATTACK)
		{
			ShootBullets(true, true);
		}
	}

	if (GetWpnData().bIsAkimbo)
	{
		if (!m_bIsLeftShoot)
			m_bIsLeftShoot = true;
		if (m_bIsLeftShoot)
			m_bIsLeftShoot = false;
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponCustom::Reload(void)
{
	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound(RELOAD);
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::AddViewKick(void)
{

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::SecondaryAttack(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (GetWpnData().m_sSecondaryMeleeEnabled)
	{
		if (pOwner->m_afButtonPressed & IN_ATTACK2)
		{
			SwingSecondary();
		}
	}
	else if (!GetWpnData().bLaserUseIronsight && GetWpnData().bLaser)
	{
		ToggleGuiding();
	}
	else if (IsSecondaryBullet())
	{
		ShootBullets(false, this->GetWpnData().m_sUsePrimaryAmmo);
	}
	else return;
}

void CWeaponCustom::ToggleIronsights(void)
{
	if (m_bIsIronsighted)
		DisableIronsights();
	else
		EnableIronsights();
}

void CWeaponCustom::EnableIronsights(void)
{
	BaseClass::EnableIronsights();

	if (GetWpnData().bLaserUseIronsight && GetWpnData().bLaser)
	{
		StartGuiding();
	}
}

void CWeaponCustom::DisableIronsights(void)
{
	BaseClass::DisableIronsights();
	if (GetWpnData().bLaserUseIronsight && GetWpnData().bLaser)
	{
		StopGuiding();
	}
}
#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flsight - 
//			flDot - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponCustom::WeaponRangeAttack2Condition(float flDot, float flDist)
{
	return COND_NONE;
}

void CWeaponCustom::Activate(void)
{
	BaseClass::Activate();

	// Restore the laser pointer after transition
	if (m_bSighting)
	{
		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner == NULL)
			return;

		if (pOwner->GetActiveWeapon() == this)
		{
			StartGuiding();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCustom::UpdateLaserPosition(Vector vecMuzzlePos, Vector vecEndPos)
{
	if (vecMuzzlePos == vec3_origin || vecEndPos == vec3_origin)
	{
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
			return;

		vecMuzzlePos = pPlayer->Weapon_ShootPosition();
		Vector	forward;

		if (g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE)
		{
			forward = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
		}
		else
		{
			pPlayer->EyeVectors(&forward);
		}

		vecEndPos = vecMuzzlePos + (forward * MAX_TRACE_LENGTH);
	}

	//Move the laser sight, if active
	trace_t	tr;

	// Trace out for the endpoint
#ifdef PORTAL
	g_bBulletPortalTrace = true;
	Ray_t rayLaser;
	rayLaser.Init(vecMuzzlePos, vecEndPos);
	UTIL_Portal_TraceRay(rayLaser, (MASK_SHOT & ~CONTENTS_WINDOW), this, COLLISION_GROUP_NONE, &tr);
	g_bBulletPortalTrace = false;
#else
	UTIL_TraceLine(vecMuzzlePos, vecEndPos, (MASK_SHOT & ~CONTENTS_WINDOW), this, COLLISION_GROUP_NONE, &tr);
#endif

	// Move the laser sprite
	if (m_hLaserSight != NULL)
	{
		Vector	laserPos = tr.endpos;
		m_hLaserSight->SetLaserPosition(laserPos, tr.plane.normal);

		if (tr.DidHitNonWorldEntity())
		{
			CBaseEntity *pHit = tr.m_pEnt;

			if ((pHit != NULL) && (pHit->m_takedamage))
			{
				m_hLaserSight->SetTargetEntity(pHit);
			}
			else
			{
				m_hLaserSight->SetTargetEntity(NULL);
			}
		}
		else
		{
			m_hLaserSight->SetTargetEntity(NULL);
		}
	}
}

void CWeaponCustom::StartGuiding(void)
{
	// Don't start back up if we're overriding this
	//if (m_bHideGuiding)
	//	return;
	Msg("Started Guiding\n");

	m_bSighting = true;

	WeaponSound(SPECIAL1);

	CreateLaserPointer();
	//StartLaserEffects();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true if the rocket is being guided, false if it's dumb
//-----------------------------------------------------------------------------
bool CWeaponCustom::IsGuiding(void)
{
	return m_bSighting;
}

//-----------------------------------------------------------------------------
// Purpose: Turn off the guiding laser
//-----------------------------------------------------------------------------
void CWeaponCustom::StopGuiding(void)
{
	m_bSighting = false;
	Msg("Stoped Guiding\n");

	WeaponSound(SPECIAL2);

	//StopLaserEffects();

	// Kill the sight completely
	if (m_hLaserSight != NULL)
	{
		m_hLaserSight->TurnOff();
		UTIL_Remove(m_hLaserSight);
		m_hLaserSight = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Toggle the guiding laser
//-----------------------------------------------------------------------------
void CWeaponCustom::ToggleGuiding(void)
{
	if (IsGuiding())
	{
		StopGuiding();
	}
	else
	{
		StartGuiding();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponCustom::CreateLaserPointer(void)
{
	if (m_hLaserSight != NULL)
		return;

	Msg("Created Laser Pointer\n");

	m_hLaserSight = CLaserSight::Create(GetAbsOrigin(), GetOwnerEntity(), true);
	m_hLaserSight->TurnOff();

	UpdateLaserPosition();
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponCustom::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75 },
	{ 5.00,		0.75 },
	{ 10.0 / 3.0, 0.75 },
	{ 5.0 / 3.0,	0.75 },
	{ 1.00,		1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

LINK_ENTITY_TO_CLASS(env_lasersight, CLaserSight);

BEGIN_DATADESC(CLaserSight)
DEFINE_FIELD(m_vecSurfaceNormal, FIELD_VECTOR),
DEFINE_FIELD(m_hTargetEnt, FIELD_EHANDLE),
DEFINE_FIELD(m_bVisibleLaserSight, FIELD_BOOLEAN),
//DEFINE_FIELD(m_bIsOn, FIELD_BOOLEAN),

//DEFINE_FIELD( m_pNext, FIELD_CLASSPTR ),	// don't save - regenerated by constructor
DEFINE_THINKFUNC(LaserThink),
END_DATADESC()

// a list of laser sights to search quickly
CEntityClassList<CLaserSight> g_LaserSightList;
template <>  CLaserSight *CEntityClassList<CLaserSight>::m_pClassList = NULL;
CLaserSight *GetLasersightList()
{
	return g_LaserSightList.m_pClassList;
}

const char *g_pLaserSightThink = "LaserThinkContext";


//-----------------------------------------------------------------------------
// Finds missiles in cone
//-----------------------------------------------------------------------------
CBaseEntity *CreateLaserSight(const Vector &origin, CBaseEntity *pOwner, bool bVisiblesight)
{
	return CLaserSight::Create(origin, pOwner, bVisiblesight);
}

void SetLaserSightTarget(CBaseEntity *pLasersight, CBaseEntity *pTarget)
{
	CLaserSight *psight = assert_cast< CLaserSight* >(pLasersight);
	psight->SetTargetEntity(pTarget);
}

void EnableLaserSight(CBaseEntity *pLasersight, bool bEnable)
{
	CLaserSight *psight = assert_cast< CLaserSight* >(pLasersight);
	if (bEnable)
	{
		psight->TurnOn();
	}
	else
	{
		psight->TurnOff();
	}
}

CLaserSight::CLaserSight(void)
{
	m_hTargetEnt = NULL;
	//m_bIsOn = true;
	g_LaserSightList.Insert(this);
}

CLaserSight::~CLaserSight(void)
{
	g_LaserSightList.Remove(this);
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
// Output : CLaserSight
//-----------------------------------------------------------------------------
CLaserSight *CLaserSight::Create(const Vector &origin, CBaseEntity *pOwner, bool bVisiblesight)
{

	Msg("Created Laser\n");
	CLaserSight *pLasersight = (CLaserSight *)CBaseEntity::Create("env_lasersight", origin, QAngle(0, 0, 0));

	if (pLasersight == NULL)
		return NULL;

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
		return NULL;

	CBaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon();
	if (pWeapon)
		return NULL;

	pLasersight->m_bVisibleLaserSight = bVisiblesight;
	pLasersight->SetMoveType(MOVETYPE_NONE);
	pLasersight->AddSolidFlags(FSOLID_NOT_SOLID);
	pLasersight->AddEffects(EF_NOSHADOW);
	UTIL_SetSize(pLasersight, vec3_origin, vec3_origin);

	//Create the graphic
	pLasersight->SpriteInit(pWeapon->GetWpnData().sLaserMaterial, origin);

	pLasersight->SetName(AllocPooledString("TEST"));

	pLasersight->SetTransparency(kRenderGlow, 255, 255, 255, 255, kRenderFxNoDissipation);
	pLasersight->SetScale(pWeapon->GetWpnData().flLaserPointerSize);

	pLasersight->SetOwnerEntity(pOwner);

	pLasersight->SetContextThink(&CLaserSight::LaserThink, gpGlobals->curtime + 0.1f, g_pLaserSightThink);
	pLasersight->SetSimulatedEveryTick(true);

	if (!bVisiblesight)
	{
		pLasersight->MakeInvisible();
	}

	return pLasersight;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserSight::LaserThink(void)
{
	SetNextThink(gpGlobals->curtime + 0.05f, g_pLaserSightThink);

	if (GetOwnerEntity() == NULL)
		return;

	Vector	viewDir = GetAbsOrigin() - GetOwnerEntity()->GetAbsOrigin();
	float	dist = VectorNormalize(viewDir);

	float	scale = RemapVal(dist, 32, 1024, 0.01f, 0.5f);
	float	scaleOffs = random->RandomFloat(-scale * 0.25f, scale * 0.25f);

	scale = clamp(scale + scaleOffs, 0.1f, 32.0f);

	SetScale(scale);
}

void CLaserSight::SetLaserPosition(const Vector &origin, const Vector &normal)
{
	SetAbsOrigin(origin);
	m_vecSurfaceNormal = normal;
}

Vector CLaserSight::GetChasePosition()
{
	return GetAbsOrigin() - m_vecSurfaceNormal * 10;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserSight::TurnOn(void)
{
	Msg("Turned On Laser\n");
	//m_bIsOn = true;
	//if (m_bVisibleLaserSight)
	{
		BaseClass::TurnOn();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserSight::TurnOff(void)
{
	Msg("Turned Off Laser\n");
	//m_bIsOn = false;
	//if (m_bVisibleLaserSight)
	{
		BaseClass::TurnOff();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CLaserSight::MakeInvisible(void)
{
	BaseClass::TurnOff();
}
