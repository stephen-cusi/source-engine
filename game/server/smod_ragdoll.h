#include "cbase.h"
#include "physics_prop_ragdoll.h"

class CSMODRagdoll : public CRagdollProp
{
	DECLARE_CLASS(CSMODRagdoll, CRagdollProp);

public:
	CSMODRagdoll(void);
	~CSMODRagdoll(void);

	void Spawn();
	void Precache();
	virtual void TraceAttack(const CTakeDamageInfo &info, const Vector &dir, trace_t *ptr, CDmgAccumulator *pAccumulator);

	void SetBloodColor( int color ) { m_bloodColor = color; }


private:
	int	m_bloodColor;			// color of blood particless
	float m_flGibHealth;

	DECLARE_DATADESC();
};

CBaseEntity *CreateSMODServerRagdoll(CBaseAnimating *pAnimating, int forceBone, const CTakeDamageInfo &info, int collisionGroup, bool bUseLRURetirement = false);
