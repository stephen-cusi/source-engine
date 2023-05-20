#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "in_buttons.h"
#include "bone_setup.h"
#include "tier0/memdbgon.h"
#include "Sprite.h"
#include "ai_basenpc.h"
class CWeaponFlame : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponFlame, CBaseHLCombatWeapon );

	DECLARE_SERVERCLASS();

	void			Spawn( void );
	void			Precache( void );
	void			ItemPostFrame( void );
	void 			UseAmmo( int count );
	void 			Fire();
	void			PrimaryAttack( void );
	void			SecondaryAttack( void );

private:	
	CHandle<CSprite>	sprite;
	float ammotype;
	float ammotime;
};

IMPLEMENT_SERVERCLASS_ST( CWeaponFlame, DT_WeaponFlame )
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_flamethrower, CWeaponFlame );
PRECACHE_WEAPON_REGISTER( weapon_flamethrower );

BEGIN_DATADESC( CWeaponFlame )
	DEFINE_FIELD( ammotime, FIELD_TIME ),	
END_DATADESC()

void CWeaponFlame::Spawn()
{
	// Call base class first
	BaseClass::Spawn();
	Precache();
	AddSolidFlags( FSOLID_NOT_SOLID );
}

void CWeaponFlame::ItemPostFrame( void )
{
	//TEMP
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer->m_afButtonPressed & IN_ATTACK )
	{
		UseAmmo( 1 );
	}
}

void CWeaponFlame::Precache( void )
{
	PrecacheModel( "sprites/flamelet1" );
	PrecacheModel( "sprites/flamelet2" );
	PrecacheModel( "sprites/flamelet3" );
	PrecacheModel( "sprites/flamelet4" );
	PrecacheModel( "sprites/flamelet5" );
	
	BaseClass::Precache();
}

void CWeaponFlame::PrimaryAttack()
{
	Fire();
}

void CWeaponFlame::SecondaryAttack()
{
}

void CWeaponFlame::UseAmmo( int count )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner == NULL )
		return;

	if ( pOwner->GetAmmoCount( ammotype ) >= count )
		pOwner->RemoveAmmo( count, ammotype );
	else
		pOwner->RemoveAmmo( pOwner->GetAmmoCount( ammotype ), ammotype );
}

void CWeaponFlame::Fire()
{
	Vector start, shootpos;
	QAngle angle;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	shootpos = pOwner->Weapon_ShootPosition();
	
	if ( pOwner )
	{
		CBaseViewModel *pVM = pOwner->GetViewModel();
		pVM->GetAttachment( pVM->LookupAttachment( "muzzle" ), start, angle );
	}

	trace_t tr;
	UTIL_TraceLine( shootpos, shootpos * MAX_TRACE_LENGTH, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

	char flame_sprite[64]; 
	int i = RandomInt( 1, 5 );
	Q_snprintf( flame_sprite, sizeof( flame_sprite ), "sprites/flamelet%d", i );
	
	sprite = CSprite::SpriteCreate( flame_sprite, tr.endpos, false );
	sprite->SetScale( 1.0 );
	sprite->SetOwnerEntity( pOwner );

	if ( tr.m_pEnt->VPhysicsGetObject() && tr.m_pEnt->IsNPC() )
	{
		dynamic_cast<CAI_BaseNPC *>( tr.m_pEnt )->Ignite( 100 );
	}
}

