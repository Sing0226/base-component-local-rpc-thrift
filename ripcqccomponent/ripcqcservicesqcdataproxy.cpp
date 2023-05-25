#include "ripcqcservicesqcdataproxy.h"

RIpcServiceQcDataProxy::RIpcServiceQcDataProxy(QObject* parent)
    : RIpcServicesDataProxy(parent)
{

}

RIpcServiceQcDataProxy::~RIpcServiceQcDataProxy()
{
}

void RIpcServiceQcDataProxy::setServiceAuthorisationStatus(const rs::qc::ResAuthorStatus& authorStatus)
{
	m_pAuthorStatus->authorStatus	= authorStatus.authorStatus;
	m_pAuthorStatus->strAutorInfo = authorStatus.strAutorInfo;
}

int RIpcServiceQcDataProxy::setServiceAuthorInfoHandlerMap(const std::string& reqId, RIpcQcResAuthorStatusCob* handler)
{
	int ret = -1;
	if (handler == nullptr)
		return ret;
	QMutexLocker authorInfoLocker(m_pAuthorStatusMutex);
	std::pair<MapAuthorStatusCob::iterator, bool> resPair = m_mapAuthorStatusCob.insert(std::pair<std::string, RIpcQcResAuthorStatusCob*>(reqId, handler));
	if (resPair.second == false)
	{
		RSLOG_DEBUG << QString("RIpcServiceQcDataProxy::setServiceAuthorInfoHandlerMap reqId(%1) is exist!").arg(QString::fromLocal8Bit(reqId.c_str())).toLocal8Bit().data();
	}
	else
	{
		ret = 0;
	}

	return ret;
}

int RIpcServiceQcDataProxy::setServiceLoginInfoHandlerMap(const std::string& reqId, RIpcQcLoginCob* handler)
{
	int ret = -1;
	if (handler == nullptr)
		return ret;
	QMutexLocker userLoginLocker(m_pLoginInfoMutex);
	std::pair<MapLoginInfoCob::iterator, bool> resPair = m_mapLoginInfoCob.insert(std::pair<std::string, RIpcQcLoginCob*>(reqId, handler));
	if (resPair.second == false)
	{
		RSLOG_DEBUG << QString("RIpcServiceQcDataProxy::setServiceLoginInfoHandlerMap reqId(%1) is exist!").arg(QString::fromLocal8Bit(reqId.c_str())).toLocal8Bit().data();
	}
	else
	{
		ret = 0;
	}

	return ret;
}


