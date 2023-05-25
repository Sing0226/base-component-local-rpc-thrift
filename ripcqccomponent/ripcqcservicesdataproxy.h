#ifndef __R_IPC_QC_SERVICES_DATA_PROXY_H__
#define __R_IPC_QC_SERVICES_DATA_PROXY_H__

#include "stdafx.h"
#include "ipc_declare.h"
#include "gen-cpp/ripcqcservices.h"
#include <map>
#include "ripcqchandlercob.h"

typedef std::map<std::string, RIpcQcResAuthorStatusCob*> MapAuthorStatusCob;
typedef std::map<std::string, RIpcQcLoginCob*> MapLoginInfoCob;

class RIPC_QC_EXPORT RIpcServicesDataProxy : public QObject
{
	Q_OBJECT
public:
	RIpcServicesDataProxy(QObject* parent);
	virtual ~RIpcServicesDataProxy();

	virtual void setServiceAuthorisationStatus(const rs::qc::ResAuthorStatus& authorStatus) = 0;
	virtual int setServiceAuthorInfoHandlerMap(const std::string& reqId, RIpcQcResAuthorStatusCob* handler) = 0;
	virtual int setServiceLoginInfoHandlerMap(const std::string& reqId, RIpcQcLoginCob* handler) = 0;

public:
	const rs::qc::ResAuthorStatus* getAuthorStatus();
	RIpcQcResAuthorStatusCob* getAuthorInfoCob(const std::string& reqId);
	RIpcQcLoginCob* getLoginInfoCob(const std::string& reqId);

protected:
	rs::qc::ResAuthorStatus* m_pAuthorStatus;

	// std::string m_strScanPolicy;
	QMutex* m_pAuthorStatusMutex;
	MapAuthorStatusCob m_mapAuthorStatusCob;

	QMutex* m_pLoginInfoMutex;
	MapLoginInfoCob m_mapLoginInfoCob;

};

#endif