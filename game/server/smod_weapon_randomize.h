#include "igamesystem.h"
#include "ai_basenpc.h"

extern const char *GiveRandomWeapon(CAI_BaseNPC *pNPC);

class CWeaponRandomize : public CAutoGameSystem
{
public:
	CWeaponRandomize(char const *name) : CAutoGameSystem(name)
	{
	}

	virtual void LevelInitPreEntity(void);
	virtual void LevelShutdownPostEntity(void);
	virtual void ParseScriptFile(const char* filename, bool official = false);
	virtual const char* GetEntityNameFromFile(KeyValues *keyvalues);

	KeyValues* m_pMapScript;
};
extern CWeaponRandomize *GetWeaponRandomizeParser();
