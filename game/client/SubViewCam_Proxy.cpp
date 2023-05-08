//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"
#include "viewrender.h"
#include <KeyValues.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// $sineVar : name of variable that controls the alpha level (float)
class CSubViewCamProxy : public CEntityMaterialProxy
{
public:
	CSubViewCamProxy();
	virtual ~CSubViewCamProxy();
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(C_BaseEntity *pC_BaseEntity);
	virtual IMaterial *GetMaterial();

private:
	char const *m_sBaseTexture;
	float m_flFOV;
	float m_nSize;
	IMaterialVar *m_BaseTextureVar;
	IMaterial *m_Overlay;
	IMaterial *m_Filter1;
	IMaterial *m_Filter2;
	float m_ScrollRate;
	float m_ScrollAngle;
	const char* m_sOverlayTexture;
};

extern CViewRender g_DefaultViewRender;

CSubViewCamProxy::CSubViewCamProxy()
{
	m_sBaseTexture = NULL;
	m_BaseTextureVar = NULL;
}

CSubViewCamProxy::~CSubViewCamProxy()
{
}


bool CSubViewCamProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	bool foundVar;
	m_sBaseTexture = pKeyValues->GetString("ReplaceTexture");
	m_flFOV = pKeyValues->GetFloat("FOV");
	m_nSize = pKeyValues->GetInt("Size");
	m_sOverlayTexture = pKeyValues->GetString("OverlayTexture");
	
	if(m_sOverlayTexture)
		m_Overlay = materials->FindMaterial(m_sOverlayTexture, TEXTURE_GROUP_OTHER);

	if(pKeyValues->GetString("Filter1"))
		m_Filter1 = materials->FindMaterial(pKeyValues->GetString("Filter1"), TEXTURE_GROUP_OTHER);

	if (pKeyValues->GetString("Filter2"))
		m_Filter2 = materials->FindMaterial(pKeyValues->GetString("Filter2"), TEXTURE_GROUP_OTHER);

	m_BaseTextureVar = pMaterial->FindVar(m_sBaseTexture, &foundVar, false);
	if (!foundVar)
		return false;

	return true;
}

void CSubViewCamProxy::OnBind(C_BaseEntity *pEnt)
{
	if (!m_BaseTextureVar)
	{
		return;
	}

	CViewRender *pViewRender = CViewRender::GetMainView();

	pViewRender->SetScopeVars(m_flFOV, m_nSize, m_Overlay, m_Filter1, m_Filter2);
	m_BaseTextureVar->SetTextureValue(GetScopeTexture());
}

IMaterial *CSubViewCamProxy::GetMaterial()
{
	if (!m_BaseTextureVar)
		return NULL;

	return m_BaseTextureVar->GetOwningMaterial();
}

EXPOSE_INTERFACE(CSubViewCamProxy, IMaterialProxy, "SubViewCam" IMATERIAL_PROXY_INTERFACE_VERSION);
