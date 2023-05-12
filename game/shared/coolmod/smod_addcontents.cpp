//========= Copyright Totterynine, All rights reserved. ======================//
//
// Purpose: Addcontents Mounting System
//
//=============================================================================//
#include "cbase.h"
#include "filesystem.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DEBUG_MSG TRUE // Change this to "TRUE" to show Debug Info on Release Mode

#if DEBUG_MSG || DEBUG
#define DebugColorMsg(msg) ConColorMsg(Color(255, 124, 0, 255), msg)
#else
#define DebugColorMsg(msg)
#endif

class CAddContent : public CAutoGameSystem
{
public:
	CAddContent(char const *name) : CAutoGameSystem(name)
	{
	}

	virtual bool Init();
	virtual void LevelInitPostEntity(void);
	virtual void LevelShutdownPostEntity(void);
	virtual void ParseScriptFile(const char* filename, bool official = false);
	void MountSteamGame(IFileSystem* filesystem);

	KeyValues* m_AddContentScript;
};

CAddContent g_AddContentParser("AddContentParser");

CAddContent *GetAddContentParser()
{
	return &g_AddContentParser;
}

//-----------------------------------------------------------------------------
// Add Content implementation.
//-----------------------------------------------------------------------------

void AddContent(IFileSystem* filesystem)
{

	CAddContent *parser = GetAddContentParser();
	if (parser)
	{
		if (parser->m_AddContentScript)
		{
			parser->MountSteamGame(filesystem);
		}
	}
}

bool CAddContent::Init()
{
	char filename[FILENAME_MAX];

	Q_snprintf(filename, sizeof(filename), "scripts/%s.txt", "addcontents");
	ParseScriptFile(filename);
	return true;
}
void CAddContent::LevelInitPostEntity()
{
}

void CAddContent::ParseScriptFile(const char* filename, bool official)
{
	if (!m_AddContentScript)
		m_AddContentScript = new KeyValues("Additional contents list");
	if (!m_AddContentScript->LoadFromFile(filesystem, filename))
	{
		m_AddContentScript->deleteThis();
		m_AddContentScript = nullptr;
		return;
	}
}

void CAddContent::MountSteamGame(IFileSystem * filesystem)
{
	KeyValues *pContents = m_AddContentScript->FindKey("Contents");
	if (pContents)
	{
		FOR_EACH_SUBKEY(pContents, pContentName)
		{
			const char *path = pContentName->GetString("path", "hl2");
			int appid = pContentName->GetInt("id", 0);

			filesystem->AddSearchPath(path, "GAME");

			char fullpath[512];
			filesystem->RelativePathToFullPath(path, "GAME", fullpath, sizeof(fullpath));

			Q_snprintf(fullpath, sizeof(fullpath), "%s*", fullpath);

			FileFindHandle_t findHandle; // note: FileFINDHandle
			const char *pFilename = filesystem->FindFirstEx(fullpath, "MOD", &findHandle);
			while (pFilename)
			{
				if(Q_stristr(pFilename, "_dir") != 0)
				{

					filesystem->AddSearchPath(CFmtStr("%s%s", path, pFilename).String(), "GAME");
				}
				pFilename = filesystem->FindNext(findHandle);
			}

			if (appid)
			{

				(filesystem->MountSteamContent(-appid) != FILESYSTEM_MOUNT_OK);
					//DebugColorMsg(CFmtStr("Unable to mount extra content with appId: %i\n", appid));
			}

		}
	}
}

void CAddContent::LevelShutdownPostEntity()
{
	if (m_AddContentScript)
	{
		m_AddContentScript->deleteThis();
		m_AddContentScript = nullptr;
	}
}