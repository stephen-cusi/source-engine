#include "cbase.h"
#include "in_buttons.h"
#include "basegrenade_shared.h"
#include "KeyValues.h"
#include "filesystem.h"

#ifdef CLIENT_DLL
#include "particles/particles.h"
#include "c_basehlcombatweapon.h"
#else
#include "particle_system.h"
#include "basehlcombatweapon.h"
#endif

#ifndef CLIENT_DLL
#include "props.h"
#include "gamestats.h"
#include "beam_shared.h"
#include "ammodef.h"
#include "baseanimating.h"
#include "EntityFlame.h"
#include "explode.h"
#include "entitylist.h"
#include "player.h"
#include "basegrenade_shared.h"
#endif

#include "tier0/memdbgon.h"

ConVar toolmode("toolmode", "0");

//for setcolor
ConVar red("red", "0");
ConVar green("green", "0");
ConVar blue("blue", "0");

//for modelscale
ConVar duration("duration", "0");

//grenade
ConVar exp_magnitude("exp_magnitude", "0");
ConVar exp_radius("exp_radius", "0");

//CHAR
ConVar tool_create("tool_create", "");
ConVar tool_emmiter_effect("tool_emmiter_effect", "env_fire_large");

#define BEAM_SPRITE "sprites/bluelaser1.vmt"

#ifdef CLIENT_DLL
#define CWeaponToolGun C_WeaponToolGun
#endif

class CWeaponToolGun : public CBaseHLCombatWeapon
{
private:
	DECLARE_CLASS(CWeaponToolGun, CBaseHLCombatWeapon);;
	DECLARE_DATADESC();
	
#ifndef CLIENT_DLL
	DECLARE_ACTTABLE();
#endif

public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponToolGun();
	virtual ~CWeaponToolGun();

private:	
	virtual void PrimaryAttack();
#ifndef CLIENT_DLL
	virtual void SecondaryAttack();
#endif
	virtual void Precache();
	virtual bool HasAnyAmmo() { return true; }
	virtual bool HasPrimaryAmmo() { return true; }
	virtual bool HasSecondaryAmmo() { return true; }
	virtual void ItemPostFrame();
	virtual void InitKeyValues();
#ifndef CLIENT_DLL
	virtual void DrawBeam( const Vector &startPos, const Vector &endPos, float width );
	virtual void DoImpactEffect( trace_t &tr, int nDamageType );
#endif

private:
#ifndef CLIENT_DLL
	CHandle<CEntityFlame> m_pIgniter;
#endif	
	CNetworkVar( int, m_iMode );
};


#ifndef CLIENT_DLL
acttable_t CWeaponToolGun::m_acttable[] = 
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

IMPLEMENT_ACTTABLE( CWeaponToolGun );
#endif


IMPLEMENT_NETWORKCLASS_ALIASED( WeaponToolGun, DT_WeaponToolGun )

BEGIN_NETWORK_TABLE( CWeaponToolGun, DT_WeaponToolGun )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO(m_iMode) ),
#else
	SendPropInt( SENDINFO(m_iMode), 10, SPROP_UNSIGNED ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CWeaponToolGun )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( weapon_toolgun, CWeaponToolGun );
PRECACHE_WEAPON_REGISTER( weapon_toolgun );

BEGIN_DATADESC(CWeaponToolGun)
	DEFINE_FIELD(m_iMode, FIELD_INTEGER),
END_DATADESC()

CWeaponToolGun::CWeaponToolGun() : BaseClass()
{
#ifndef CLIENT_DLL
	m_pIgniter = NULL;
#endif
	InitKeyValues();
}

CWeaponToolGun::~CWeaponToolGun()
{
}

void CWeaponToolGun::InitKeyValues()
{
}

void CWeaponToolGun::Precache()
{
	BaseClass::Precache();
}

void CWeaponToolGun::PrimaryAttack()
{
#ifndef CLIENT_DLL
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	QAngle vecAngles( 0, GetAbsAngles().y - 90, 0 );

	Vector vForward, vRight, vUp;
	pOwner->EyeVectors(&vForward, &vRight, &vUp);
	Vector muzzlePoint = pOwner->Weapon_ShootPosition() + vForward + vRight + vUp;

	trace_t tr;
	UTIL_TraceLine(muzzlePoint, muzzlePoint + vForward * MAX_TRACE_LENGTH, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOwner->Weapon_ShootPosition();
	DrawBeam( vecShootOrigin, tr.endpos, 4 );

	/* 0 - remover
	   1 - setcolor
	   2 - modelscale
	   3 - igniter
	   4 - dynamite
	   5 - spawner
	   6 - emitter
	*/
	m_iMode = toolmode.GetInt();

	switch( m_iMode )
	{
		case 0: 
			if (tr.m_pEnt->IsNPC())
			{
				UTIL_Remove(tr.m_pEnt);
			}
			else if(tr.m_pEnt->VPhysicsGetObject())
			{
				UTIL_Remove(tr.m_pEnt);
			}
			break; 

		case 1:
			if(tr.m_pEnt->IsNPC() || tr.m_pEnt->VPhysicsGetObject())
			{
				int r = red.GetInt();
				int g = green.GetInt();
				int b = blue.GetInt();
				tr.m_pEnt->SetRenderColor( r, g, b, 255);
			}
			break;

		case 2:
			if(tr.m_pEnt->IsNPC() || tr.m_pEnt->VPhysicsGetObject())
			{
				float a = duration.GetFloat();
				dynamic_cast<CBaseAnimating *>(tr.m_pEnt)->SetModelScale(a, 0.0f);
			}
			break;

		case 3:
			if(tr.m_pEnt->IsNPC() || tr.m_pEnt->VPhysicsGetObject())
			{
				UTIL_Remove(m_pIgniter);
				m_pIgniter = CEntityFlame::Create(tr.m_pEnt, true);
			}
			break;
		case 4:
			{
				CBaseEntity *pEntity = CreateEntityByName( "npc_grenade_frag" );
				if( pEntity )
				{	
					pEntity->SetAbsOrigin( tr.endpos );
					pEntity->SetAbsAngles( vecAngles );
					DispatchSpawn( pEntity );
					pEntity->Activate();
				}	
			}
			break;
		case 5:
			{
				CBaseEntity *pEntity = CreateEntityByName( tool_create.GetString() );
				if( pEntity )
				{
					pEntity->SetAbsOrigin( tr.endpos );
					pEntity->SetAbsAngles( vecAngles );
					DispatchSpawn( pEntity );
					pEntity->Activate();
				}	
			}
			break;
		case 6:
			{
				CBaseEntity *pProp = CreateEntityByName("prop_ragdoll");
				pProp->KeyValue("model", "models/props_lab/tpplug.mdl");
				pProp->SetAbsOrigin( tr.endpos );
				pProp->SetAbsAngles( vecAngles );
				DispatchSpawn( pProp );
				pProp->Activate();

				CParticleSystem *pParticle = (CParticleSystem *) CreateEntityByName( "info_particle_system" );
				pParticle->KeyValue( "start_active", "1" );
				pParticle->KeyValue( "effect_name", tool_emmiter_effect.GetString() );
				pParticle->SetParent( pProp );
				pParticle->SetLocalOrigin( vec3_origin );
				DispatchSpawn( pParticle );
				pParticle->Activate();
			}
		}
#endif
}

#ifndef CLIENT_DLL
void CWeaponToolGun::SecondaryAttack()
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	Vector vForward, vRight, vUp;
	pOwner->EyeVectors(&vForward, &vRight, &vUp);
	Vector muzzlePoint = pOwner->Weapon_ShootPosition() + vForward + vRight + vUp;

	trace_t tr;
	UTIL_TraceLine(muzzlePoint, muzzlePoint + vForward * MAX_TRACE_LENGTH, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOwner->Weapon_ShootPosition();
	DrawBeam( vecShootOrigin, tr.endpos, 4 );

	m_iMode = toolmode.GetInt();

	switch(m_iMode)
	{
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	case 4:
		{
			if ( tr.m_pEnt->VPhysicsGetObject() )
			{
				char *entityName = tr.m_pEnt->GetClassname();
				if (entityName == NULL )
				{
					Msg("NULL Name!\n");
				}
				else
				{	
					if ( FStrEq( entityName, "npc_grenade_frag" ) )
					{
						dynamic_cast<CBaseGrenade *>(tr.m_pEnt)->Detonate();
					}				
				}
			}
		}
	}
}
#endif

void CWeaponToolGun::ItemPostFrame()
{
	BaseClass::ItemPostFrame();
}

#ifndef CLIENT_DLL
void CWeaponToolGun::DrawBeam( const Vector &startPos, const Vector &endPos, float width )
{
	UTIL_Tracer( startPos, endPos, 0, TRACER_DONT_USE_ATTACHMENT, 6500, false, "GaussTracer" );
	CBeam *pBeam = CBeam::BeamCreate( BEAM_SPRITE, 4 );
	pBeam->SetStartPos( startPos );
	pBeam->PointEntInit( endPos, this );
	pBeam->SetEndAttachment( LookupAttachment("Muzzle") );
	pBeam->SetWidth( width );
	pBeam->SetBrightness( 255 );
	pBeam->SetColor( 66, 170, 255 );
	pBeam->RelinkBeam();
	pBeam->LiveForTime( 0.1f );
}

void CWeaponToolGun::DoImpactEffect( trace_t &tr, int nDamageType )
{
	DrawBeam( tr.startpos, tr.endpos, 4 );
}

#endif