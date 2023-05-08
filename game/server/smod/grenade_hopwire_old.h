//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
//=============================================================================

#ifndef GRENADE_HOPWIRE_H
#define GRENADE_HOPWIRE_H
#ifdef _WIN32
#pragma once
#endif

#include "basegrenade_shared.h"
#include "Sprite.h"

class CGrenadeHopWireOld : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeHopWireOld, CBaseGrenade );
	DECLARE_DATADESC();

public:
	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );
	void	SetTimer( float timer );
	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	void	Detonate( void );
	
	void	CombatThink( void );
	void	TetherThink( void );

protected:

	int			m_nHooksShot;
	CSprite		*m_pGlow;
};

extern CBaseGrenade *HopWireOld_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer );

#endif // GRENADE_HOPWIRE_H
