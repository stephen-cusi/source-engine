//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"

#ifdef CLIENT_DLL
#include "c_basehlcombatweapon.h"
#else
#include "grenade_hopwire.h"
#include "basehlcombatweapon.h"
#include "npcevent.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_frag.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "items.h"
#include "in_buttons.h"
#include "soundent.h"
#endif

#define GRENADE_TIMER	2.0f //Seconds

//-----------------------------------------------------------------------------
// Hopwire grenade
//-----------------------------------------------------------------------------
#ifdef CLIENT_DLL
#define CWeaponHopWire C_WeaponHopWire
#endif

class CWeaponHopWire : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponHopWire, CBaseHLCombatWeapon);

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	void	Precache( void );
#ifndef CLIENT_DLL
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
#endif
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );
	bool	Deploy();
	bool	Holster( CBaseCombatWeapon *pSwitchingTo );

#ifndef CLIENT_DLL
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
#endif

	bool	Reload( void );

private:
	
	void	ThrowGrenade( CBasePlayer *pPlayer );
	void	RollGrenade( CBasePlayer *pPlayer );
	void	LobGrenade( CBasePlayer *pPlayer );

	bool	m_bRedraw;	//Draw the weapon again after throwing a grenade

#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif
};

#ifndef CLIENT_DLL

acttable_t	CWeaponHopWire::m_acttable[] = 
{
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_GRENADE,					false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_GRENADE,					false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_GRENADE,			false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_GRENADE,			false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_GRENADE,	false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_GRENADE,		false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_GRENADE,					false },
};

IMPLEMENT_ACTTABLE(CWeaponHopWire);

#endif

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponHopWire, DT_WeaponHopWire);

BEGIN_NETWORK_TABLE(CWeaponHopWire, DT_WeaponHopWire)
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponHopWire )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_hopwire, CWeaponHopWire );
PRECACHE_WEAPON_REGISTER( weapon_hopwire );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHopWire::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "npc_grenade_hopwire" );
#endif
	
	m_bRedraw = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponHopWire::Deploy()
{
	m_bRedraw = false;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHopWire::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
#ifndef CLIENT_DLL
void CWeaponHopWire::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	switch( pEvent->event )
	{
		case EVENT_WEAPON_THROW:
			ThrowGrenade( pOwner );
			DecrementAmmo( pOwner );
			break;

		case EVENT_WEAPON_THROW2:
			RollGrenade( pOwner );
			DecrementAmmo( pOwner );
			break;

		case EVENT_WEAPON_THROW3:
			LobGrenade( pOwner );
			DecrementAmmo( pOwner );
			break;

		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHopWire::Reload( void )
{
	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHopWire::SecondaryAttack( void )
{
	if ( m_bRedraw )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( pOwner == NULL )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pOwner );
	
	if ( pPlayer == NULL )
		return;

	//See if we're ducking
	if ( pPlayer->GetFlags() & FL_DUCKING )
	{
		//Send the weapon animation
		SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	}
	else
	{
		//Send the weapon animation
		SendWeaponAnim( ACT_VM_HAULBACK );
	}

	m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHopWire::PrimaryAttack( void )
{
	if ( m_bRedraw )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
	{ 
		return;
	}

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );;

	if ( pPlayer == NULL )
		return;

	SendWeaponAnim( ACT_VM_THROW );
	
	m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponHopWire::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
#ifndef CLIENT_DLL
		pOwner->Weapon_Drop( this );
		UTIL_Remove(this);
#endif
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHopWire::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( m_bRedraw )
	{
		Reload();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHopWire::ThrowGrenade( CBasePlayer *pPlayer )
{
	Vector	vecSrc = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	vecSrc += vForward * 18.0f + vRight * 8.0f;

	vForward[2] += 0.1f;

	Vector vecThrow = vForward * 1200 + pPlayer->GetAbsVelocity();
#ifndef CLIENT_DLL	
	HopWire_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(600,random->RandomInt(-1200,1200),0), pPlayer, GRENADE_TIMER );
#endif
	m_bRedraw = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHopWire::LobGrenade( CBasePlayer *pPlayer )
{
	Vector	vecSrc = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	vecSrc += vForward * 18.0f + vRight * 8.0f + Vector( 0, 0, -8 );
	
	Vector vecThrow = vForward * 350 + pPlayer->GetAbsVelocity() + Vector( 0, 0, 50 );
#ifndef CLIENT_DLL	
	HopWire_Create( vecSrc, vec3_angle, vecThrow, AngularImpulse(200,random->RandomInt(-600,600),0), pPlayer, GRENADE_TIMER );
#endif
	// Play throw sound
	EmitSound( "WeaponFrag.Throw" );

	m_bRedraw = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHopWire::RollGrenade( CBasePlayer *pPlayer )
{
	// BUGBUG: Hardcoded grenade width of 4 - better not change the model :)
	Vector vecSrc( pPlayer->WorldSpaceCenter().x, pPlayer->WorldSpaceCenter().y, pPlayer->GetAbsOrigin().z + 4 );
#ifndef CLIENT_DLL
	Vector vecFacing = pPlayer->BodyDirection2D( );
	// no up/down direction
	vecFacing.z = 0;
	VectorNormalize( vecFacing );
	vecSrc = vecSrc + (vecFacing * 18.0);
	trace_t tr;
	UTIL_TraceLine( vecSrc, vecSrc - Vector(0,0,16), MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if ( tr.fraction != 1.0 )
	{
		// compute forward vec parallel to floor plane and roll grenade along that
		Vector tangent;
		CrossProduct( vecFacing, tr.plane.normal, tangent );
		CrossProduct( tr.plane.normal, tangent, vecFacing );
	}
	Vector vecThrow = vecFacing * 500 + pPlayer->GetAbsVelocity();
	// put it on its side
	QAngle orientation(0,pPlayer->GetLocalAngles().y,-90);
	// roll it
	AngularImpulse rotSpeed(0,0,720);
	HopWire_Create( vecSrc, orientation, vecThrow, rotSpeed, pPlayer, GRENADE_TIMER );
#endif

	// Play throw sound
	EmitSound( "WeaponFrag.Roll" );

	m_bRedraw = true;
}

