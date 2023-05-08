//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Pistol - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "hl2_player.h"
#include "bullet_9mm.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	PISTOL_FASTEST_REFIRE_TIME		0.1f
#define	PISTOL_FASTEST_DRY_REFIRE_TIME	0.2f

#define	PISTOL_ACCURACY_SHOT_PENALTY_TIME		0.2f	// Applied amount of time each shot adds to the time we must recover from
#define	PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME	1.5f	// Maximum penalty to deal out

ConVar	pistol_use_new_accuracy( "pistol_use_new_accuracy", "1" );

ConVar  bullettimesim( "bullettimesim", "0", FCVAR_ARCHIVE );

extern ConVar disable_bullettime;

//-----------------------------------------------------------------------------
// CWeaponPistol
//-----------------------------------------------------------------------------

class CWeaponPistol : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS( CWeaponPistol, CBaseHLCombatWeapon );

	CWeaponPistol(void);

	DECLARE_SERVERCLASS();

	void	Precache( void );
	void	ItemPostFrame( void );
	void	ItemPreFrame( void );
	void	ItemBusyFrame( void );
	void	PrimaryAttack( void );
	void	AddViewKick( void );
	void	DryFire( void );
	void    Fire9MMBullet( void );
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	void	UpdatePenaltyTime( void );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	Activity	GetPrimaryAttackActivity( void );

	virtual bool Reload( void );

	virtual const Vector& GetBulletSpread( void )
	{		
		// Handle NPCs first
		static Vector npcCone = VECTOR_CONE_5DEGREES;
		if ( GetOwner() && GetOwner()->IsNPC() )
			return npcCone;
			
		static Vector cone;

		if ( pistol_use_new_accuracy.GetBool() )
		{
			float ramp = RemapValClamped(	m_flAccuracyPenalty, 
											0.0f, 
											PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME, 
											0.0f, 
											1.0f ); 

			// We lerp from very accurate to inaccurate over time
			VectorLerp( VECTOR_CONE_1DEGREES, VECTOR_CONE_6DEGREES, ramp, cone );
		}
		else
		{
			// Old value
			cone = VECTOR_CONE_4DEGREES;
		}

		return cone;
	}
	
	virtual int	GetMinBurst() 
	{ 
		return 1; 
	}

	virtual int	GetMaxBurst() 
	{ 
		return 3; 
	}

	virtual float GetFireRate( void ) 
	{
		return 0.5f; 
	}

	DECLARE_ACTTABLE();

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	int		m_nNumShotsFired;
};


IMPLEMENT_SERVERCLASS_ST(CWeaponPistol, DT_WeaponPistol)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_pistol, CWeaponPistol );
PRECACHE_WEAPON_REGISTER( weapon_pistol );

BEGIN_DATADESC( CWeaponPistol )

	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),
	DEFINE_FIELD( m_flLastAttackTime,		FIELD_TIME ),
	DEFINE_FIELD( m_flAccuracyPenalty,		FIELD_FLOAT ), //NOTENOTE: This is NOT tracking game time
	DEFINE_FIELD( m_nNumShotsFired,			FIELD_INTEGER ),

END_DATADESC()

acttable_t	CWeaponPistol::m_acttable[] = 
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
	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_PISTOL, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_PISTOL, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_PISTOL, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_PISTOL, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_HL2MP_GESTURE_RELOAD_PISTOL, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_PISTOL, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_PISTOL, false },
};


IMPLEMENT_ACTTABLE( CWeaponPistol );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponPistol::CWeaponPistol( void )
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
	m_flAccuracyPenalty = 0.0f;

	m_fMinRange1		= 24;
	m_fMaxRange1		= 1500;
	m_fMinRange2		= 24;
	m_fMaxRange2		= 200;

	m_bFiresUnderwater	= true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::Precache( void )
{
	BaseClass::Precache();

	UTIL_PrecacheOther( "bullet_9mm" );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponPistol::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CHL2_Player *pPlayer = dynamic_cast < CHL2_Player* >( UTIL_PlayerByIndex( 1 ) );

	switch( pEvent->event )
	{
		case EVENT_WEAPON_PISTOL_FIRE:
		{
			if ( !pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0 )
			{
				 Vector vecShootOrigin, vecShootDir;
				 vecShootOrigin = pOperator->Weapon_ShootPosition();

				 CAI_BaseNPC *npc = pOperator->MyNPCPointer();
				 ASSERT(npc != NULL);

				 vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

				 CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

				 WeaponSound(SINGLE_NPC);
				 pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
				 pOperator->DoMuzzleFlash();
				 m_iClip1 = m_iClip1 - 1;
			}
			else if ( pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0 )
			{
				Fire9MMBullet();
			}

			if ( bullettimesim.GetInt() == 1 && disable_bullettime.GetInt() == 1 )
			{
				Fire9MMBullet();
			}
		}
		break;
		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponPistol::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flSoonestPrimaryAttack	= gpGlobals->curtime + PISTOL_FASTEST_DRY_REFIRE_TIME;
	m_flNextPrimaryAttack		= gpGlobals->curtime + SequenceDuration();
}

void CWeaponPistol::Fire9MMBullet( void )
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
	
	pBullet->SetAbsVelocity( vForward * 475 );
	pBullet->SetOwnerEntity( this );
			
	CSoundEnt::InsertSound( SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, this, SOUNDENT_CHANNEL_WEAPON, GetEnemy() );

	WeaponSound( SINGLE_NPC );

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponPistol::PrimaryAttack( void )
{
	CHL2_Player *pPlayer = dynamic_cast < CHL2_Player* >( UTIL_PlayerByIndex( 1 ) );

	if ( !pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0 )
	{
		if ((gpGlobals->curtime - m_flLastAttackTime) > 0.5f)
		{
			m_nNumShotsFired = 0;
		}
		else
		{
			m_nNumShotsFired++;
		}

		m_flLastAttackTime = gpGlobals->curtime;
		m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;
		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, GetOwner());

		CBasePlayer *pOwner = ToBasePlayer(GetOwner());

		if (pOwner)
		{
			// Each time the player fires the pistol, reset the view punch. This prevents
			// the aim from 'drifting off' when the player fires very quickly. This may
			// not be the ideal way to achieve this, but it's cheap and it works, which is
			// great for a feature we're evaluating. (sjb)
			pOwner->ViewPunchReset();
		}

		BaseClass::PrimaryAttack();

		// Add an accuracy penalty which can move past our maximum penalty time if we're really spastic
		m_flAccuracyPenalty += PISTOL_ACCURACY_SHOT_PENALTY_TIME;

		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired(pOwner, true, GetClassname());
	}
	else if ( pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0 )
	{
		// If my clip is empty (and I use clips) start reload
		if (UsesClipsForAmmo1() && !m_iClip1)
		{
			Reload();
			return;
		}

		// Only the player fires this way so we can cast
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		if (!pPlayer)
		{
			return;
		}

		pPlayer->DoMuzzleFlash();

		m_flLastAttackTime = gpGlobals->curtime;
		m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;

		SendWeaponAnim(GetPrimaryAttackActivity());

		// player "shoot" animation
		pPlayer->SetAnimation(PLAYER_ATTACK1);

		FireBulletsInfo_t info;
		info.m_vecSrc = pPlayer->Weapon_ShootPosition();

		info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

		// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
		// especially if the weapon we're firing has a really fast rate of fire.
		info.m_iShots = 0;
		float fireRate = GetFireRate();

		while (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// MUST call sound before removing a round from the clip of a CMachineGun
			WeaponSound(SINGLE, m_flNextPrimaryAttack);
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
			info.m_iShots++;
			if (!fireRate)
				break;
		}

		// Make sure we don't fire more than the amount in the clip
		if (UsesClipsForAmmo1())
		{
			info.m_iShots = MIN(info.m_iShots, m_iClip1);
			m_iClip1 -= info.m_iShots;
		}
		else
		{
			info.m_iShots = MIN(info.m_iShots, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
			pPlayer->RemoveAmmo(info.m_iShots, m_iPrimaryAmmoType);
		}

		info.m_flDistance = MAX_TRACE_LENGTH;
		info.m_iAmmoType = m_iPrimaryAmmoType;
		info.m_iTracerFreq = 2;

		Vector vecAiming;
		vecAiming = pPlayer->GetAutoaimVector(0);
		Vector vecSrc = pPlayer->Weapon_ShootPosition();

		QAngle angAiming;
		VectorAngles(vecAiming, angAiming);

		CBullet9MM *pBullet = CBullet9MM::BulletCreate(vecSrc, angAiming, pPlayer);

		if (pPlayer->GetWaterLevel() == 3)
		{
			pBullet->SetAbsVelocity(vecAiming * 1100);
		}
		else
		{
			pBullet->SetAbsVelocity(vecAiming * 1400);
		}

		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;

		/*
		// Fire the bullets
		info.m_vecSpread = pPlayer->GetAttackSpread(this);

		pPlayer->FireBullets(info);
		*/

		if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		{
			// HEV suit - indicate out of ammo condition
			pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
		}

		//Add our view kick in
		AddViewKick();
	}

	if ( bullettimesim.GetInt() == 1 && disable_bullettime.GetInt() == 1 )
	{
		// If my clip is empty (and I use clips) start reload
		if (UsesClipsForAmmo1() && !m_iClip1)
		{
			Reload();
			return;
		}

		// Only the player fires this way so we can cast
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

		if (!pPlayer)
		{
			return;
		}

		pPlayer->DoMuzzleFlash();

		m_flLastAttackTime = gpGlobals->curtime;
		m_flSoonestPrimaryAttack = gpGlobals->curtime + PISTOL_FASTEST_REFIRE_TIME;

		SendWeaponAnim(GetPrimaryAttackActivity());

		// player "shoot" animation
		pPlayer->SetAnimation(PLAYER_ATTACK1);

		FireBulletsInfo_t info;
		info.m_vecSrc = pPlayer->Weapon_ShootPosition();

		info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

		// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
		// especially if the weapon we're firing has a really fast rate of fire.
		info.m_iShots = 0;
		float fireRate = GetFireRate();

		while (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// MUST call sound before removing a round from the clip of a CMachineGun
			WeaponSound(SINGLE, m_flNextPrimaryAttack);
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
			info.m_iShots++;
			if (!fireRate)
				break;
		}

		// Make sure we don't fire more than the amount in the clip
		if (UsesClipsForAmmo1())
		{
			info.m_iShots = MIN(info.m_iShots, m_iClip1);
			m_iClip1 -= info.m_iShots;
		}
		else
		{
			info.m_iShots = MIN(info.m_iShots, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
			pPlayer->RemoveAmmo(info.m_iShots, m_iPrimaryAmmoType);
		}

		info.m_flDistance = MAX_TRACE_LENGTH;
		info.m_iAmmoType = m_iPrimaryAmmoType;
		info.m_iTracerFreq = 2;

		Vector vecAiming;
		vecAiming = pPlayer->GetAutoaimVector(0);
		Vector vecSrc = pPlayer->Weapon_ShootPosition();

		QAngle angAiming;
		VectorAngles(vecAiming, angAiming);

		CBullet9MM *pBullet = CBullet9MM::BulletCreate(vecSrc, angAiming, pPlayer);

		if (pPlayer->GetWaterLevel() == 3)
		{
			pBullet->SetAbsVelocity(vecAiming * 1100);
		}
		else
		{
			pBullet->SetAbsVelocity(vecAiming * 1400);
		}

		m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;

		/*
		// Fire the bullets
		info.m_vecSpread = pPlayer->GetAttackSpread(this);

		pPlayer->FireBullets(info);
		*/

		if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		{
			// HEV suit - indicate out of ammo condition
			pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
		}

		//Add our view kick in
		AddViewKick();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::UpdatePenaltyTime( void )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	// Check our penalty time decay
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flAccuracyPenalty -= gpGlobals->frametime;
		m_flAccuracyPenalty = clamp( m_flAccuracyPenalty, 0.0f, PISTOL_ACCURACY_MAXIMUM_PENALTY_TIME );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::ItemPreFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemPreFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::ItemBusyFrame( void )
{
	UpdatePenaltyTime();

	BaseClass::ItemBusyFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Allows firing as fast as button is pressed
//-----------------------------------------------------------------------------
void CWeaponPistol::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	if ( m_bInReload )
		return;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	//Allow a refire as fast as the player can click
	if ( ( ( pOwner->m_nButtons & IN_ATTACK ) == false ) && ( m_flSoonestPrimaryAttack < gpGlobals->curtime ) )
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}
	else if ( ( pOwner->m_nButtons & IN_ATTACK ) && ( m_flNextPrimaryAttack < gpGlobals->curtime ) && ( m_iClip1 <= 0 ) )
	{
		DryFire();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
Activity CWeaponPistol::GetPrimaryAttackActivity( void )
{
	if ( m_nNumShotsFired < 1 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nNumShotsFired < 2 )
		return ACT_VM_RECOIL1;

	if ( m_nNumShotsFired < 3 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponPistol::Reload( void )
{
	bool fRet = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );
	if ( fRet )
	{
		WeaponSound( RELOAD );
		m_flAccuracyPenalty = 0.0f;
	}
	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPistol::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle	viewPunch;

	viewPunch.x = random->RandomFloat( 0.25f, 0.5f );
	viewPunch.y = random->RandomFloat( -.6f, .6f );
	viewPunch.z = 0.0f;

	//Add it to the view punch
	pPlayer->ViewPunch( viewPunch );
}
