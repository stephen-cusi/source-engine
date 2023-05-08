#include "igamesystem.h"

class CMapScriptParser : public CAutoGameSystem
{
public:
	CMapScriptParser(char const *name);

	virtual void LevelInitPostEntity(void);
	virtual void OnRestore(void);
	virtual void LevelShutdownPostEntity(void);
	virtual void ParseScriptFile(const char* filename, bool official = false);
	virtual void ParseEntities(KeyValues *keyvalues);
	virtual void ParseRandomEntities(KeyValues *keyvalues);

	void ExecuteRandomEntites();

	void SetRestored(bool restored) { m_bRestored = restored; }

	KeyValues* m_pMapScript;
private:
	bool		m_bRestored;
	bool		m_hasLua;
};
extern CMapScriptParser *GetMapScriptParser();