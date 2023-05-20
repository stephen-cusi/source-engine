//========= Copyright Totterynine, All rights reserved. ======================//
//
// Purpose: .SNL Parser System
//
//=============================================================================//
#include "cbase.h"
#include "filesystem.h"
#include "fmtstr.h"
#include "ai_network.h"
#include "ai_node.h"
#include "ai_networkmanager.h"
#include "smod_nodelistparser.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DEBUG_MSG TRUE // Change this to "TRUE" to show Debug Info on Release Mode

#if DEBUG_MSG || DEBUG
#define DebugColorMsg(msg) ConColorMsg(Color(0, 124, 255, 255), msg)
#else
#define DebugColorMsg(msg)
#endif

CNodeListParser g_NodeListParser("NodeListParser");

CNodeListParser *GetNodeListParser()
{
	return &g_NodeListParser;
}

CON_COMMAND(sv_nodelist_debug, "Parsing file and output the error message")
{
	int msg = GetNodeListParser()->ParseScriptFile(CFmtStr("mapadd/nodes/%s.snl", gpGlobals->mapname));

	switch (msg)
	{
	case 0:
		Warning("NodeList: Parsed File Succesfuly\n");
		break;
	case 1:
		Warning("NodeList: Opened file but couldnt parse\n");
		break;
	case 2:
		Warning("NodeList: Could not open the file\n");
		break;
	default:
		break;
	}
}


bool CNodeListParser::Init()
{
	return true;
}

void CNodeListParser::LevelInitPostEntity()
{
	//ParseScriptFile(CFmtStr("mapadd/nodes/%s.snl", gpGlobals->mapname));
}


void CNodeListParser::LevelShutdownPostEntity()
{
}

int CNodeListParser::ParseScriptFile(const char *filename)
{
	FileHandle_t fh = filesystem->Open(filename, "rb");
	bool parsed = false;


	if (fh)
	{
		int file_len = filesystem->Size(fh);
		char* nodelist = new char[file_len + 1];

		char szLine[MAX_PATH];

		while (!filesystem->EndOfFile(fh)) {

			// get a single line
			szLine[0] = 0;
			g_pFullFileSystem->ReadLine(szLine, sizeof(szLine) - 1, fh);
			szLine[sizeof(szLine) - 1] = 0;

			char func_type[16];

			sscanf(szLine, "%s ", func_type);

			if (FStrEq(func_type, "N"))
			{
				int iNodeType;
				char hint[16], node_type[16], pos_x[32], pos_y[32], pos_z[32], ang_yaw[32];
				float flpos_x, flpos_y, flpos_z, flang_yaw;
				sscanf(szLine, "%s \"%s %s %s %s %s %s\"", func_type, hint, node_type, pos_x, pos_y, pos_z, ang_yaw);
				iNodeType = atoi(node_type);
				flpos_x = atof(pos_x);
				flpos_y = atof(pos_y);
				flpos_z = atof(pos_z);
				flang_yaw = atof(ang_yaw);

				CAI_Node *new_node = g_pBigAINet->AddNode(Vector(flpos_x, flpos_y, flpos_z), flang_yaw);
				new_node->SetType(NODE_GROUND);

				g_AINetworkBuilder.InitNodePosition(g_pBigAINet, new_node);
			}
			else if (FStrEq(func_type, "L"))
			{
				char connectpoint1[16], connectpoint2[16], type[16];
				int iConnectPoint1, iConnectPoint2, iType;
				sscanf(szLine, "%s \"%s %s %s\"", func_type, connectpoint1, connectpoint2, type);

				iConnectPoint1 = atoi(connectpoint1);
				iConnectPoint2 = atoi(connectpoint2);
				iType = atoi(type);

				g_pBigAINet->CreateLink(iConnectPoint1, iConnectPoint2);
			}

			parsed = true;
		}
		g_AINetworkBuilder.Build(g_pBigAINet);
		g_AINetworkBuilder.Rebuild(g_pBigAINet);

		filesystem->Close(fh);

		delete[] nodelist;

		if (!parsed)
			return 1;
	}
	else
	{
		return 2;
	}

	return 0;
}