#include "cbase.h"
#include "detail_entities.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "coolmod/smod_cvars.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(blooddrip, CBloodDrip);
LINK_ENTITY_TO_CLASS(blooddrip_alien, CBloodDrip);

ConVar gore_ragdoll_bloodstain( "gore_ragdoll_bloodstain", "1", FCVAR_ARCHIVE );

CBloodDrip::CBloodDrip()
{
	m_bShouldSpawnDrops = true;
}

void CBloodDrip::Precache()
{
	PrecacheScriptSound("Player.DropBlood");
}

void CBloodDrip::Spawn()
{
	Precache();

	m_flNextParticle = 0;
	m_bHitWater = false;

	m_iHealth = 50 * random->RandomFloat(1.0f, 5.0f);

	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetSolid(SOLID_NONE);

	QAngle Dir = vec3_angle;
	Vector VectorVelocity = GetAbsVelocity();

	VectorAngles(VectorVelocity, Dir);

	SetAbsAngles(Dir);

	CEffectData	data;

	const char *pEntName = STRING(this->GetEntityName());

	if (Q_stristr(pEntName, "alien") || Q_stristr(GetClassname(), "alien"))
	{
		SetThink(&CBloodDrip::AlienBloodThink);
	}
	else
	{
		SetThink(&CBloodDrip::BloodThink);
	}
	SetNextThink(gpGlobals->curtime);
}

void CBloodDrip::BloodThink()
{
	SetNextThink(gpGlobals->curtime + 0.01);
	int bloodcolor = BLOOD_COLOR_RED;

	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 0.02, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	if ( gore_ragdoll_bloodstain.GetBool() )
	{
		UTIL_BloodDecalTrace(&tr, bloodcolor);
	}
	else
	{
		return;
	}

	if (tr.plane.normal[2] < -0.5f && m_bShouldSpawnDrops && tr.DidHitWorld())
	{
		CBloodDrop *pBloodDrop = (CBloodDrop*)CreateEntityByName("blooddrop");
		pBloodDrop->SetBloodColor(ibloodcolor);
		pBloodDrop->SetAbsOrigin(tr.endpos);
		pBloodDrop->Spawn();
	}

	CEffectData	data;
	data.m_vOrigin = GetAbsOrigin();
	data.m_vStart = GetAbsVelocity();
	data.m_flScale = 5.0f;
	data.m_fFlags = 0xFF;
	data.m_nColor = bloodcolor;

	if (m_flNextParticle < gpGlobals->curtime)
	{
		DispatchEffect("bloodtrail", data);
		m_flNextParticle = gpGlobals->curtime + 0.1;
	}

	if (!m_bHitWater && (enginetrace->GetPointContents(tr.endpos) & (CONTENTS_WATER | CONTENTS_SLIME)) && gore_spreadwatersurface.GetBool())
	{
		CEffectData quadData;
		quadData.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		quadData.m_vNormal = tr.plane.normal;
		quadData.m_nColor = bloodcolor;

		DispatchEffect("BloodPuddle", data);
		m_bHitWater = true;
	}

	CEffectData	dataimpact;
	if (tr.DidHit())
	{

		dataimpact.m_vOrigin = GetAbsOrigin();
		dataimpact.m_vNormal = tr.plane.normal;
		dataimpact.m_flScale = (float)5;
		dataimpact.m_fFlags = 0xFF;
		dataimpact.m_nColor = 0;

		DispatchEffect("bloodsplat", data);
		EmitSound("Player.DropBlood");
	}

	data.m_vOrigin = GetAbsOrigin();
	data.m_vStart = GetAbsVelocity();
	data.m_flScale = (float)5;
	data.m_fFlags = 0xFF;
	data.m_nColor = 0;

	if (m_iHealth-- < 1 || tr.DidHitWorld())
	{
		UTIL_Remove(this);
	}
}

void CBloodDrip::AlienBloodThink()
{

	SetNextThink(gpGlobals->curtime + 0.01f);

	int bloodcolor = BLOOD_COLOR_GREEN;

	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * 0.02, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);

	if ( gore_ragdoll_bloodstain.GetBool() )
	{
		UTIL_BloodDecalTrace(&tr, bloodcolor);
	}
	else
	{
		return;
	}

	CEffectData	data;
	data.m_vOrigin = GetAbsOrigin();
	data.m_vStart = GetAbsVelocity();
	data.m_flScale = 2.5f;
	data.m_fFlags = 0xFF;
	data.m_nColor = 1;

	if (m_flNextParticle < gpGlobals->curtime)
	{
		DispatchEffect("bloodtrail", data);
		m_flNextParticle = gpGlobals->curtime + 0.1;
	}

	if (!m_bHitWater && (enginetrace->GetPointContents(tr.endpos) & (CONTENTS_WATER | CONTENTS_SLIME)) && gore_spreadwatersurface.GetBool())
	{
		CEffectData quadData;
		quadData.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
		quadData.m_vNormal = tr.plane.normal;
		quadData.m_nColor = bloodcolor;

		DispatchEffect("BloodPuddle", data);
		m_bHitWater = true;
	}

	CEffectData	dataimpact;
	if (tr.DidHit())
	{
		dataimpact.m_vOrigin = GetAbsOrigin();
		dataimpact.m_vNormal = tr.plane.normal;
		dataimpact.m_flScale = (float)5;
		dataimpact.m_fFlags = 0xFF;
		dataimpact.m_nColor = 1;

		DispatchEffect("bloodsplat", data);
		EmitSound("Player.DropBlood");
	}

	data.m_vOrigin = GetAbsOrigin();
	data.m_vStart = GetAbsVelocity();
	data.m_flScale = (float)5;
	data.m_fFlags = 0xFF;
	data.m_nColor = 1;

	if (m_iHealth-- < 1 || tr.DidHitWorld())
	{
		Remove();
	}
}

void CBloodDrip::SetBloodColor(int bloodcolor)
{
	bloodcolor = ibloodcolor;
}

int CBloodDrip::GetBloodColor()
{
	return ibloodcolor;
}

BEGIN_DATADESC(CBloodDrip)
DEFINE_THINKFUNC(BloodThink),
END_DATADESC()

LINK_ENTITY_TO_CLASS(bloodsource, CBloodBleedingSource);

void CBloodBleedingSource::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("Player.Spout");
}

void CBloodBleedingSource::Spawn()
{
#ifndef CLIENT_DLL

	Precache();
	m_iHealth = 150 * random->RandomFloat(1.0f, 2.0f);
	m_flNextBloodDrip = 0;
	iBloodDrips = 1;

	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);

	SetThink(&CBloodBleedingSource::BloodThink);
	SetNextThink(gpGlobals->curtime);

#endif // !CLIENT_DLL
}

void CBloodBleedingSource::SetVariables(ragdoll_t *ragdoll, int bone)
{
	pRagdoll = ragdoll;
	physBone = bone;
}

void CBloodBleedingSource::BloodThink()
{
#ifndef CLIENT_DLL

	SetNextThink(gpGlobals->curtime + (0.01f));

	if (m_iHealth-- < 1)
	{
		Remove();
	}

	if (!pRagdoll)
		return;

	if (!pRagdoll->list[physBone].pObject)
		return;

	Vector bonePos;
	QAngle boneAng;
	pRagdoll->list[physBone].pObject->GetPosition(&bonePos, &boneAng);
	boneAng += QAngle(90, 0, 0);
	SetAbsOrigin(bonePos);
	SetAbsAngles(boneAng);

	Vector forward;
	AngleVectors(boneAng, &forward);

	if (m_flNextBloodDrip < gpGlobals->curtime)
	{
		if (ibloodcolor == DONT_BLEED)
		{
			// nothin
		}
		else if (ibloodcolor == BLOOD_COLOR_RED)
		{
			EmitSound("Player.Spout");
			CBloodDrip *pBloodDrip = (CBloodDrip *)CreateEntityByName("blooddrip");
			pBloodDrip->SetAbsOrigin(GetAbsOrigin());
			pBloodDrip->SetAbsVelocity(forward * (1000.0f / ((float)iBloodDrips / 2.0f)));
			pBloodDrip->ShouldSpawnDrops(true);
			pBloodDrip->Spawn();
		}
		else
		{
			EmitSound("Player.Spout");
			CBloodDrip *pBloodDrip = (CBloodDrip *)CreateEntityByName("blooddrip_alien");
			pBloodDrip->SetAbsOrigin(GetAbsOrigin());
			pBloodDrip->SetAbsVelocity(forward * (1000.0f / ((float)iBloodDrips / 2.0f)));
			pBloodDrip->ShouldSpawnDrops(true);
			pBloodDrip->Spawn();
		}


		m_flNextBloodDrip = gpGlobals->curtime + 0.01f * iBloodDrips;
		iBloodDrips++;
	}

#endif // !CLIENT_DLL
}

void CBloodBleedingSource::SetBloodColor(int bloodcolor)
{
	bloodcolor = ibloodcolor;
}

int CBloodBleedingSource::GetBloodColor()
{
	return ibloodcolor;
}

BEGIN_DATADESC(CBloodBleedingSource)
#ifndef CLIENT_DLL
DEFINE_THINKFUNC(BloodThink),
#endif // !CLIENT_DLL
END_DATADESC()

LINK_ENTITY_TO_CLASS(blooddrop, CBloodDrop);

void CBloodDrop::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("Player.Spout");
}

void CBloodDrop::Spawn()
{
#ifndef CLIENT_DLL

	Precache();
	m_iHealth = 150 * random->RandomFloat(1.0f, 2.0f);
	m_flNextBloodDrip = 0;
	iBloodDrips = 1;

	Vector vecVelocity;

	vecVelocity.x = random->RandomFloat(-200, 200);
	vecVelocity.y = random->RandomFloat(-200, 200);
	vecVelocity.z = random->RandomFloat(-100, 200);

	if (random->RandomInt(0, 1) == 0)
		vecVelocity.x *= -1;

	if (random->RandomInt(0, 1) == 0)
		vecVelocity.y *= -1;

	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));
	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);

	SetAbsVelocity(vecVelocity);

	SetThink(&CBloodDrop::BloodThink);
	SetNextThink(gpGlobals->curtime);

#endif // !CLIENT_DLL
}

void CBloodDrop::BloodThink()
{
#ifndef CLIENT_DLL

	SetNextThink(gpGlobals->curtime + (0.01f));

	if (m_flNextBloodDrip < gpGlobals->curtime)
	{
		EmitSound("Player.Spout");
		if (ibloodcolor == 0)
		{
			CBloodDrip *pBloodDrip = (CBloodDrip *)CreateEntityByName("blooddrip");
			pBloodDrip->SetAbsOrigin(GetAbsOrigin());
			pBloodDrip->ShouldSpawnDrops(false);
			pBloodDrip->Spawn();
		}
		else
		{
			CBloodDrip *pBloodDrip = (CBloodDrip *)CreateEntityByName("blooddrip_alien");
			pBloodDrip->SetAbsOrigin(GetAbsOrigin());
			pBloodDrip->ShouldSpawnDrops(false);
			pBloodDrip->Spawn();
		}
		m_flNextBloodDrip = gpGlobals->curtime + RandomFloat(0.01f, 0.025f) * iBloodDrips;
		iBloodDrips++;
	}

	if (m_iHealth-- < 1)
	{
		Remove();
	}

#endif // !CLIENT_DLL
}

void CBloodDrop::SetBloodColor(int bloodcolor)
{
	bloodcolor = ibloodcolor;
}

int CBloodDrop::GetBloodColor()
{
	return ibloodcolor;
}

BEGIN_DATADESC(CBloodDrop)
#ifndef CLIENT_DLL
DEFINE_THINKFUNC(BloodThink),
#endif // !CLIENT_DLL
END_DATADESC()

LINK_ENTITY_TO_CLASS(sparkthing, CSparkDrip);

void CSparkDrip::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound("DoSpark");
}

void CSparkDrip::Spawn()
{
#ifndef CLIENT_DLL
	Precache();

	EmitSound("DoSpark");

	m_iHealth = 20 + random->RandomInt(0, 5);

	UTIL_SetSize(this, Vector(0, 0, 0), Vector(0, 0, 0));
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetSolid(SOLID_NONE);

	SetThink(&CSparkDrip::SparkThink);
	SetNextThink(gpGlobals->curtime);
#endif // !CLIENT_DLL
}

void CSparkDrip::SparkThink()
{
#ifndef CLIENT_DLL
	SetNextThink(gpGlobals->curtime + 0.005f);

	g_pEffects->Sparks(GetAbsOrigin(), 1);

	if (m_iHealth-- < 1)
	{
		UTIL_Remove(this);
	}
#endif // !CLIENT_DLL
}

BEGIN_DATADESC(CSparkDrip)
#ifndef CLIENT_DLL
DEFINE_THINKFUNC(SparkThink),
#endif // !CLIENT_DLL
END_DATADESC()
