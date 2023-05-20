#include "igamesystem.h"

const char *ReplaceEntity(const char *entityname);

class CNodeListParser : public CAutoGameSystem
{
public:
	CNodeListParser(char const *name) : CAutoGameSystem(name)
	{
	}

	virtual bool Init();
	virtual void LevelInitPostEntity(void);
	virtual void LevelShutdownPostEntity(void);
	virtual int ParseScriptFile(const char* filename);
};
extern CNodeListParser *GetNodeListParser();