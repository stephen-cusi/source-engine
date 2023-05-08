#include "cbase.h"
#include "instant_trigger.h"
#include "map_parser.h"

LINK_ENTITY_TO_CLASS(instant_trig, CInstantTrig)

BEGIN_DATADESC(CInstantTrig)
DEFINE_KEYFIELD(m_iszLabel, FIELD_STRING, "label"),
DEFINE_KEYFIELD(m_iszRandomSpawnLabel, FIELD_STRING, "randomspawnlabel"),
DEFINE_KEYFIELD(m_nRadius, FIELD_INTEGER, "radius"),
DEFINE_KEYFIELD(m_iszTouchName, FIELD_STRING, "touchname"),
DEFINE_KEYFIELD(m_flTimer, FIELD_FLOAT, "timer"),
DEFINE_KEYFIELD(m_iszIsLived, FIELD_STRING, "islived"),
DEFINE_KEYFIELD(m_nGroup, FIELD_INTEGER, "group"),
DEFINE_KEYFIELD(m_nRemoveGroup, FIELD_INTEGER, "removegroup"),
DEFINE_KEYFIELD(m_bNoClear, FIELD_BOOLEAN, "noclear"),
END_DATADESC()

void CInstantTrig::Spawn(void)
{
	if (m_flTimer > 0)
		SetNextThink(gpGlobals->curtime + m_flTimer);
	else if (m_iszIsLived.ToCStr() || (m_nRadius > 0))
		SetNextThink(gpGlobals->curtime + 1.0);
}

void CInstantTrig::Think(void)
{
	if (m_flTimer > 0) //We are using the timer
	{
		Trigger();
	}
	else if (m_nRadius > 0)
	{
		CBaseEntity *pEntity = nullptr;
		if (m_iszTouchName.ToCStr())
			pEntity = gEntList.FindEntityByName(NULL, m_iszTouchName.ToCStr());
		if (!pEntity)
			pEntity = UTIL_GetLocalPlayer();

		if (pEntity)
		{
			if ((pEntity->GetAbsOrigin() - GetAbsOrigin()).Length() < m_nRadius)
				Trigger();
		}
	}
	else if (m_iszIsLived.ToCStr())
	{
		bool ShouldTrigger = true;

		CBaseEntity *pEntity = gEntList.FindEntityByClassname(NULL, m_iszIsLived.ToCStr());
		if (pEntity)
			ShouldTrigger = false;
		pEntity = gEntList.FindEntityByName(NULL, m_iszIsLived.ToCStr());
		if (pEntity)
			ShouldTrigger = false;

		if (ShouldTrigger)
			Trigger();
	}
	SetNextThink(gpGlobals->curtime + 1.0); //Not Triggered Yet?, Keep Thinking
}

void CInstantTrig::Trigger()
{
	char* buffer = strdup(m_iszLabel.ToCStr());
	char* labels = strtok(buffer, ":");
	while (labels != NULL)
	{
		char labelname[FILENAME_MAX];
		Q_snprintf(labelname, sizeof(labelname), "entities:%s", labels);
		CMapScriptParser *parser = GetMapScriptParser();
		if (parser) {
			KeyValues* m_pLabel = parser->m_pMapScript->FindKey(labelname);
			if (m_pLabel)
				parser->ParseEntities(m_pLabel);
		}
		labels = strtok(NULL, ":");
	}
	free(buffer);

	if (!FStrEq(m_iszRandomSpawnLabel.ToCStr(), ""))
	{
		buffer = strdup(m_iszRandomSpawnLabel.ToCStr());
		labels = strtok(buffer, ":");
		while (labels != NULL)
		{
			char labelname[FILENAME_MAX];
			Q_snprintf(labelname, sizeof(labelname), "randomspawn:%s", labels);
			CMapScriptParser *parser = GetMapScriptParser();
			if (parser) {
				KeyValues* m_pLabel = parser->m_pMapScript->FindKey(labelname);
				if (m_pLabel)
					parser->ParseRandomEntities(m_pLabel);
			}
			labels = strtok(NULL, ":");
		}
	}
	free(buffer);

	CInstantTrig *pEntity = (CInstantTrig *)gEntList.FindEntityByClassname(NULL, "instant_trig");
	if (pEntity && (pEntity->m_nGroup == m_nRemoveGroup))
		UTIL_Remove(pEntity);

	if (!m_bNoClear)
		Remove();
}
