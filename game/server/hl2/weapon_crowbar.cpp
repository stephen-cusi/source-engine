//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Crowbar - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_crowbar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_plr_dmg_crowbar		( "sk_plr_dmg_crowbar","0");
ConVar    sk_npc_dmg_crowbar		( "sk_npc_dmg_crowbar","0");

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponCrowbar, DT_WeaponCrowbar)
END_SEND_TABLE()

#ifndef HL2MP
LINK_ENTITY_TO_CLASS( weapon_crowbar, CWeaponCrowbar );
PRECACHE_WEAPON_REGISTER( weapon_crowbar );
#endif

acttable_t CWeaponCrowbar::m_acttable[] = 
{
	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_MELEE, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_MELEE, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_MELEE, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_MELEE, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_MELEE, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_MELEE, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_MELEE, false },
};

IMPLEMENT_ACTTABLE(CWeaponCrowbar);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponCrowbar::CWeaponCrowbar( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponCrowbar::GetDamageForActivity( Activity hitActivity )
{
	if ( ( GetOwner() != NULL ) && ( GetOwner()->IsPlayer() ) )
		return sk_plr_dmg_crowbar.GetFloat();

	return sk_npc_dmg_crowbar.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponCrowbar::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat( 1.0f, 2.0f );
	punchAng.y = random->RandomFloat( -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng ); 
}


//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
//-----------------------------------------------------------------------------
ConVar sk_crowbar_lead_time( "sk_crowbar_lead_time", "0.9" );

int CWeaponCrowbar::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_crowbar_lead_time.GetFloat();
	dt += random->RandomFloat( -0.3f, 0.2f );
	if ( dt < 0.0f )
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta );

	if ( fabs( vecDelta.z ) > 70 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D() );
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D(), vecForward.AsVector2D() );
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}


//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponCrowbar::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles(), &vecDirection );

	CBaseEntity *pEnemy = pOperator->MyNPCPointer() ? pOperator->MyNPCPointer()->GetEnemy() : NULL;
	if ( pEnemy )
	{
		Vector vecDelta;
		VectorSubtract( pEnemy->WorldSpaceCenter(), pOperator->Weapon_ShootPosition(), vecDelta );
		VectorNormalize( vecDelta );
		
		Vector2D vecDelta2D = vecDelta.AsVector2D();
		Vector2DNormalize( vecDelta2D );
		if ( DotProduct2D( vecDelta2D, vecDirection.AsVector2D() ) > 0.8f )
		{
			vecDirection = vecDelta;
		}
	}

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd );
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition(), vecEnd, 
		Vector(-16,-16,-16), Vector(36,36,36), sk_npc_dmg_crowbar.GetFloat(), DMG_CLUB, 0.75 );
	
	// did I hit someone?
	if ( pHurt )
	{
		// play sound
		WeaponSound( MELEE_HIT );

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine( pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit );
		ImpactEffect( traceHit );
	}
	else
	{
		WeaponSound( MELEE_MISS );
	}
}


//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponCrowbar::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit( pEvent, pOperator );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

void CWeaponCrowbar::SecondaryAttack()
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	Vector eyepos, qqwee, right, forward, impulse, cthrow;
	eyepos = pPlayer->EyePosition();
	pPlayer->EyeVectors( &forward, &right, NULL );
	
	trace_t tr;
	UTIL_TraceLine(qqwee, qqwee + forward * MAX_TRACE_LENGTH, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);


	qqwee = eyepos + forward * 20.0f + right * 10.0f;
	pPlayer->GetVelocity( &cthrow, NULL );
	cthrow += forward * 1984 + Vector( 0, 0, 50 );
	AngularImpulse velocity = AngularImpulse( 600, 1200, 0 );
	Drop( pPlayer->GetActiveWeapon(), &tr.endpos, &velocity );	
	SetVelocity( cthrow, velocity );
}

void CWeaponCrowbar::Drop( CBaseCombatWeapon *pWeapon, const Vector *target, const Vector *velocity )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner( ) );

	if ( !pWeapon )
		return;

	if ( IsPlayer() )
	{
		Vector forward, right;
		pPlayer->EyeVectors( &forward, &right, NULL );
		Vector vThrowPos = forward + right - Vector( 0, 0, 12 );

		if( UTIL_PointContents(vThrowPos) & CONTENTS_SOLID )
		{
			Msg("Weapon spawning in solid!\n");
		}

		pWeapon->SetAbsOrigin( vThrowPos );

		QAngle gunAngles;
		VectorAngles( pPlayer->BodyDirection2D(), gunAngles );
		pWeapon->SetAbsAngles( gunAngles );
	}

	Vector vecThrow;
	if ( target )
	{
		// I've been told to throw it somewhere specific.
		vecThrow = VecCheckToss( this, pWeapon->GetAbsOrigin(), *target, 0.2, 1.0, false );
	}
	else
	{
		if ( velocity )
		{
			vecThrow = *velocity;
			float flLen = vecThrow.Length();
			if (flLen > 400)
			{
				VectorNormalize(vecThrow);
				//vecThrow *= 1984;
				vecThrow = pPlayer->BodyDirection3D() * 1984;
			}
		}
	}

	pWeapon->Drop( vecThrow );
	pPlayer->Weapon_Detach( pWeapon );

	CBaseViewModel *vm = pPlayer->GetViewModel();
	if ( vm )
	{
		vm->AddEffects( EF_NODRAW );
	}
}

void CWeaponCrowbar::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if ( pPhysicsObject != NULL )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}