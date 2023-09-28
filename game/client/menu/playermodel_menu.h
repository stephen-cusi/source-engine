//========= Copyright iSquad Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "iclientmode.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include "game_controls/basemodel_panel.h"
/*
class SMPList : public vgui::PanelListPanel
{
public:
	typedef vgui::PanelListPanel BaseClass;
	SMPList( vgui::Panel *parent, const char *pName );
	virtual void OnTick( void );
	virtual void OnCommand( const char *command );
	virtual void PerformLayout();
	//virtual void AddImageButton( vgui::PanelListPanel *panel, const char *image, const char *hover, const char *command );
	virtual void AddModelPanel( vgui::PanelListPanel *panel, const char *mdlname, const char *cmd);
private:
	CUtlVector<vgui::Panel *> m_LayoutItems;
	
	KeyValues *pVMTKeyValues = new KeyValues( "UnlitGeneric" );
};
*/
class CSPMenu : public vgui::PropertyDialog
{
	typedef vgui::PropertyDialog BaseClass;
public:
	CSPMenu( vgui::VPANEL *parent, const char *panelName );

	void ParsePlayermodels(const char *, vgui::ComboBox *box);
	void OnTick();
	void OnCommand( const char *command ); 

};

class SMPModels : public vgui::PropertyPage
{
	typedef vgui::PropertyPage BaseClass;

public:
	SMPModels( vgui::Panel *parent, const char *panelName );
	virtual void OnTick();
	virtual void OnCommand( const char *command );
	vgui::ComboBox *box;

private:
	vgui::PanelListPanel *list;
	CMDLPanel *mdl;

	char sz_mdlname[260];
};

class SMPPanel
{
public:
	virtual void		Create(vgui::VPANEL parent) = 0;
	virtual void		Destroy(void) = 0;
	virtual void		Activate(void) = 0;
};

extern SMPPanel* smpenu;
