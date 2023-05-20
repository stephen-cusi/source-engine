#include "cbase.h"
#include "basehlcombatweapon.h"
#include "props.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "datacache/imdlcache.h"
#include "EntityDissolve.h"
#include "ai_basenpc.h"
#include "physics_prop_ragdoll.h"
#include "saverestore_utlvector.h"
#include "tier0/memdbgon.h"

ConVar physlaunch_throwforce("physlaunch_throwforce", "2000");
ConVar physlaunch_maxprops("physlaunch_maxprops", "20");
ConVar physlaunch_lifetime("physlaunch_lifetime", "30");

class CWeaponPhysLauncher : public CBaseHLCombatWeapon
{
private:
	DECLARE_CLASS(CWeaponPhysLauncher, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();

public:
	CWeaponPhysLauncher();
	virtual ~CWeaponPhysLauncher();

private:
	static studiohdr_t *ModelHasPhys(const char *model);
	CBaseEntity *CreatePhysProp(const Vector &absOrigin, const QAngle &absAngles);

	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void Precache();
	virtual bool HasAnyAmmo() { return true; }
	virtual bool HasPrimaryAmmo() { return true; }
	virtual bool HasSecondaryAmmo() { return true; }
	virtual void ItemPostFrame();

public:
	struct physpropinfo
	{
		//DECLARE_SIMPLE_DATADESC();
		CHandle<CBaseEntity> handle;
		float curtime;
	};

private:
	CUtlVector<physpropinfo> m_hPhysProps;
	bool m_bIsRagdoll;
	string_t m_szModelName;
	CHandle<CEntityDissolve> m_pDissolver;
	int m_nModelSkin;
	int m_nModelBody;
};

acttable_t CWeaponPhysLauncher::m_acttable[] = 
{
	{ACT_RANGE_ATTACK1,		ACT_RANGE_ATTACK_RPG,	true},
	{ACT_IDLE_RELAXED,		ACT_IDLE_RPG_RELAXED,	true},
	{ACT_IDLE_STIMULATED,	ACT_IDLE_ANGRY_RPG,		true},
	{ACT_IDLE_AGITATED,		ACT_IDLE_ANGRY_RPG,		true},
	{ACT_IDLE,				ACT_IDLE_RPG,			true},
	{ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_RPG,		true},
	{ACT_WALK,				ACT_WALK_RPG,			true},
	{ACT_WALK_CROUCH,		ACT_WALK_CROUCH_RPG,	true},
	{ACT_RUN,				ACT_RUN_RPG,			true},
	{ACT_RUN_CROUCH,		ACT_RUN_CROUCH_RPG,		true},
	{ACT_COVER_LOW,			ACT_COVER_LOW_RPG,		true},
};

IMPLEMENT_ACTTABLE(CWeaponPhysLauncher);

/*
BEGIN_SIMPLE_DATADESC(CWeaponPhysLauncher::physpropinfo)
	DEFINE_FIELD(handle, FIELD_EHANDLE),
	DEFINE_FIELD(curtime, FIELD_TIME),
END_DATADESC()
*/

BEGIN_DATADESC(CWeaponPhysLauncher)
	//DEFINE_UTLVECTOR(m_hPhysProps, FIELD_EMBEDDED),
	DEFINE_FIELD(m_bIsRagdoll, FIELD_BOOLEAN),
	DEFINE_FIELD(m_szModelName, FIELD_MODELNAME),
	//DEFINE_FIELD(m_pDissolver, FIELD_EHANDLE),
	DEFINE_FIELD(m_nModelSkin, FIELD_INTEGER),
	DEFINE_FIELD(m_nModelBody, FIELD_INTEGER),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponPhysLauncher, DT_WeaponPhysLauncher)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_physlauncher, CWeaponPhysLauncher);
PRECACHE_WEAPON_REGISTER(weapon_physlauncher);

CWeaponPhysLauncher::CWeaponPhysLauncher()
	: BaseClass()
{
	m_bIsRagdoll = false;
	m_szModelName = MAKE_STRING("models/props_junk/watermelon01.mdl");
	m_pDissolver = NULL;
	m_nModelSkin = 0;
	m_nModelBody = 0;
}

CWeaponPhysLauncher::~CWeaponPhysLauncher()
{
	FOR_EACH_VEC(m_hPhysProps, i) {
		const physpropinfo &info = m_hPhysProps.Element(i);
		UTIL_Remove(info.handle);
	}

	m_hPhysProps.RemoveAll();

	UTIL_Remove(m_pDissolver);
}

void CWeaponPhysLauncher::Precache()
{
	BaseClass::Precache();

	PrecacheModel(STRING(m_szModelName));
}

void CWeaponPhysLauncher::PrimaryAttack()
{
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	Vector vForward, vRight, vUp;
	pOwner->EyeVectors(&vForward, &vRight, &vUp);
	Vector muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;
	QAngle vecAngles;
	VectorAngles(vForward, vecAngles);

	CBaseEntity *pProp = CreatePhysProp(muzzlePoint + vForward * 32.0f, vecAngles);
	if(!pProp) {
		WeaponSound(EMPTY);
		return;
	}

	pProp->SetOwnerEntity(this);

	IPhysicsObject *pPhys = pProp ? pProp->VPhysicsGetObject() : NULL;
	if(pPhys) {
		physpropinfo info;
		info.handle = pProp;
		info.curtime = gpGlobals->curtime;

		m_hPhysProps.AddToHead(info);

		int count = m_hPhysProps.Count()-1;
		if(count >= physlaunch_maxprops.GetInt()) {
			const physpropinfo &info = m_hPhysProps.Element(count);
			UTIL_Remove(info.handle);
			m_hPhysProps.Remove(count);
		}

		m_iPrimaryAttacks++;

		SendWeaponAnim(ACT_VM_PRIMARYATTACK);
		WeaponSound(SINGLE);

		float force = pPhys->GetMass() * physlaunch_throwforce.GetFloat();
		pPhys->ApplyForceCenter(vForward * force);

		pOwner->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
		pOwner->RumbleEffect(RUMBLE_SHOTGUN_SINGLE, 0, RUMBLE_FLAG_RESTART);

		gamestats->Event_WeaponFired(pOwner, true, GetClassname());
		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, pOwner, SOUNDENT_CHANNEL_WEAPON);
	} else {
		WeaponSound(EMPTY);
		UTIL_Remove(pProp);
	}
}

void CWeaponPhysLauncher::SecondaryAttack()
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	Vector vForward, vRight, vUp;
	pOwner->EyeVectors(&vForward, &vRight, &vUp);
	Vector muzzlePoint = pOwner->Weapon_ShootPosition() + vForward * 12.0f + vRight * 6.0f + vUp * -3.0f;

	trace_t tr;
	UTIL_TraceLine(muzzlePoint, muzzlePoint + vForward * MAX_TRACE_LENGTH, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	if(tr.fraction == 1.0 || tr.allsolid || !tr.m_pEnt) {
		WeaponSound(SPECIAL1);
		return;
	}

	studiohdr_t *pStudioHdr = CWeaponPhysLauncher::ModelHasPhys(STRING(tr.m_pEnt->GetModelName()));
	if(!tr.m_pEnt->GetModel() || !pStudioHdr) {
		WeaponSound(SPECIAL1);
		return;
	}

	CBaseAnimating *pAnim = tr.m_pEnt->GetBaseAnimating();
	if(pAnim) {
		m_nModelSkin = pAnim->m_nSkin;
		m_nModelBody = pAnim->m_nBody;
	}

	if(tr.m_pEnt->IsNPC() || dynamic_cast<CRagdollProp *>(tr.m_pEnt))
		m_bIsRagdoll = true;
	else
		m_bIsRagdoll = false;

	m_szModelName = tr.m_pEnt->GetModelName();

	UTIL_Remove(m_pDissolver);
	m_pDissolver = CEntityDissolve::Create(tr.m_pEnt, STRING(m_szModelName), gpGlobals->curtime, ENTITY_DISSOLVE_ELECTRICAL_LIGHT, NULL);

	SendWeaponAnim(ACT_VM_RELOAD);
	WeaponSound(SPECIAL2);
}

CBaseEntity *CWeaponPhysLauncher::CreatePhysProp(const Vector &absOrigin, const QAngle &absAngles)
{
	studiohdr_t *pStudioHdr = CWeaponPhysLauncher::ModelHasPhys(STRING(m_szModelName));
	if(!pStudioHdr)
		return NULL;

	trace_t tr;
	UTIL_TraceHull(absOrigin, absOrigin, pStudioHdr->hull_min, pStudioHdr->hull_max, MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
	if(tr.fraction != 1.0 || tr.allsolid)
		return NULL;

	VectorMA(tr.endpos, 1.0f, tr.plane.normal, tr.endpos);

	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache(true);
	CBaseEntity *pProp = NULL;
	if(m_bIsRagdoll)
		pProp = CreateEntityByName("prop_ragdoll");
	else
		pProp = CreateEntityByName("prop_physics");
	if(pProp)
	{
		char buf[512];
		V_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", tr.endpos.x, tr.endpos.y, tr.endpos.z);
		pProp->KeyValue("origin", buf);
		V_snprintf(buf, sizeof(buf), "%.10f %.10f %.10f", absAngles.x, absAngles.y, absAngles.z);
		pProp->KeyValue("angles", buf);
		pProp->KeyValue("model", STRING(m_szModelName));
		pProp->KeyValue("fademindist", "-1");
		pProp->KeyValue("fademaxdist", "0");
		pProp->KeyValue("fadescale", "1");
		pProp->KeyValue("inertiaScale", "1.0");
		pProp->KeyValue("physdamagescale", "0.1");
		pProp->Precache();
		DispatchSpawn(pProp);
		pProp->Activate();
		CBaseAnimating *pAnim = pProp->GetBaseAnimating();
		if(pAnim) {
			pAnim->m_nSkin = m_nModelSkin;
			pAnim->m_nBody = m_nModelBody;
		}
	}
	CBaseEntity::SetAllowPrecache(bAllowPrecache);

	return pProp;
}

void CWeaponPhysLauncher::ItemPostFrame()
{
	BaseClass::ItemPostFrame();

	FOR_EACH_VEC(m_hPhysProps, i) {
		const physpropinfo &info = m_hPhysProps.Element(i);
		if(!info.handle.Get())
			m_hPhysProps.Remove(i);

		if((gpGlobals->curtime-info.curtime) >= physlaunch_lifetime.GetFloat()) {
			UTIL_Remove(info.handle);
			m_hPhysProps.Remove(i);
		}
	}
}

studiohdr_t *CWeaponPhysLauncher::ModelHasPhys(const char *model)
{
	MDLCACHE_CRITICAL_SECTION();

	MDLHandle_t h = mdlcache->FindMDL(model);
	if(h == MDLHANDLE_INVALID)
// Rara is 1984		
		return NULL;

	studiohdr_t *pStudioHdr = mdlcache->GetStudioHdr(h);
	if(!pStudioHdr)
		return NULL;

	if(!mdlcache->GetVCollide(h))
		return NULL;

	return pStudioHdr;
}