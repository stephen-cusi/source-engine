#include "cbase.h"
#include "ragdoll_shared.h"

class CBloodDrip : public CBaseEntity
{
	DECLARE_CLASS(CBloodDrip, CBaseEntity);	// setup some macros
public:
	CBloodDrip();

	void Spawn(void);
	void ShouldSpawnDrops(bool should) { m_bShouldSpawnDrops = should; }
private:

	void BloodThink(void);
	void AlienBloodThink(void);

	void SetBloodColor(int bloodcolor);
	int GetBloodColor();

	int ibloodcolor;

	float m_flNextParticle;
	bool m_bHitWater;

	bool m_bShouldSpawnDrops;

	virtual void Precache();

	DECLARE_DATADESC();
};

class CBloodBleedingSource : public CBaseEntity
{
	DECLARE_CLASS(CBloodBleedingSource, CBaseEntity);
	void BloodThink(void);

	int GetBloodColor();

	int ibloodcolor;
	int iBloodDrips;
	float m_flNextBloodDrip;

	virtual void Precache();

	DECLARE_DATADESC();
public:
	void Spawn(void);
	void SetVariables(ragdoll_t *ragdoll, int bone);
	void SetBloodColor(int bloodcolor);

private:
	int physBone;
	ragdoll_t *pRagdoll;
};

class CBloodDrop : public CBaseEntity
{
	DECLARE_CLASS(CBloodDrop, CBaseEntity);
	void BloodThink(void);

	int GetBloodColor();

	int ibloodcolor;
	int iBloodDrips;
	float m_flNextBloodDrip;

	virtual void Precache();

	DECLARE_DATADESC();
public:
	void Spawn(void);
	void SetBloodColor(int bloodcolor);
};

class CSparkDrip : public CBaseEntity
{
	DECLARE_CLASS(CSparkDrip, CBaseEntity);
	void Spawn(void);
	void SparkThink(void);

	virtual void Precache();

	DECLARE_DATADESC();
};