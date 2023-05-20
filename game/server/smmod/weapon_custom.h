
#ifndef	WEAPONCUSTOM_H
#define	WEAPONCUSTOM_H

#include "basehlcombatweapon.h"
#include "weapon_rpg.h"

//-----------------------------------------------------------------------------
// Laser Dot
//-----------------------------------------------------------------------------
class CLaserSight : public CSprite
{
	DECLARE_CLASS(CLaserSight, CSprite);
public:

	CLaserSight(void);
	~CLaserSight(void);

	static CLaserSight *Create(const Vector &origin, CBaseEntity *pOwner = NULL, bool bVisibleDot = true);

	void	SetTargetEntity(CBaseEntity *pTarget) { m_hTargetEnt = pTarget; }
	CBaseEntity *GetTargetEntity(void) { return m_hTargetEnt; }

	void	SetLaserPosition(const Vector &origin, const Vector &normal);
	Vector	GetChasePosition();
	void	TurnOn(void);
	void	TurnOff(void);
	bool	IsOn() const { return m_bIsOn; }

	void	LaserThink(void);

	int		ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

	void	MakeInvisible(void);

protected:
	Vector				m_vecSurfaceNormal;
	EHANDLE				m_hTargetEnt;
	bool				m_bVisibleLaserSight;
	bool				m_bIsOn;

	DECLARE_DATADESC();
public:
	CLaserSight * m_pNext;
};

class CWeaponCustom : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCustom, CBaseHLCombatWeapon);

	CWeaponCustom();

	DECLARE_SERVERCLASS();

	// Important stuff
	void	Precache(void);
	void	AddViewKick(void);
	void	ShootBullets(bool isPrimary = true, bool usePrimaryAmmo = true);
	void	ShootProjectile(bool isPrimary, bool usePrimaryAmmo);
	void	ItemPostFrame(void);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void);

	// Ironsight stuff
	void	ToggleIronsights(void);
	void	EnableIronsights(void);
	void	DisableIronsights(void);

	// Shotgun stuff
	bool ShotgunStartReload(void);
	bool ShotgunReload(void);
	void ShotgunFillClip(void);
	void ShotgunFinishReload(void);
	void ItemHolsterFrame(void);
	void ShotgunHolsterFrame(void);
	void ShotgunPostFrame(void);

	// SMOD Bullet Time
	void Fire9MMBullet( void );

	// Melee stuff
	void	Swing();
	void	SwingSecondary();
	void	Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary);
	virtual float	GetMeleeRange(bool isSecondary) { return	isSecondary ? GetWpnData().m_sSecondaryMeleeRange : GetWpnData().m_sPrimaryMeleeRange; }
	virtual float	GetMeleeDamage(bool isSecondary) { return	isSecondary ? GetWpnData().m_sSecondaryMeleeDamage : GetWpnData().m_sPrimaryMeleeDamage; }
	virtual float	GetMeleeFireRate(bool isSecondary) { return	(isSecondary ? GetWpnData().m_sNextSecondaryMeleeAttack : GetWpnData().m_sNextPrimaryMeleeAttack) / 10; }

	void	UpdatePenaltyTime(void);

	const Vector&		GetBulletSpread() { return RandomVector(sin(GetWpnData().m_flMaxSpread/2), sin(GetWpnData().m_flMaxSpread / 2)); }

	virtual void Equip(CBaseCombatCharacter *pOwner);
	bool Reload(void);

	float	GetFireRate(void) { return this->GetWpnData().m_sPrimaryFireRate; }	// 13.3hz
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack2Condition(float flDot, float flDist);
	Activity	GetPrimaryAttackActivity(void);

	virtual Vector GetBulletSpreadPrimary(void)
	{
		CBaseCombatCharacter *pOwner = GetOwner();
		float speed = pOwner->GetAbsVelocity().Length2D();
		speed = RemapValClamped(speed, 0, 320, 0, 1);
		float degrees = GetWpnData().m_flDefaultSpread  * M_PI / 180;
		float degreesrun = GetWpnData().m_flRunSpread  * M_PI / 180;
		float degreescrouch = GetWpnData().m_flCrouchSpread  * M_PI / 180;
		float coneValue = sinf(degrees / 2);
		if (!FBitSet(pOwner->GetFlags(), FL_ONGROUND))
			coneValue = sinf(degrees / 2);
		else if (pOwner->GetAbsVelocity().Length2D() > GetWpnData().m_flRunSpeedSpread)
			coneValue = sinf(degreesrun / 2) * speed;
		else if (FBitSet(pOwner->GetFlags(), FL_DUCKING))
			coneValue = sinf(degreescrouch / 2);
		Vector cone = Vector(coneValue, coneValue, coneValue);
		return cone;
	}

	virtual const Vector& GetBulletSpreadSecondary(void)
	{
		static const Vector cone = this->GetWpnData().m_vSecondarySpread;
		return cone;
	}
	bool IsPrimaryBullet(void) { return this->GetWpnData().m_sPrimaryBulletEnabled; }

	bool IsSecondaryBullet(void) { return this->GetWpnData().m_sSecondaryBulletEnabled; }

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	// NPC stuff
	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	int		GetMinBurst() { return GetWpnData().iNpcBurstMin; }
	int		GetMaxBurst() { return GetWpnData().iNpcBurstMax; }

	// Laser stuff
	void	Activate(void);
	void	StartGuiding(void);
	void	StopGuiding(void);
	void	ToggleGuiding(void);
	bool	IsGuiding(void);
	void	CreateLaserPointer(void);
	void	UpdateLaserPosition(Vector vecMuzzlePos = vec3_origin, Vector vecEndPos = vec3_origin);
	//Vector	GetLaserPosition(void);

	static acttable_t m_acttableSMG[];
	static acttable_t m_acttableRIFLE[];
	static acttable_t m_acttableSHOTGUN[];
	static acttable_t m_acttablePISTOL[];
	static acttable_t m_acttableCROWBAR[];
	acttable_t *ActivityList(void);
	int ActivityListCount(void);

private:
	float	m_flSoonestPrimaryAttack;
	float	m_flLastAttackTime;
	float	m_flAccuracyPenalty;
	float	m_flNextMeleeAttack;

	bool	m_bIsLeftShoot;

	bool				m_bSighting;
	bool				m_bHideLaser;		//User to override the player's wish to guide under certain circumstances
	CHandle<CLaserSight>	m_hLaserSight;


protected: //Why did I put this in? I have no idea...
	CHandle<CMissile>	m_hMissile;
};

#define CustomWeaponAdd( num )										\
class CWeaponCustom##num : public CWeaponCustom						\
{																	\
	DECLARE_DATADESC();												\
	public:															\
	DECLARE_CLASS( CWeaponCustom##num, CWeaponCustom );				\
	CWeaponCustom##num() {};										\
	DECLARE_SERVERCLASS();											\
};																	\
IMPLEMENT_SERVERCLASS_ST(CWeaponCustom##num, DT_WeaponCustom##num)	\
END_SEND_TABLE()													\
BEGIN_DATADESC( CWeaponCustom##num )										\
END_DATADESC()														\
LINK_ENTITY_TO_CLASS( weapon_custom##num, CWeaponCustom##num );		\
PRECACHE_WEAPON_REGISTER(weapon_custom##num);

#define CustomWeaponNamedAdd( customname )										\
class CWeaponCustomNamed##customname : public CWeaponCustom						\
{																	\
	DECLARE_DATADESC();												\
	public:															\
	DECLARE_CLASS( CWeaponCustomNamed##customname, CWeaponCustom );				\
	CWeaponCustomNamed##customname() {};										\
	DECLARE_SERVERCLASS();											\
};																	\
IMPLEMENT_SERVERCLASS_ST(CWeaponCustomNamed##customname, DT_WeaponCustomNamed##customname)	\
END_SEND_TABLE()													\
BEGIN_DATADESC( CWeaponCustomNamed##customname )										\
END_DATADESC()														\
LINK_ENTITY_TO_CLASS( weapon_##customname, CWeaponCustomNamed##customname );		\
PRECACHE_WEAPON_REGISTER(weapon_##customname);
#endif	//WEAPONCUSTOM_H