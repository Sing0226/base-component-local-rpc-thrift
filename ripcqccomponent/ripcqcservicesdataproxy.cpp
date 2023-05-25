#include "ripcqcservicesdataproxy.h"

#define SAFEDELETE(obj) if (obj) {delete obj ; obj = NULL;}

#define CLEARHANDLERMAP(handler_name) {														\
	QMutexLocker handler_name##Locker(m_p##handler_name##Mutex);							\
	Map##handler_name##Cob::iterator it##handler_name = m_map##handler_name##Cob.begin();	\
	for (; it##handler_name != m_map##handler_name##Cob.end(); ++it##handler_name)			\
	{																						\
		if (it##handler_name->second)														\
		{																					\
			delete it##handler_name->second;												\
			it##handler_name->second = nullptr;													\
		}																					\
	}																						\
	m_map##handler_name##Cob.clear();														\
}

#define CLEARLOGHANDLERMAP(handler_name) {													\
	QMutexLocker handler_name##Locker(m_p##handler_name##Mutex);							\
	MapUploadLogCob::iterator it##handler_name = m_map##handler_name##Cob.begin();			\
	for (; it##handler_name != m_map##handler_name##Cob.end(); ++it##handler_name)			\
	{																						\
		if (it##handler_name->second)														\
		{																					\
			delete it##handler_name->second;												\
			it##handler_name->second = nullptr;												\
		}																					\
	}																						\
	m_map##handler_name##Cob.clear();														\
}

RIpcServicesDataProxy::RIpcServicesDataProxy(QObject* parent)
	: QObject(parent)
	, m_pAuthorStatus(new rs::qc::ResAuthorStatus())
{
	m_pAuthorStatusMutex = new QMutex;
	m_pLoginInfoMutex = new QMutex;
}

RIpcServicesDataProxy::~RIpcServicesDataProxy()
{
	CLEARHANDLERMAP(AuthorStatus);
	CLEARHANDLERMAP(LoginInfo);

	SAFEDELETE(m_pAuthorStatus);
	SAFEDELETE(m_pAuthorStatusMutex);
	SAFEDELETE(m_pLoginInfoMutex);
}

const rs::qc::ResAuthorStatus* RIpcServicesDataProxy::getAuthorStatus()
{
	return m_pAuthorStatus;
}

RIpcQcResAuthorStatusCob* RIpcServicesDataProxy::getAuthorInfoCob(const std::string& reqId)
{
	RIpcQcResAuthorStatusCob* ret = nullptr;
	MapAuthorStatusCob::iterator it = m_mapAuthorStatusCob.find(reqId);
	if (it != m_mapAuthorStatusCob.end())
	{
		QMutexLocker policyLocker(m_pAuthorStatusMutex);
		ret = it->second;
		m_mapAuthorStatusCob.erase(it);
	}

	return ret;
}

RIpcQcLoginCob* RIpcServicesDataProxy::getLoginInfoCob(const std::string& reqId)
{
	RIpcQcLoginCob* ret = nullptr;
	MapLoginInfoCob::iterator it = m_mapLoginInfoCob.find(reqId);
	if (it != m_mapLoginInfoCob.end())
	{
		QMutexLocker policyLocker(m_pLoginInfoMutex);
		ret = it->second;
		m_mapLoginInfoCob.erase(it);
	}

	return ret;
}