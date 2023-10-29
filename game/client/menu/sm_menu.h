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
#include <vgui_controls/Divider.h>
#include "game_controls/basemodel_panel.h"

class SMToolMenu : public vgui::PropertyPage
{
	typedef vgui::PropertyPage BaseClass;
public:
	SMToolMenu( vgui::Panel *parent, const char *panelName );

	virtual void OnTick( void );
	virtual void OnCommand( const char *pcCommand );
private:
	vgui::TextEntry *m_Red; 
	vgui::TextEntry *m_Green;
	vgui::TextEntry *m_Blue; 
	vgui::TextEntry *m_Modelscale; 
	vgui::TextEntry *m_EntCreate;
};

class SMMainMenu : public vgui::PropertyPage
{
	typedef vgui::PropertyPage BaseClass;
public:
	SMMainMenu(vgui::Panel* parent, const char* panelName);
	virtual void OnTick(void);
	virtual void OnCommand(const char* command);
	//virtual void OnCommand(const char* pcCommand);
private:
	int hTestFont;
	vgui::Button* m_Impulse;
	vgui::Divider* m_ImpNoDiv;
	vgui::Button* m_Noclip;
	vgui::Divider* m_NoGoDiv;
	vgui::Button* m_God;
	vgui::Divider* m_GoNoTDiv;
	vgui::Button* m_Notarget;
	//vgui::TextEntry* m_Green;
	//vgui::TextEntry* m_Blue;
	//vgui::TextEntry* m_Modelscale;
	//vgui::TextEntry* m_EntCreate;
};

class SMModels : public vgui::PropertyPage
{
	typedef vgui::PropertyPage BaseClass;

public:
	SMModels( vgui::Panel *parent, const char *panelName );
	virtual void OnTick();
	const char *GetText();
	virtual void InitModels( Panel *panel, const char *modeltype, const char *modelfolder, const char *mdlPath  );
	virtual void OnCommand( const char *command );

private:
	vgui::ComboBox *box;
	vgui::PanelListPanel *list;
	CMDLPanel *mdl;

	char sz_mdlname[260];
};

class SMList : public vgui::PanelListPanel
{
public:
	typedef vgui::PanelListPanel BaseClass;

	SMList( vgui::Panel *parent, const char *pName );

	virtual void OnTick( void );
	virtual void OnCommand( const char *command );
	virtual void PerformLayout();
	virtual void AddImageButton( vgui::PanelListPanel *panel, const char *image, const char *hover, const char *command, const char* textName, const char* text);
	virtual void AddTextButton(vgui::PanelListPanel *panel, const char *name, const char *text, vgui::PanelListPanel *panelTarget, const char *command);
	virtual void AddModelPanel( vgui::PanelListPanel *panel, const char *mdlname, const char *cmd );
	virtual void InitEntities( KeyValues *kv, vgui::PanelListPanel *panel, const char *enttype );
	virtual void InitWeapons(KeyValues* kv, vgui::PanelListPanel* panel, const char* enttype);
	virtual void InitModels( vgui::PanelListPanel *panel, const char *modeltype, const char *modelfolder, const char *mdlPath );
private:
	CUtlVector<vgui::Panel *> m_LayoutItems;
	vgui::Label *m_Impulse;
	int hTestFont;
};

class CSMenu : public vgui::PropertyDialog
{
	typedef vgui::PropertyDialog BaseClass;
public:
	CSMenu( vgui::VPANEL *parent, const char *panelName );

	void OnTick();
	void OnCommand( const char *command ); 
};

class SMPanel
{
public:
	virtual void		Create(vgui::VPANEL parent) = 0;
	virtual void		Destroy(void) = 0;
	virtual void		Activate(void) = 0;
};

extern SMPanel* smenu;

