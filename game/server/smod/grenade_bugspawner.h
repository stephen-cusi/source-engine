//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Molotov grenades
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	GRENADEMOLOTOV_H
#define	GRENADEMOLOTOV_H

#include "basegrenade_shared.h"
#include "smoke_trail.h"

class CGrenadeBugSpawner : public CBaseGrenade
{
public:
	DECLARE_CLASS( CGrenadeBugSpawner, CBaseGrenade );

	virtual void	Spawn( void );
	virtual void	Precache( void );
	virtual void	Detonate( void );
	void			MolotovTouch( CBaseEntity *pOther );
	void			MolotovThink( void );

protected:

	// SmokeTrail		*m_pFireTrail;

	DECLARE_DATADESC();
};

#endif	//GRENADEMOLOTOV_H
