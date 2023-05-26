#include "ripcqcserver.h"
#include "ripcqcserviceshost.h"
#include "requestrunqctask.h"
#include "ripcqcservice.h"
#include <QApplication>
#include <QFileInfo>
#include "threadpool/rthreadpool.h"
#include "threadpool/rthreadpoolcallback.h"
#include "rslogger_declare.h"
#include "rslog.h"
#include "rslogging.h"
#include <QTextCodec>
#include <ctime>
#include <random>

//////////////////////////////////////////////////////////////////////////
//
#define MAX_INTERVAL_USER_ACCOUNT 30*1000  // 10分钟
#define START_TIMER_USER_ACCOUNT 2000  // 2秒

#define MAX_INTERVAL_MS_USER_CFG 30*60*1000		// 30分钟
#define START_TIMER_USER_CFG 10*1000		// 10秒

#define MAX_INTERVAL_SCAN_POLICY 30*60*1000  // 30分钟
#define START_TIMER_SCAN_POLICY 30*1000  // 30秒

#define TEST_LOCAL_IPC_QC_AUTHOR_INFO "/setting.txt"

RIpcQcServer::RIpcQcServer(QObject *parent /*= NULL*/)
	: QObject(parent)
	, m_serviceHost(NULL)
	, m_pNotifyAuthorChangedTimer(nullptr)
{
	m_pThreadPoolCallback = new RThreadPoolCallBack;
	m_pThreadPool = new RThreadPool(m_pThreadPoolCallback);
	m_pReqQcTask = new RRequestRunTestTask(m_pThreadPool);
	m_serviceHost = new RIpcQcServicesHost(this, m_pReqQcTask);	
}

RIpcQcServer::~RIpcQcServer()
{
	if (m_pNotifyAuthorChangedTimer)
	{
		if (m_pNotifyAuthorChangedTimer->isActive())
		{
			m_pNotifyAuthorChangedTimer->stop();
		}

		delete m_pNotifyAuthorChangedTimer;
		m_pNotifyAuthorChangedTimer = nullptr;
	}

	RSLOG_DEBUG << "start stop thread pool";
	m_pThreadPool->stop();
	RSLOG_DEBUG << "end stop thread pool";

	if (m_serviceHost)
	{
		delete m_serviceHost;
		m_serviceHost = nullptr;
	}

	if (m_pReqQcTask)
	{
		delete m_pReqQcTask;
		m_pReqQcTask = nullptr;
	}
	
	if (m_pThreadPool)
	{
		delete m_pThreadPool;
		m_pThreadPool = nullptr;
	}

	if (m_pThreadPoolCallback)
	{
		delete m_pThreadPoolCallback;
		m_pThreadPoolCallback = nullptr;
	}
}

void RIpcQcServer::onAuthorStatusChanged()
{
	ripcqc::AuthorInfo authorInfo;
	int errCode = ripcqc::RIpcQcErrorNoError;
	QString errMsg;
	std::default_random_engine e;
	std::uniform_int_distribution<int> u(1, 1000); // 左闭右闭区间
	e.seed(time(0));
	int num = u(e);
	if (u(e) % 3)
	{
		errMsg = QString::fromLocal8Bit("license 剩余天数:  %1").arg(num);
		authorInfo.authorStatus = true;
	}
	else
	{
		errMsg = QString::fromLocal8Bit("系统未授权，请联系厂商提供授权信息!");
		authorInfo.authorStatus = false;
	}

	authorInfo.authorInfo = QString::fromLocal8Bit("{\"errCode\":%1, \"errMsg\": \"%2\"}")
		.arg(errCode).arg(errMsg).toLocal8Bit().data();

	if (m_serviceHost)
	{
		// 通知用户账号变化
		m_serviceHost->onServerAuthorStatus(authorInfo);
	}
}


RIpcQcServer& RIpcQcServer::getInstance()
{
	static RIpcQcServer s_instance;

	return s_instance;
}

void RIpcQcServer::serve()
{
	RSLOG_DEBUG << "begin start servet!";
	m_pThreadPool->start(5, 10);
	// test local
	m_pNotifyAuthorChangedTimer = new QTimer(this);
    connect(m_pNotifyAuthorChangedTimer, SIGNAL(timeout()), this,SLOT(onTimerNotifyAuthorChanged()));
	m_pNotifyAuthorChangedTimer->start(START_TIMER_USER_ACCOUNT);	
	QString fileName = QApplication::applicationDirPath() + TEST_LOCAL_IPC_QC_AUTHOR_INFO;
	QFileInfo fileInfo(fileName);
	if (fileInfo.exists())
	{
        RSLOG_DEBUG << fileName.toLocal8Bit().data() << "author info exist!";
		
	}
    else
    {
		RSLOG_DEBUG <<  fileName.toLocal8Bit().data() <<"\n There is no author info!";
    }
	onAuthorStatusChanged();
}

bool RIpcQcServer::_isLocalIpcTest(QString& author)
{
	QString fileName = QApplication::applicationDirPath() + TEST_LOCAL_IPC_QC_AUTHOR_INFO;
	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly))
	{
		QByteArray allBytes = file.readAll();
		author = QString(allBytes).simplified().remove("-");
        file.close();
        return true;
	}
	return false;
}

void RIpcQcServer::onTimerNotifyAuthorChanged()
{
	ripcqc::AuthorInfo authorInfo;
	int errCode = ripcqc::RIpcQcErrorNoError;
	QString errMsg;
	QString lic_file_path = QApplication::applicationDirPath() + TEST_LOCAL_IPC_QC_AUTHOR_INFO;
	QFileInfo file_lic(lic_file_path);
	
	std::default_random_engine e;
	std::uniform_int_distribution<int> u(1, 1000); // 左闭右闭区间
	e.seed(time(0));
	int num = u(e);
	if (u(e) % 3)
	{
		errMsg = QString::fromLocal8Bit("license 剩余天数:  %1").arg(num);
		authorInfo.authorStatus = true;
	}
	else
	{
		errMsg = QString::fromLocal8Bit("系统未授权，请联系厂商提供授权信息!");
		authorInfo.authorStatus = false;
	}

	authorInfo.authorInfo = QString::fromLocal8Bit("{\"errCode\":%1, \"errMsg\": \"%2\"}")
		.arg(errCode).arg(errMsg).toLocal8Bit().data();

	if (m_serviceHost)
	{
		m_serviceHost->onServerAuthorStatus(authorInfo);
	}
	m_pNotifyAuthorChangedTimer->setInterval(MAX_INTERVAL_USER_ACCOUNT);
}

void RIpcQcServer::onLoginResult(const std::string& reqId, int loginStatus)
{
	if (m_serviceHost)
	{
		m_serviceHost->onLoginResult(reqId, loginStatus);
	}
}

void RIpcQcServer::onRunAuthorInfoTaskFinish(const std::string& reqId, const ripcqc::AuthorInfo& authorInfo)
{
	if (m_serviceHost)
	{
		m_serviceHost->onAuthorStatus(reqId, authorInfo);
	}
}
