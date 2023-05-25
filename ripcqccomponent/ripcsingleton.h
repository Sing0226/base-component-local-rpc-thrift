/**************************************************************************

Copyright:Rayshape.com

Author: Simone Shi

Date: 2023-01-12

Version: 1.0.0

Description: 单例模板封装

**************************************************************************/


#ifndef  __RS_IPC_SINGLETON_H__
#define  __RS_IPC_SINGLETON_H__

#include "stdafx.h"
#include <QMutexLocker>
#include <QMutex>
#include <QAtomicPointer>

template <class ParentT, class ChildT> class RIPCSingleton
{
public:
	static ParentT* instance()
	{
		QMutexLocker locker(s_pMutex);
		if (s_pParentT == nullptr)
		{
			s_pParentT = new ChildT();
		}
		return s_pParentT;
	}

	static void destroy()
	{
		// ParentT *tins = s_instance.fetchAndStoreOrdered(0);// Atomic fetch-and-store
		QMutexLocker locker(s_pMutex);
		if (s_pParentT) 
		{
			delete s_pParentT;
			s_pParentT = nullptr;
		}
	}

	static QMutex* s_pMutex;

protected:
	RIPCSingleton(){}
	~RIPCSingleton() {}

private:
	RIPCSingleton (const RIPCSingleton& );
	RIPCSingleton& operator= (const RIPCSingleton& );

private:
	static ParentT* s_pParentT;
};

template <class ParentT, class ChildT> QMutex* RIPCSingleton<ParentT, ChildT>::s_pMutex = nullptr;
template <class ParentT, class ChildT> ParentT* RIPCSingleton<ParentT, ChildT>::s_pParentT  = nullptr;

#endif
