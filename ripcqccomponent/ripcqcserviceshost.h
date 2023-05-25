#ifndef __RIPC_QC_SERVICES_HOST_H__
#define __RIPC_QC_SERVICES_HOST_H__

#include "ipcbase/ripc.h"
#include "ripcqchandlercob.h"
#include "ripcqcservice.h"
#include <string>
#include <list>
#include <vector>

class RIpcQcServicesHostPrivate;
class RIpcQcServicesCobClientWrap;
class RIpcServicesDataProxy;
class RAbstractRequestRunQcTask;

namespace rs {
	namespace qc {
		class PlaneInfo;
		class PatientInfo;
	}
}


class RIPC_QC_EXPORT RIpcQcServicesHost
	: public QObject
{
	Q_OBJECT
public:
	explicit RIpcQcServicesHost(QObject* parent, RAbstractRequestRunQcTask* reqRunTask);
	~RIpcQcServicesHost();

	::std::shared_ptr<RIPCServerConnection> incomingConnection(quintptr desc);
	std::shared_ptr<RIpcQcServicesCobClientWrap> cobClient(quintptr desc);

public:
	void onServerAuthorStatus(const ripcqc::AuthorInfo& authorInfo);

	void reqAuthorStatus(const std::string& reqId, RIpcQcResAuthorStatusCob* handler);
	void reqUserLogin(const std::string& userName, const std::string& userPwd, RIpcQcLoginCob* handler);

	void onAuthorStatus(const std::string& reqId, const ripcqc::AuthorInfo& authorInfo);
	void onLoginResult(const std::string& reqId, int loginStatus);

public slots:
	void onNotifyAuthorStatusChanged(const QString& authorMsg);

signals:

	// to services
	void sigNotifyAuthorStatusChanged(const QString& authorMsg);

private:
	std::string _createUUID();

private:

	RIpcQcServicesHostPrivate* m_d;

	RIpcServicesDataProxy* m_pServicesDataProxy;

	RAbstractRequestRunQcTask* m_pReqRunTask;
	int m_bFirstNotifyAuthorInfo = false;
};

#endif //__RIPC_QC_SERVICES_HOST_H__
