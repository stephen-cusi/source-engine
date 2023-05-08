//========= Made by The Totterynine, Some rights reserved. ============//
//
// Purpose: Combat ready npcs from SMOD
//
//=============================================================================//

#include "cbase.h"
#include "npc_citizen17.h"

ConVar	sk_f_combatnpc_health("sk_f_combatnpc_health", "25");

class CNPC_BaseCombat : public CNPC_Citizen
{
public:
	DECLARE_CLASS(CNPC_BaseCombat, CNPC_Citizen);
	//DECLARE_SERVERCLASS();

	virtual void Precache();
	void	Spawn(void);
	Class_T Classify(void);
	void SetClass(Class_T value) { class_t = value; }
	void SetCombatModel(char* newmodel) { modelname = AllocPooledString(newmodel); }

private:

	Class_T class_t;
	string_t modelname;
};

//IMPLEMENT_SERVERCLASS_ST(CNPC_BaseCombat, DT_NPC_BaseCombat)
//END_SEND_TABLE()


void CNPC_BaseCombat::Precache(void)
{
	m_Type = CT_UNIQUE;

	SetModelName(modelname);

	CNPC_Citizen::Precache();
}


Class_T	CNPC_BaseCombat::Classify(void)
{
	return	class_t;
}

void CNPC_BaseCombat::Spawn(void)
{
	Precache();

	CNPC_Citizen::Spawn();

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);

	m_iHealth = 10;
}

class CNPC_BarneyCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_BarneyCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_barney, CNPC_BarneyCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_BarneyCombat, DT_NPC_BarneyCombat)
//END_SEND_TABLE()

void CNPC_BarneyCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/barney.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_AlyxCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_AlyxCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_alyx, CNPC_AlyxCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_AlyxCombat, DT_NPC_AlyxCombat)
//END_SEND_TABLE()

void CNPC_AlyxCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/alyx.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_MonkCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_MonkCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_monk, CNPC_MonkCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_MonkCombat, DT_NPC_MonkCombat)
//END_SEND_TABLE()

void CNPC_MonkCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/monk.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_EliCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_EliCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_eli, CNPC_EliCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_EliCombat, DT_NPC_EliCombat)
//END_SEND_TABLE()

void CNPC_EliCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/eli.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_MossmanCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_MossmanCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_mossman, CNPC_MossmanCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_MossmanCombat, DT_NPC_MossmanCombat)
//END_SEND_TABLE()

void CNPC_MossmanCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/mossman.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_MagnussonCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_MagnussonCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_magnusson, CNPC_MagnussonCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_MagnussonCombat, DT_NPC_MagnussonCombat)
//END_SEND_TABLE()

void CNPC_MagnussonCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/magnusson.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_BreenCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_BreenCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_breen, CNPC_BreenCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_BreenCombat, DT_NPC_BreenCombat)
//END_SEND_TABLE()

void CNPC_BreenCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/breen.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_GmanCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_GmanCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_gman, CNPC_GmanCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_GmanCombat, DT_NPC_GmanCombat)
//END_SEND_TABLE()

void CNPC_GmanCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/gman.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_OdessaCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_OdessaCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_odessa, CNPC_OdessaCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_OdessaCombat, DT_NPC_OdessaCombat)
//END_SEND_TABLE()

void CNPC_OdessaCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/odessa.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_KleinerCombat : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_KleinerCombat, CNPC_BaseCombat);
	//DECLARE_SERVERCLASS();
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_kleiner, CNPC_KleinerCombat);

//IMPLEMENT_SERVERCLASS_ST(CNPC_OdessaCombat, DT_NPC_OdessaCombat)
//END_SEND_TABLE()

void CNPC_KleinerCombat::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/kleiner.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}

class CNPC_FriendlyCombine : public CNPC_BaseCombat
{
	DECLARE_CLASS(CNPC_FriendlyCombine, CNPC_BaseCombat);
	void Spawn();
};

LINK_ENTITY_TO_CLASS(npc_f_combine, CNPC_FriendlyCombine);

void CNPC_FriendlyCombine::Spawn(void)
{
	SetClass(CLASS_PLAYER_ALLY);
	SetCombatModel("models/combine_f_soldier.mdl");
	Precache();

	CNPC_Citizen::Spawn();

	m_iHealth = sk_f_combatnpc_health.GetInt();
}