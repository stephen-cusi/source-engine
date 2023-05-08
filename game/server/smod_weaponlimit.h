#include "igamesystem.h"
#include "hl2_player_shared.h"

extern bool CanPickUpWeapon(CHL2_Player *pPlayer, CBaseCombatWeapon *pWeapon);

class CWeaponLimit : public CAutoGameSystem
{
public:
	CWeaponLimit(char const *name) : CAutoGameSystem(name)
	{
	}

	virtual void LevelInitPostEntity(void);
	virtual void LevelShutdownPostEntity(void);
	virtual void ParseScriptFile(const char* filename, bool official = false);
	virtual const char* GetEntityNameFromFile(KeyValues *keyvalues);

	KeyValues* m_pMapScript;

	const char *szReplacedEntityName[512][16];
};
extern CWeaponLimit *GetWeaponLimit();