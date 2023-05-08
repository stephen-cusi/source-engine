//========= Copyright © 2018, Aulov Nikolay, All rights reserved. ============//
//
// Purpose: 762mm Bullet!!!
//
//=============================================================================//

#ifndef BULLET_762MM_H
#define BULLET_762MM_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_behavior.h"
#include "ai_goalentity.h"

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "beam_shared.h"
#include "rumble_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// #define BULLET_MODEL	"models/weapons/bt_9mm.mdl"

#define BOLT_AIR_VELOCITY	2500
#define BOLT_WATER_VELOCITY	1500

extern ConVar sk_plr_dmg_crossbow;
extern ConVar sk_npc_dmg_crossbow;

void TE_StickyBolt( IRecipientFilter& filter, float delay,	Vector vecDirection, const Vector *origin );

#define	BOLT_SKIN_NORMAL	0
#define BOLT_SKIN_GLOW		1

//-----------------------------------------------------------------------------
// Crossbow Bolt
//-----------------------------------------------------------------------------
class CBullet762MM : public CBaseCombatCharacter
{
	DECLARE_CLASS( CBullet762MM, CBaseCombatCharacter );

public:
	CBullet762MM() { };
	~CBullet762MM();

	Class_T Classify( void ) { return CLASS_NONE; }

public:
	void Spawn( void );
	void Precache( void );
	void BulletThink( void );
	void BulletTouch( CBaseEntity *pOther );
	bool CreateVPhysics( void );
	unsigned int PhysicsSolidMaskForEntity() const;
	static CBullet762MM *BulletCreate( const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner = NULL );

private:

	Ray_t	m_rayShot;

protected:

	bool	CreateSprites( void );

	CHandle<CSpriteTrail>	m_pGlowTrail;

	DECLARE_DATADESC();

	/*
	DECLARE_SERVERCLASS();
	*/
};

LINK_ENTITY_TO_CLASS( bullet_762mm, CBullet762MM );

#endif // AI_BEHAVIOR_ACTBUSY_H
