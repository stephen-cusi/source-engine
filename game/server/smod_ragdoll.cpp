#include "cbase.h"
#include "smod_ragdoll.h"
#include "bone_setup.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "soundent.h"
#include "coolmod/smod_cvars.h"
#include "decals.h"
#include "detail_entities.h"
#include "gib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar ragdoll_twitch("ragdoll_twitch", "0", FCVAR_ARCHIVE);
ConVar ragdoll_zvector("ragdoll_zvector", "0", FCVAR_ARCHIVE);
ConVar gore_bleeding("gore_bleeding", "1", FCVAR_ARCHIVE);
ConVar gore_togib("gore_togib", "0", FCVAR_ARCHIVE);
ConVar gore_gibhealth("gore_gibhealth", "50", FCVAR_ARCHIVE);

extern ConVar ragdoll_forceserverragdoll;

//-----------------------------------------------------------------------------
// Spawnflags
//-----------------------------------------------------------------------------
#define	SF_RAGDOLLPROP_DEBRIS		0x0004
#define SF_RAGDOLLPROP_USE_LRU_RETIREMENT	0x1000
#define	SF_RAGDOLLPROP_ALLOW_DISSOLVE		0x2000	// Allow this prop to be dissolved
#define	SF_RAGDOLLPROP_MOTIONDISABLED		0x4000
#define	SF_RAGDOLLPROP_ALLOW_STRETCH		0x8000
#define	SF_RAGDOLLPROP_STARTASLEEP			0x10000

LINK_ENTITY_TO_CLASS(prop_smod_ragdoll, CSMODRagdoll);

CSMODRagdoll::CSMODRagdoll()
{
	m_bloodColor = BLOOD_COLOR_RED;
	m_flGibHealth = gore_gibhealth.GetFloat();
}

CSMODRagdoll::~CSMODRagdoll()
{
}

void CSMODRagdoll::Precache()
{
}

void CSMODRagdoll::Spawn()
{
	Precache();
}

void CSMODRagdoll::TraceAttack(const CTakeDamageInfo &info, const Vector &dir, trace_t *ptr, CDmgAccumulator *pAccumulator)
{
	UTIL_BloodImpact(info.GetDamagePosition(), ptr->plane.normal, m_bloodColor, 20);
	ptr->m_pEnt->TraceBleed(info.GetDamage(), dir, ptr, info.GetDamageType());

	if ( gore_togib.GetBool() )
	{
		m_flGibHealth -= 4;

		if (m_bloodColor == BLOOD_COLOR_RED)
		{
			if (m_flGibHealth <= 0)
			{
				CGib::SpawnHeadGib(this);
				CGib::SpawnRandomGibs(this, 7, GIB_HUMAN);

				UTIL_BloodSpray(WorldSpaceCenter(), info.GetDamageForce(), BLOOD_COLOR_RED, 13, FX_BLOODSPRAY_ALL);

				EmitSound("Player.Splat");

				RemoveDeferred();
			}
		}

		if (m_bloodColor == BLOOD_COLOR_GREEN)
		{
			if (m_flGibHealth <= 0)
			{
				CGib::SpawnRandomGibs(this, 4, GIB_ALIEN);	// Throw alien gibs

				UTIL_BloodSpray(WorldSpaceCenter(), info.GetDamageForce(), BLOOD_COLOR_GREEN, 13, FX_BLOODSPRAY_ALL);

				EmitSound("Player.Splat");

				RemoveDeferred();
			}
		}

		if (m_bloodColor == BLOOD_COLOR_YELLOW)
		{
			if (m_flGibHealth <= 0)
			{
				CGib::SpawnRandomGibs(this, 4, GIB_ALIEN);

				UTIL_BloodSpray(WorldSpaceCenter(), info.GetDamageForce(), BLOOD_COLOR_YELLOW, 13, FX_BLOODSPRAY_ALL);

				EmitSound("Player.Splat");

				RemoveDeferred();
			}
		}

		if (info.GetDamageType() & DMG_BLAST)
		{
			if (m_bloodColor == BLOOD_COLOR_RED)
			{
				CGib::SpawnHeadGib(this);
				CGib::SpawnRandomGibs(this, 7, GIB_HUMAN);

				UTIL_BloodSpray(WorldSpaceCenter(), info.GetDamageForce(), BLOOD_COLOR_RED, 13, FX_BLOODSPRAY_ALL);

				EmitSound("Player.Splat");

				RemoveDeferred();
			}
		}

		if (info.GetDamageType() & DMG_BLAST)
		{
			if (m_bloodColor == BLOOD_COLOR_GREEN)
			{
				CGib::SpawnRandomGibs(this, 4, GIB_ALIEN);

				UTIL_BloodSpray(WorldSpaceCenter(), info.GetDamageForce(), BLOOD_COLOR_GREEN, 13, FX_BLOODSPRAY_ALL);

				EmitSound("Player.Splat");

				RemoveDeferred();
			}
		}

		if (info.GetDamageType() & DMG_BLAST)
		{
			if (m_bloodColor == BLOOD_COLOR_YELLOW)
			{
				CGib::SpawnRandomGibs(this, 4, GIB_ALIEN);

				UTIL_BloodSpray(WorldSpaceCenter(), info.GetDamageForce(), BLOOD_COLOR_YELLOW, 13, FX_BLOODSPRAY_ALL);

				EmitSound("Player.Splat");

				RemoveDeferred();
			}
		}
	}

	if (ragdoll_twitch.GetBool())
	{
		for (int i = 0; i < m_ragdoll.listCount; i++)
		{
			m_ragdoll.list[i].pObject->ApplyTorqueCenter(RandomVector(-200, 200));
		}
	}

	BaseClass::TraceAttack(info, dir, ptr, pAccumulator);
}

BEGIN_DATADESC(CSMODRagdoll)
END_DATADESC()

static void SyncAnimatingWithPhysics(CBaseAnimating *pAnimating)
{
	IPhysicsObject *pPhysics = pAnimating->VPhysicsGetObject();
	if (pPhysics)
	{
		Vector pos;
		pPhysics->GetShadowPosition(&pos, NULL);
		pAnimating->SetAbsOrigin(pos);
	}
}

CBaseEntity *CreateSMODServerRagdoll(CBaseAnimating *pAnimating, int forceBone, const CTakeDamageInfo &info, int collisionGroup, bool bUseLRURetirement)
{
	if (info.GetDamageType() & (DMG_VEHICLE | DMG_CRUSH))
	{
		// if the entity was killed by physics or a vehicle, move to the vphysics shadow position before creating the ragdoll.
		SyncAnimatingWithPhysics(pAnimating);
	}
	CSMODRagdoll *pRagdoll = (CSMODRagdoll *)CBaseEntity::CreateNoSpawn("prop_smod_ragdoll", pAnimating->GetAbsOrigin(), vec3_angle, NULL);
	pRagdoll->CopyAnimationDataFrom(pAnimating);
	pRagdoll->SetOwnerEntity(pAnimating);

	pRagdoll->InitRagdollAnimation();
	matrix3x4_t pBoneToWorld[MAXSTUDIOBONES], pBoneToWorldNext[MAXSTUDIOBONES];

	float dt = 0.1f;

	// Copy over dissolve state...
	if (pAnimating->IsEFlagSet(EFL_NO_DISSOLVE))
	{
		pRagdoll->AddEFlags(EFL_NO_DISSOLVE);
	}

	// NOTE: This currently is only necessary to prevent manhacks from
	// colliding with server ragdolls they kill
	pRagdoll->SetKiller(info.GetInflictor());
	pRagdoll->SetSourceClassName(pAnimating->GetClassname());

	// NPC_STATE_DEAD npc's will have their COND_IN_PVS cleared, so this needs to force SetupBones to happen
	unsigned short fPrevFlags = pAnimating->GetBoneCacheFlags();
	pAnimating->SetBoneCacheFlags(BCF_NO_ANIMATION_SKIP);

	// UNDONE: Extract velocity from bones via animation (like we do on the client)
	// UNDONE: For now, just move each bone by the total entity velocity if set.
	// Get Bones positions before
	// Store current cycle
	float fSequenceDuration = pAnimating->SequenceDuration(pAnimating->GetSequence());
	float fSequenceTime = pAnimating->GetCycle() * fSequenceDuration;

	if (fSequenceTime <= dt && fSequenceTime > 0.0f)
	{
		// Avoid having negative cycle
		dt = fSequenceTime;
	}

	float fPreviousCycle = clamp(pAnimating->GetCycle() - (dt * (1 / fSequenceDuration)), 0.f, 1.f);
	float fCurCycle = pAnimating->GetCycle();
	// Get current bones positions
	pAnimating->SetupBones(pBoneToWorldNext, BONE_USED_BY_ANYTHING);
	// Get previous bones positions
	pAnimating->SetCycle(fPreviousCycle);
	pAnimating->SetupBones(pBoneToWorld, BONE_USED_BY_ANYTHING);
	// Restore current cycle
	pAnimating->SetCycle(fCurCycle);

	// Reset previous bone flags
	pAnimating->ClearBoneCacheFlags(BCF_NO_ANIMATION_SKIP);
	pAnimating->SetBoneCacheFlags(fPrevFlags);

	Vector vel = pAnimating->GetAbsVelocity();
	if ((vel.Length() == 0) && (dt > 0))
	{
		// Compute animation velocity
		CStudioHdr *pstudiohdr = pAnimating->GetModelPtr();
		if (pstudiohdr)
		{
			Vector deltaPos;
			QAngle deltaAngles;
			if (Studio_SeqMovement(pstudiohdr,
				pAnimating->GetSequence(),
				fPreviousCycle,
				pAnimating->GetCycle(),
				pAnimating->GetPoseParameterArray(),
				deltaPos,
				deltaAngles))
			{
				VectorRotate(deltaPos, pAnimating->EntityToWorldTransform(), vel);
				vel /= dt;
			}
		}
	}

	if (vel.LengthSqr() > 0)
	{
		int numbones = pAnimating->GetModelPtr()->numbones();
		vel *= dt;
		for (int i = 0; i < numbones; i++)
		{
			Vector pos;
			MatrixGetColumn(pBoneToWorld[i], 3, pos);
			pos -= vel;
			MatrixSetColumn(pos, 3, pBoneToWorld[i]);
		}
	}

	#if RAGDOLL_VISUALIZE
	pAnimating->DrawRawSkeleton(pBoneToWorld, BONE_USED_BY_ANYTHING, true, 20, false);
	pAnimating->DrawRawSkeleton(pBoneToWorldNext, BONE_USED_BY_ANYTHING, true, 20, true);
	#endif

	Vector damageForce = info.GetDamageForce();

	// Is this a vehicle / NPC collision?
	if ((info.GetDamageType() & DMG_VEHICLE) && pAnimating->MyNPCPointer())
	{
		// init the ragdoll with no forces
		pRagdoll->InitRagdoll(vec3_origin, -1, vec3_origin, pBoneToWorld, pBoneToWorldNext, dt, collisionGroup, true);

		// apply vehicle forces
		// Get a list of bones with hitboxes below the plane of impact
		int boxList[128];
		Vector normal(0, 0, -1);
		int count = pAnimating->GetHitboxesFrontside(boxList, ARRAYSIZE(boxList), normal, DotProduct(normal, info.GetDamagePosition()));

		// distribute force over mass of entire character
		float massScale = Studio_GetMass(pAnimating->GetModelPtr());
		massScale = clamp(massScale, 1.f, 1.e4f);
		massScale = 1.f / massScale;

		// distribute the force
		// BUGBUG: This will hit the same bone twice if it has two hitboxes!!!!
		ragdoll_t *pRagInfo = pRagdoll->GetRagdoll();
		for (int i = 0; i < count; i++)
		{
			int physBone = pAnimating->GetPhysicsBone(pAnimating->GetHitboxBone(boxList[i]));
			IPhysicsObject *pPhysics = pRagInfo->list[physBone].pObject;
			pPhysics->ApplyForceCenter(info.GetDamageForce() * pPhysics->GetMass() * massScale);
		}
	}
	else if ((info.GetDamageType() & (DMG_BULLET | DMG_BUCKSHOT | DMG_CLUB)) && pAnimating->MyNPCPointer())
	{
		pRagdoll->InitRagdoll(damageForce * phys_npcdamagephysicsscale1.GetFloat() + Vector(0, 0, ragdoll_zvector.GetFloat()), forceBone, info.GetDamagePosition(), pBoneToWorld, pBoneToWorldNext, dt, collisionGroup, true);
		if (gore_bleeding.GetBool())
		{
			CBloodBleedingSource *pBleed = (CBloodBleedingSource *)CreateEntityByName("bloodsource");
			pBleed->SetBloodColor(pRagdoll->BloodColor());
			pBleed->SetVariables(pRagdoll->GetRagdoll(), forceBone);
			pBleed->Spawn();
		}
	}
	else
	{
		pRagdoll->InitRagdoll(damageForce * phys_npcdamagephysicsscale2.GetFloat() + Vector(0, 0, ragdoll_zvector.GetFloat()), forceBone, info.GetDamagePosition(), pBoneToWorld, pBoneToWorldNext, dt, collisionGroup, true);
	}

	// Are we dissolving?
	if (pAnimating->IsDissolving())
	{
		pRagdoll->TransferDissolveFrom(pAnimating);
	}
	else if (bUseLRURetirement)
	{
		pRagdoll->AddSpawnFlags(SF_RAGDOLLPROP_USE_LRU_RETIREMENT);
		s_RagdollLRU.MoveToTopOfLRU(pRagdoll);
	}

	Vector mins, maxs;
	mins = pAnimating->CollisionProp()->OBBMins();
	maxs = pAnimating->CollisionProp()->OBBMaxs();
	pRagdoll->CollisionProp()->SetCollisionBounds(mins, maxs);

	return pRagdoll;
}