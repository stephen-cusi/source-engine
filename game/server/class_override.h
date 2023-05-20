#include "igamesystem.h"

const char *ReplaceEntity(const char *entityname);

class CClassOverrideParser : public CAutoGameSystem
{
public:
	CClassOverrideParser(char const *name) : CAutoGameSystem(name)
	{
	}

	virtual void LevelInitPreEntity(void);
	virtual void LevelShutdownPostEntity(void);
	virtual void ParseScriptFile(const char* filename, bool official = false);
	virtual const char* GetEntityNameFromFile(KeyValues *keyvalues);

	KeyValues* m_pMapScript;
};
extern CClassOverrideParser *GetClassOverrideParser();
