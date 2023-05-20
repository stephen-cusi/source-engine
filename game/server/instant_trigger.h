class CInstantTrig : public CLogicalEntity
{
public:
	DECLARE_CLASS(CInstantTrig, CLogicalEntity)
	DECLARE_DATADESC();

	void Spawn(void);
	void Think(void);
	void Trigger(void);

	string_t	m_iszLabel;
	string_t	m_iszRandomSpawnLabel;
	string_t	m_iszTouchName;
	string_t	m_iszIsLived;
	float	m_flTimer;
	int	m_nRadius;
	int		m_nGroup;
	int		m_nRemoveGroup;
	bool	m_bNoClear;
};