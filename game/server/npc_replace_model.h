#include "igamesystem.h"

const char *ReplaceModel(CBaseEntity *pEntity);

class CReplaceModelParser : public CAutoGameSystem
{
public:
	CReplaceModelParser(char const *name) : CAutoGameSystem(name)
	{
	}

	virtual void LevelInitPostEntity(void);
	virtual void LevelShutdownPostEntity(void);
	virtual void ParseScriptFile(const char* filename, bool official = false);
	virtual const char* GetEntityNameFromFile(KeyValues *keyvalues);

	KeyValues* m_pMapScript;

	const char *szReplacedEntityName[512][16];
};
extern CReplaceModelParser *GetReplaceModelParser();
