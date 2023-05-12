#include "cbase.h"
#include "tneRenderTargets.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"
#include "viewrender.h"

//extern CViewRender g_DefaultViewRender;

ITexture* CTNERenderTargets::CreateScopeTexture(IMaterialSystem* pMaterialSystem)
{
	CViewRender* viewrender = CViewRender::GetMainView();
	//float dummy;
	int size = 512;
	if (viewrender)
	{
		//	viewrender->GetScopeVars(dummy, size);
	}

	//	DevMsg("Creating Scope Render Target: _rt_Scope\n");
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Scope",
		size, size, RT_SIZE_OFFSCREEN,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR);

}

ITexture* CTNERenderTargets::CreateBloomTexture(IMaterialSystem* pMaterialSystem)
{
	CViewRender* viewrender = CViewRender::GetMainView();
	//float dummy;
	int size = 512;
	if (viewrender)
	{
		//	viewrender->GetScopeVars(dummy, size);
	}

	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Bloom",
		size, size, RT_SIZE_OFFSCREEN,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR);

}

//-----------------------------------------------------------------------------
// Purpose: Called by the engine in material system init and shutdown.
//			Clients should override this in their inherited version, but the base
//			is to init all standard render targets for use.
// Input  : pMaterialSystem - the engine's material system (our singleton is not yet inited at the time this is called)
//			pHardwareConfig - the user hardware config, useful for conditional render target setup
//-----------------------------------------------------------------------------
void CTNERenderTargets::InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig)
{
	m_ScopeTexture.Init(CreateScopeTexture(pMaterialSystem));
	m_BloomTexture.Init(CreateBloomTexture(pMaterialSystem));
	// Water effects & camera from the base class (standard HL2 targets) 
	BaseClass::InitClientRenderTargets(pMaterialSystem, pHardwareConfig);
}

//-----------------------------------------------------------------------------
// Purpose: Shut down each CTextureReference we created in InitClientRenderTargets.
//			Called by the engine in material system shutdown.
// Input  :  - 
//-----------------------------------------------------------------------------
void CTNERenderTargets::ShutdownClientRenderTargets()
{
	m_ScopeTexture.Shutdown();
	m_BloomTexture.Shutdown();

	// Clean up standard HL2 RTs (camera and water) 
	BaseClass::ShutdownClientRenderTargets();
}

//add the interface!
static CTNERenderTargets g_TNERenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CTNERenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, g_TNERenderTargets);
CTNERenderTargets* TNERenderTargets = &g_TNERenderTargets;