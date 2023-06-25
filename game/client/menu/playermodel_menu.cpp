#include "cbase.h"
#include <stdio.h>
#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/MenuButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/MenuItem.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include "playermodel_menu.h"
#include "filesystem.h"
#include "vgui_imagebutton.h"
#include "game_controls/basemodel_panel.h"

#include "tier0/memdbgon.h"

using namespace vgui;

/*
// List of playermodels
SMPList::SMPList( vgui::Panel *parent, const char *pName ) : BaseClass( parent, pName )
{
	SetBounds( 0, 0, 800, 640 );
}

void SMPList::OnTick( void )
{
	BaseClass::OnTick();

	if ( !IsVisible() )
		return;

	int c = m_LayoutItems.Count();
	for ( int i = 0; i < c; i++ )
	{
		vgui::Panel *p = m_LayoutItems[ i ];
		p->OnTick();
	}
}

void SMPList::OnCommand( const char *command )
{
	printf("%s\n", command);
	engine->ClientCmd( (char *)command );
}

void SMPList::PerformLayout()
{
	BaseClass::PerformLayout();

	int w = 64;
	int h = 64;
	int x = 5;
	int y = 5;

	for ( int i = 0; i < m_LayoutItems.Count(); i++ )
	{	
		vgui::Panel *p = m_LayoutItems[ i ];
		p->SetBounds( x, y, w, h );

		x += ( w + 2 );
		if ( x >= GetWide() - w )
		{
			y += ( h + 2 );
			x = 5;
		}	
	}
}

void RenderModelToVTFTexture(const char* mdlname)
{

	model_t *model = (model_t *)engine->LoadModel( mdlname );
	studiohdr_t* studioHdr = modelinfo->GetStudiomodel(model);

}

void SMPList::AddModelPanel( PanelListPanel *panel, const char *mdlname, const char *cmd)
{
	CMDLPanel *mdl = new CMDLPanel( panel, "MDLPanel", cmd );
	mdl->SetMDL( mdlname );
//	mdl->SetSequence(ACT_IDLE_AGITATED);
	m_LayoutItems.AddToTail( mdl );
	panel->AddItem( NULL, mdl );
	RenderModelToVTFTexture(mdlname);
}

*/

SMPModels::SMPModels( vgui::Panel *parent, const char *panelName ) : BaseClass( parent, panelName )
{
	vgui::ivgui()->AddTickSignal( GetVPanel(), 250 );
	box = new ComboBox( this, "ComboBox", 100, true);
	mdl = new CMDLPanel( this, "MDLPanel", NULL, false );
	LoadControlSettings("resource/ui/playermenu.res");
}

void SMPModels::OnTick()
{
}

void SMPModels::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );

	if(!Q_stricmp(command, "setmodel"))
	{
		char spawncommand[260];
		box->GetText( sz_mdlname, 260 );
		if (strlen(sz_mdlname) == 0)
			return;
		Q_snprintf( spawncommand, sizeof( spawncommand ), "cl_playermodel %s", sz_mdlname );
		printf("%s\n", spawncommand);
		engine->ClientCmd( spawncommand );
	}
	else if ( !Q_stricmp( command, "preview")  )
	{	
		char mdlname[260];
		box->GetText( sz_mdlname, 260 );
		if (strlen(sz_mdlname) == 0)
			return;
		mdl->SetMDL( sz_mdlname );
	}
}

ConVar sm_pmenu("sm_pmenu", "0", FCVAR_CLIENTDLL, "Opens playermodel selection menu");

void CSPMenu::ParsePlayermodels(const char* directory, vgui::ComboBox *box)
{
    FileFindHandle_t fh;
    char searchPath[MAX_PATH];
    Q_snprintf(searchPath, sizeof(searchPath), "%s/*", directory);

    for (const char* pFile = filesystem->FindFirstEx(searchPath, "MOD", &fh); pFile && *pFile; pFile = filesystem->FindNext(fh))
    {
        char filePath[MAX_PATH], cmd[MAX_PATH];
        Q_snprintf(filePath, sizeof(filePath), "%s/%s", directory, pFile);

        if (filesystem->FindIsDirectory(fh))
        {
            if (Q_strcmp(pFile, ".") != 0 && Q_strcmp(pFile, "..") != 0)
                ParsePlayermodels(filePath, box);
        }
        else
        {
            if (Q_strstr(pFile, ".mdl") && !Q_strstr(pFile, "anims.mdl"))
			{
				Q_snprintf(cmd, sizeof(cmd), "cl_playermodel %s", filePath);
				box->AddItem( filePath, NULL );
			}
        }
    }
    filesystem->FindClose(fh);
}

CSPMenu::CSPMenu( vgui::VPANEL *parent, const char *panelName ) : BaseClass( NULL, "SMenu" )
{
	SetWide( 800 );
	SetTall( 640 );
	SetTitle( "Playermodel menu", true );
	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/sch.res", "SourceScheme"));
	
	SMPModels *mdl = new SMPModels( this, "panel" );
	
    ParsePlayermodels("models/player", mdl->box);
	AddPage( mdl, "Playermodels" );
	
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
	GetPropertySheet()->SetTabWidth(72);
	SetMoveable( true );
	SetVisible( true );
	SetSizeable( false );
}

void CSPMenu::OnTick()
{
	BaseClass::OnTick();
	SetVisible(sm_pmenu.GetBool());
}

void CSPMenu::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );
		
	if (!Q_stricmp(command, "Close"))	
	{
		sm_pmenu.SetValue(0);
	}
}

class CSMPPanelInterface : public SMPPanel
{
private:
	CSPMenu *SMPPanel;
public:
	CSMPPanelInterface()
	{
		SMPPanel = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		SMPPanel = new CSPMenu(&parent, "SMenu");
	}
	void Destroy()
	{
		if (SMPPanel)
		{
			SMPPanel->SetParent((vgui::Panel *)NULL);
			delete SMPPanel;
		}
	}
	void Activate(void)
	{
		if (SMPPanel)
		{
			SMPPanel->Activate();
		}
	}
};
static CSMPPanelInterface g_SMPPanel;
SMPPanel* smpenu = (SMPPanel*)&g_SMPPanel;
