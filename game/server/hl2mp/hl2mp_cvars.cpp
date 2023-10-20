//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:  
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2mp_cvars.h"



// Ready restart
ConVar mp_readyrestart(
							"mp_readyrestart",
							"0",
							FCVAR_GAMEDLL,
							"If non-zero, game will restart once each player gives the ready signal" );

// Ready signal
ConVar mp_ready_signal(
							"mp_ready_signal",
							"ready",
							FCVAR_GAMEDLL,
							"Text that each player must speak for the match to begin" );

// Skip defaults
ConVar mp_skipdefaults(
                            "mp_skipdefaults",
                            "0",
                            FCVAR_GAMEDLL,
                            "If non-zero, game will not give player default weapons and ammo" );

// Allow pickup items with +use
ConVar mp_allowpickup(
                            "mp_allowpickup",
                            "1",
                            FCVAR_GAMEDLL,
                            "If non-zero, game will allow pickup items" );