//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:	Stickylauncher (Demoman Time? :troll: )
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "in_buttons.h"

#ifndef CLIENT_DLL
#include "npcevent.h"
#include "grenade_stickybomb.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "vstdlib/random.h"
#include "explode.h"
#include "shake.h"
#include "model_types.h"
#include "studio.h"
#else 
#include "basehlcombatweapon_shared.h"
#endif

#define	PISTOL_FASTEST_REFIRE_TIME		0.1f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

#ifndef CLIENT_DLL
void LaunchStickyBomb( CBaseCombatCharacter *pOwner, const Vector &origin, const Vector &direction )
{
	CStickyBomb *pGrenade = (CStickyBomb *)CBaseEntity::Create( "grenade_stickybomb", origin, vec3_angle, pOwner );
	pGrenade->SetVelocity( direction * 1200, vec3_origin );
}
#endif

//-----------------------------------------------------------------------------
// CWeaponStickyLauncher
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
#define CWeaponStickyLauncher C_WeaponStickyLauncher
#endif

class CWeaponStickyLauncher : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponStickyLauncher, CBaseHLCombatWeapon );
public:

	CWeaponStickyLauncher(void);

	DECLARE_PREDICTABLE();
	DECLARE_NETWORKTABLE();

	void	ItemPostFrame();
	void	PrimaryAttack();
	void	SecondaryAttack();
	void	AddViewKick();
	void	DryFire();

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_3DEGREES;
		return cone;
	}
	
	virtual float GetFireRate( void ) { return 1.0f; }
#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
};

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponStickyLauncher )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_stickylauncher, CWeaponStickyLauncher );
PRECACHE_WEAPON_REGISTER( weapon_stickylauncher );

#ifndef CLIENT_DLL
acttable_t CWeaponStickyLauncher::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_PISTOL,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_PISTOL,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_PISTOL,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_PISTOL,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_PISTOL,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_PISTOL,					false },
	{ ACT_RANGE_ATTACK1,				ACT_RANGE_ATTACK_PISTOL,				false },
};
IMPLEMENT_ACTTABLE( CWeaponStickyLauncher );
#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponStickyLauncher::CWeaponStickyLauncher( void )
{
	m_fMinRange1 = 65;
	m_fMinRange2 = 65;
	m_fMaxRange1 = 512;
	m_fMaxRange2 = 512;

	m_bFiresUnderwater = true;
}

void CWeaponStickyLauncher::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

void CWeaponStickyLauncher::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
		return;

	m_flNextPrimaryAttack = gpGlobals->curtime + GetFireRate();

	WeaponSound(SINGLE);
//	pPlayer->m_fEffects |= EF_MUZZLEFLASH;

	SendWeaponAnim( GetPrimaryAttackActivity() );

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc = pPlayer->Weapon_ShootPosition( );
	Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	m_iClip1 = m_iClip1 - 1;
#ifndef CLIENT_DLL
	LaunchStickyBomb( pPlayer, vecSrc, vecAiming );
#endif	
	AddViewKick();
}

void CWeaponStickyLauncher::SecondaryAttack()
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
#ifndef CLIENT_DLL 	
	CStickyBomb::DetonateByOperator( GetOwner() );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponStickyLauncher::ItemPostFrame( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//Allow a refire as fast as the player can click
	if ( !( pOwner->m_nButtons & IN_ATTACK ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
	else if ( ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack < gpGlobals->curtime ) && ( m_iClip1 <= 0 ) )
	{
		DryFire();
		return;
	}

	BaseClass::ItemPostFrame();
}

/*
==================================================
AddViewKick
==================================================
*/

void CWeaponStickyLauncher::AddViewKick( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle viewPunch;

	viewPunch.x = random->RandomFloat( -2.0f, -1.0f );
	viewPunch.y = random->RandomFloat( 0.5f, 1.0f );
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}
