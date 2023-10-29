//========= Copyright Valve Corporation, All rights reserved. ============//
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
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_weapon_smg2_firerate("sk_weapon_smg2_firerate", "0.065");

class CWeaponSMG2 : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS(CWeaponSMG2, CHLSelectFireMachineGun);

	CWeaponSMG2();

	DECLARE_SERVERCLASS();

	const Vector& GetBulletSpread(void);
	void			Precache(void);
	void			AddViewKick(void);
	void			Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);
	float			GetFireRate(void);

	int				CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	int		GetMinBurst() { return 10; }
	int		GetMaxBurst() { return 30; }

	bool Reload();

	void SecondaryAttack(void);

	DECLARE_ACTTABLE();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponSMG2, DT_WeaponSMG2)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_smg2, CWeaponSMG2);
PRECACHE_WEAPON_REGISTER(weapon_smg2);

acttable_t	CWeaponSMG2::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SMG1,			false },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				false },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					false },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			false },

	{ ACT_WALK,						ACT_WALK_RIFLE,					false },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				false  },

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

		{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				false },
		{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			false },
		{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		false },
		{ ACT_RUN,						ACT_RUN_RIFLE,					false },
		{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				false },
		{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			false },
		{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		false },
		{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	false },
		{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		false },
		{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
		{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
		{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
		{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		false },
};


IMPLEMENT_ACTTABLE(CWeaponSMG2);

//=========================================================
CWeaponSMG2::CWeaponSMG2()
{
	m_fMaxRange1 = 2000;
	m_fMinRange1 = 32;

	m_iFireMode = FIREMODE_FULLAUTO;
	m_bIsSilenced = true;

}

void CWeaponSMG2::Precache(void)
{
	BaseClass::Precache();
}

float CWeaponSMG2::GetFireRate(void)
{
	switch (m_iFireMode)
	{
	case FIREMODE_SEMI:
		// the time between rounds fired on semi auto
		return 0.1f;	// 600 rounds per minute = 0.1 seconds per bullet
		break;

	case FIREMODE_3RNDBURST:
		// the time between rounds fired within a single burst
		return 0.05f;
		break;

	default:
		return sk_weapon_smg2_firerate.GetFloat();
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponSMG2::SecondaryAttack(void)
{
	// change fire modes.

	switch (m_iFireMode)
	{
	case FIREMODE_SEMI:
		m_iFireMode = FIREMODE_FULLAUTO;
		WeaponSound(SPECIAL1);
		break;

	case FIREMODE_3RNDBURST:
		m_iFireMode = FIREMODE_SEMI;
		WeaponSound(SPECIAL1);
		break;

	case FIREMODE_FULLAUTO:
		m_iFireMode = FIREMODE_3RNDBURST;
		WeaponSound(SPECIAL2);
		break;
	}

	SendWeaponAnim(GetSecondaryAttackActivity());

	m_flNextSecondaryAttack = gpGlobals->curtime + 0.3;

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		m_iSecondaryAttacks++;
		gamestats->Event_WeaponFired(pOwner, false, GetClassname());
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : const Vector
//-----------------------------------------------------------------------------
const Vector& CWeaponSMG2::GetBulletSpread(void)
{
	//	static const Vector cone = VECTOR_CONE_10DEGREES;

	//	Vector cone = VECTOR_CONE_10DEGREES;

	static const Vector cone1 = VECTOR_CONE_1DEGREES;
	static const Vector cone2 = VECTOR_CONE_2DEGREES;
	static const Vector cone3 = VECTOR_CONE_6DEGREES;
	static Vector npcCone = VECTOR_CONE_10DEGREES;

	if (GetOwner() && GetOwner()->IsNPC())
		return npcCone;

	switch (m_iFireMode)
	{
	case FIREMODE_SEMI:
		return cone1;
		break;

	case FIREMODE_3RNDBURST:
		return cone2;
		break;

	case FIREMODE_FULLAUTO:
		return cone3;
		break;
	}

	return cone3;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponSMG2::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC* npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		WeaponSound(SINGLE_NPC);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

bool CWeaponSMG2::Reload()
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
	}
	return BaseClass::Reload();
}



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponSMG2::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	2.0f	//Degrees
#define	SLIDE_LIMIT			1.0f	//Seconds

	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}
