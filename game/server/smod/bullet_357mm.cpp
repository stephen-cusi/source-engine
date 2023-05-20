//========= Copyright © 2018, Aulov Nikolay, All rights reserved. ============//
//
// Purpose: 9mm Bullet!!!
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "decals.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "bullet_357mm.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "beam_shared.h"
#include "rumble_shared.h"
#include "coolmod/smod_cvars.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define BULLET_357MODEL	"models/weapons/bt_357.mdl"

#define BOLT_AIR_VELOCITY	2500
#define BOLT_WATER_VELOCITY	1500

extern ConVar sk_plr_dmg_crossbow;
extern ConVar sk_npc_dmg_crossbow;

ConVar sk_plr_dmg_bullet_357mm( "sk_plr_dmg_bullet_357mm", "40" );
ConVar sk_npc_dmg_bullet_357mm( "sk_npc_dmg_bullet_357mm", "40" );

void TE_StickyBolt( IRecipientFilter& filter, float delay,	Vector vecDirection, const Vector *origin );

#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1

BEGIN_DATADESC( CBullet357MM )
	// Function Pointers
	DEFINE_FUNCTION( BulletThink ),
	DEFINE_FUNCTION( BulletTouch ),
	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

END_DATADESC()

/*
IMPLEMENT_SERVERCLASS_ST( CBullet357MM, DT_Bullet357MM )
END_SEND_TABLE()
*/

CBullet357MM *CBullet357MM::BulletCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner )
{
	// Create a new entity with CBullet357MM private data
	CBullet357MM *pBullet = (CBullet357MM *)CreateEntityByName( "bullet_357mm" );
	UTIL_SetOrigin( pBullet, vecOrigin );
	pBullet->SetAbsAngles( angAngles );
	pBullet->Spawn();
	pBullet->SetOwnerEntity( pentOwner );

	return pBullet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBullet357MM::~CBullet357MM( void )
{
	if ( m_pGlowTrail )
	{
		UTIL_Remove( m_pGlowTrail );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBullet357MM::CreateVPhysics( void )
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CBullet357MM::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBullet357MM::CreateSprites( void )
{
	// Start up the eye trail
	m_pGlowTrail = CSpriteTrail::SpriteTrailCreate( "sprites/orangelaser1.vmt", GetLocalOrigin(), false );

	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, 255, 255, 255, 220, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( 0.55f );
		m_pGlowTrail->SetEndWidth( 0.55f );
		m_pGlowTrail->SetLifeTime( 0.075f );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBullet357MM::Spawn( void )
{
	Precache( );

	SetModel( BULLET_357MODEL );
	SetMoveType( MOVETYPE_FLY );
	UTIL_SetSize( this, -Vector(1,1,1), Vector(1,1,1) );
	SetSolid( SOLID_BBOX );
	SetGravity( 0.05f );
	
	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch( &CBullet357MM::BulletTouch );

	SetThink( &CBullet357MM::BulletThink );
	SetNextThink( gpGlobals->curtime + 0.1f );
	
	CreateSprites();
}


void CBullet357MM::Precache( void )
{
	PrecacheModel( BULLET_357MODEL );

	PrecacheModel( "sprites/orangelaser1.vmt" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CBullet357MM::BulletTouch( CBaseEntity *pOther )
{
	/*
	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;
	*/

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

		if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC() )
		{
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), sk_plr_dmg_bullet_357mm.GetFloat(), DMG_BULLET );
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}
		else
		{
			CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), sk_npc_dmg_bullet_357mm.GetFloat(), DMG_BULLET );
			CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
			dmgInfo.SetDamagePosition( tr.endpos );
			pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );
		}

		ApplyMultiDamage();

		surfacedata_t *psurf = physprops->GetSurfaceData(tr.surface.surfaceProps);

		if (psurf->game.material == CHAR_TEX_METAL)
		{
			EmitSound("SolidMetal.BulletImpact");

			g_pEffects->MetalSparks(tr.endpos, tr.plane.normal);

			if (sparkshower.GetInt())
			{
				int sparkCount = random->RandomInt(0, 1);

				for (int i = 0; i < sparkCount; i++)
				{
					QAngle angles;
					VectorAngles(tr.plane.normal, angles);
					Create("spark_shower", tr.endpos, angles, NULL);
				}
			}
		}

		if (psurf->game.material == CHAR_TEX_VENT)
		{
			EmitSound("SolidMetal.BulletImpact");

			g_pEffects->MetalSparks(tr.endpos, tr.plane.normal);

			if (sparkshower.GetInt())
			{
				int sparkCount = random->RandomInt(0, 1);

				for (int i = 0; i < sparkCount; i++)
				{
					QAngle angles;
					VectorAngles(tr.plane.normal, angles);
					Create("spark_shower", tr.endpos, angles, NULL);
				}
			}
		}

		//Adrian: keep going through the glass.
		if ( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			 return;

		SetAbsVelocity( Vector( 0, 0, 0 ) );

		Vector vForward;

		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize ( vForward );

		UTIL_TraceLine( GetAbsOrigin(),	GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );
		
		SetTouch( NULL );
		SetThink( NULL );

		if ( !g_pGameRules->IsMultiplayer() )
		{
			UTIL_Remove( this );
		}
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if ( pOther->GetMoveType() == MOVETYPE_NONE && !( tr.surface.flags & SURF_SKY ) )
		{
			SetThink( &CBullet357MM::SUB_Remove );
			SetNextThink( gpGlobals->curtime + 2.0f );
				
			//FIXME: We actually want to stick (with hierarchy) to what we've hit
			SetMoveType( MOVETYPE_NONE );
			
			Vector vForward;

			AngleVectors( GetAbsAngles(), &vForward );
			VectorNormalize ( vForward );
				
			UTIL_ImpactTrace( &tr, DMG_BULLET );

			surfacedata_t *psurf = physprops->GetSurfaceData( tr.surface.surfaceProps );

			if (psurf->game.material == CHAR_TEX_METAL)
			{
				EmitSound("SolidMetal.BulletImpact");

				if (sparkshower.GetInt())
				{
					int sparkCount = random->RandomInt(0, 1);

					for (int i = 0; i < sparkCount; i++)
					{
						QAngle angles;
						VectorAngles(tr.plane.normal, angles);
						Create("spark_shower", tr.endpos, angles, NULL);
					}
				}
			}

			if (psurf->game.material == CHAR_TEX_VENT)
			{
				EmitSound("SolidMetal.BulletImpact");

				if (sparkshower.GetInt())
				{
					int sparkCount = random->RandomInt(0, 1);

					for (int i = 0; i < sparkCount; i++)
					{
						QAngle angles;
						VectorAngles(tr.plane.normal, angles);
						Create("spark_shower", tr.endpos, angles, NULL);
					}
				}
			}

			AddEffects( EF_NODRAW );
			SetTouch( NULL );
			SetThink( &CBullet357MM::SUB_Remove );
			SetNextThink( gpGlobals->curtime + 2.0f );

			if ( m_pGlowTrail != NULL )
			{
				m_pGlowTrail->TurnOn();
				m_pGlowTrail->FadeAndDie( 3.0f );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBullet357MM::BulletThink( void )
{
	QAngle angNewAngles;

	VectorAngles( GetAbsVelocity(), angNewAngles );
	SetAbsAngles( angNewAngles );

	SetNextThink( gpGlobals->curtime + 0.1f );

	// Make danger sounds out in front of me, to scare snipers back into their hole
	CSoundEnt::InsertSound( SOUND_DANGER_SNIPERONLY, GetAbsOrigin() + GetAbsVelocity() * 0.2, 120.0f, 0.5f, this, SOUNDENT_CHANNEL_REPEATED_DANGER );

	if ( GetWaterLevel()  == 0 )
		return;

	UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5 );
}
