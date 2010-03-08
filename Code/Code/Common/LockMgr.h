#ifndef _LOCKMGR_H_
#define _LOCKMGR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _COMMON_H_
	#include "..\Common\Common.h"
#endif

class CCSWrapper
{
public:
	CCSWrapper();
	virtual ~CCSWrapper();
	// 
	// This function waits for ownership of the specified critical section object 
	// 
	void Enter();
	//
	// Releases ownership of the specified critical section object. 
	// 
	void Leave();
private:
	CRITICAL_SECTION m_cs;
	long m_nSpinCount;
};



template <class T>
class CLockMgr  
{
public:
	//
	// Constructor
	//
	CLockMgr(T& lockObject, BOOL bEnabled):
		m_rLockObject( lockObject ),
		m_bEnabled( bEnabled )
	{
		if ( m_bEnabled )
			m_rLockObject.Enter();
	}
	//
	// Destructor
	//
	virtual ~CLockMgr()
	{
		if ( m_bEnabled )
			m_rLockObject.Leave();
	}
private:
	T&   m_rLockObject;
	BOOL m_bEnabled;
};

#endif