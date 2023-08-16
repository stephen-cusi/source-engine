#ifndef NETWORKVAR_OTHER_H
#define NETWORKVAR_OTHER_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"

template< class Type, class Changer >
class CNetworkVarBaseTimeScaled
{
public:
	inline CNetworkVarBaseTimeScaled()
	{
		NetworkVarConstruct(m_Value);
	}

	template< class C >
	const Type& operator=(const C& val)
	{
		return Set((const Type)val);
	}

	template< class C >
	const Type& operator=(const CNetworkVarBaseTimeScaled< C, Changer >& val)
	{
		return Set((const Type)val.m_Value);
	}

	const Type& Set(const Type& val)
	{
		if (memcmp(&m_Value, &val, sizeof(Type)))
		{
			NetworkStateChanged();
			m_Value = (val-gpGlobals->curtime) * Changer::ScaleConvar() + gpGlobals->curtime;
		}
		return m_Value;
	}

	Type& GetForModify()
	{
		NetworkStateChanged();
		return m_Value;
	}

	template< class C >
	const Type& operator+=(const C& val)
	{
		return Set(m_Value + (const Type)val);
	}

	template< class C >
	const Type& operator-=(const C& val)
	{
		return Set(m_Value - (const Type)val);
	}

	template< class C >
	const Type& operator/=(const C& val)
	{
		return Set(m_Value / (const Type)val);
	}

	template< class C >
	const Type& operator*=(const C& val)
	{
		return Set(m_Value * (const Type)val);
	}

	template< class C >
	const Type& operator^=(const C& val)
	{
		return Set(m_Value ^ (const Type)val);
	}

	template< class C >
	const Type& operator|=(const C& val)
	{
		return Set(m_Value | (const Type)val);
	}

	const Type& operator++()
	{
		return (*this += 1);
	}

	Type operator--()
	{
		return (*this -= 1);
	}

	Type operator++(int) // postfix version..
	{
		Type val = m_Value;
		(*this += 1);
		return val;
	}

	Type operator--(int) // postfix version..
	{
		Type val = m_Value;
		(*this -= 1);
		return val;
	}

	// For some reason the compiler only generates type conversion warnings for this operator when used like 
	// CNetworkVarBase<unsigned char> = 0x1
	// (it warns about converting from an int to an unsigned char).
	template< class C >
	const Type& operator&=(const C& val)
	{
		return Set(m_Value & (const Type)val);
	}

	operator const Type& () const
	{
		return m_Value;
	}

	const Type& Get() const
	{
		return m_Value;
	}

	const Type* operator->() const
	{
		return &m_Value;
	}

	Type m_Value;

protected:
	inline void NetworkStateChanged()
	{
		Changer::NetworkStateChanged(this);
	}
};


// Use this macro to define a network variable.
#define CNetworkVarTimeScaled( type, name, convar ) \
	NETWORK_VAR_START( type, name ) \
	NETWORK_VAR_TIME_SCALED_END( type, name, CNetworkVarBaseTimeScaled, NetworkStateChanged, convar )

#define NETWORK_VAR_TIME_SCALED_END( type, name, base, stateChangedFn, conVar ) \
	public: \
		static inline void NetworkStateChanged( void *ptr ) \
		{ \
			CHECK_USENETWORKVARS ((ThisClass*)(((char*)ptr) - MyOffsetOf(ThisClass,name)))->stateChangedFn( ptr ); \
		} \
		static float ScaleConvar() \
		{ \
			static const ConVar *var = g_pCVar->FindVar( #conVar ); \
			return var->GetFloat(); \
		} \
	}; \
	base< type, NetworkVar_##name > name;


#endif