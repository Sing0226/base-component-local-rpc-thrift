#ifndef __RIPC_QC_SERVICE_SRC_DSPSSERVER_H__
#define __RIPC_QC_SERVICE_SRC_DSPSSERVER_H__

#include <QObject>
#include <QString>
#include <QTimer>
#include "ripcqcservice.h"

class RIpcQcServicesHost;
class RRequestRunTestTask;
class RThreadPool;
class RThreadPoolCallBack;

class RIpcQcServer : public QObject
{
	Q_OBJECT
public:
	static RIpcQcServer& getInstance();

public:
	void serve();
	
public:
	void onAuthorStatusChanged();
	void onLoginResult(const std::string& reqId, int loginStatus);
	void onRunAuthorInfoTaskFinish(const std::string& reqId, const ripcqc::AuthorInfo& authorInfo);

private:
	RIpcQcServer(QObject *parent = NULL);
	~RIpcQcServer();

	Q_DISABLE_COPY(RIpcQcServer)

private slots:
	void onTimerNotifyAuthorChanged();

protected:
	// local ipc test
	bool _isLocalIpcTest(QString& authorInfo);

private:
	RIpcQcServicesHost* m_serviceHost;
	RRequestRunTestTask* m_pReqQcTask;
	QTimer* m_pNotifyAuthorChangedTimer;

	RThreadPool* m_pThreadPool;
	RThreadPoolCallBack* m_pThreadPoolCallback;
};

#endif // __RIPC_QC_SERVICE_SRC_DSPSSERVER_H__
