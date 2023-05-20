#include "cbase.h"

ConVar sparkshower("sparkshower", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED);

ConVar bullettimesim_disable_ai("bullettimesim_disable_ai", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "When enabled, NPC AI is turned off while bullet-time is active.");

ConVar gore_moregore("gore_moregore", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar gore_drawgoo("gore_drawgoo", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar gore_spreadwatersurface("gore_spreadwatersurface", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);

ConVar phys_npcdamagephysicsscale1("phys_npcdamagephysicsscale1", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar phys_npcdamagephysicsscale2("phys_npcdamagephysicsscale2", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);

ConVar cl_playermodel("cl_playermodel", "models/player/Gordon.mdl", FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar weapon_ironsightwalkspeed("weapon_ironsightwalkspeed", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);

ConVar kick_launchplayer("kick_launchplayer", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar kick_launchplayer_force("kick_launchplayer_force", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);

ConVar cl_freeaim("smod_ro_aimmode", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED, "enables free aim.");
ConVar cl_freeaim_ironsight("smod_ro_aimmode_ironsight", "1", FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar cl_freeaim_limit("smod_ro_aimmode_radius", "10", FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar cl_freeaim_mult("smod_ro_aimmode_sensetivity", "0.01", FCVAR_ARCHIVE | FCVAR_REPLICATED);

ConVar cl_debug_screeneffect("cl_debug_screeneffect", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED | FCVAR_DEVELOPMENTONLY);

ConVar r_increaseparticles("r_increaseparticles", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED);

ConVar smod_cod_healthregen("smod_cod_healthregen", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED);
ConVar smod_cod_healthregen_rate("smod_cod_healthregen_rate", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED);

ConVar r_shockeffect("r_shockeffect", "0", FCVAR_ARCHIVE | FCVAR_REPLICATED);