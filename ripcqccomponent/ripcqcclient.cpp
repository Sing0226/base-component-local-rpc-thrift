#include "stdafx.h"
#include "ripcqcclient.h"
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TTransportException.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/async/TAsyncProtocolProcessor.h>
#include "ripcqcservicescobclient.h"

class RIpcQcServicesClientHandler : public rs::qc::ripcqcservicesCobSvIf
{
public:
	explicit RIpcQcServicesClientHandler(RIpcQcClient* pClient);
	~RIpcQcServicesClientHandler()
	{
		m_listClientNotify.clear();

		if (m_pListLockerMutex)
		{
			delete m_pListLockerMutex;
			m_pListLockerMutex = nullptr;
		}
	}

	// to server
	virtual void notifyAuthorizeStatusChanged(::std::function<void()> cob, const std::string& authId) override;

	// to client
	virtual void queryAuthorStatus(::std::function<void(rs::qc::ResAuthorStatus const& _return)> cob, 
		const std::string& reqId) override;
	virtual void userLogin(::std::function<void(int32_t const& _return)> cob, 
		const std::string& userName, const std::string& userPwd) override;

public:
	// 处理通知消息使用 add by Simone 2022-02-15
	void resgisterIpcClientNotify(rs_client_ipc::IRsIpcClientNotify* clientNotify);
	void unresgisterIpcClientNotify(rs_client_ipc::IRsIpcClientNotify* clientNotify);

private:
	RIpcQcClient* m_pClient;

	std::list<rs_client_ipc::IRsIpcClientNotify*> m_listClientNotify;
	QMutex* m_pListLockerMutex;
};

RIpcQcServicesClientHandler::RIpcQcServicesClientHandler(RIpcQcClient* pClient)
	: m_pClient(pClient)
	, m_pListLockerMutex(new QMutex())
{

}

void RIpcQcServicesClientHandler::resgisterIpcClientNotify(rs_client_ipc::IRsIpcClientNotify* clientNotify)
{
	if (clientNotify)
	{
		m_pListLockerMutex->lock();
		m_listClientNotify.push_back(clientNotify);
		m_pListLockerMutex->unlock();
	}
}

void RIpcQcServicesClientHandler::unresgisterIpcClientNotify(rs_client_ipc::IRsIpcClientNotify* clientNotify)
{
	std::list<rs_client_ipc::IRsIpcClientNotify*>::iterator it = m_listClientNotify.begin();
	while (it != m_listClientNotify.end())
	{
		if (*it == clientNotify)
		{
			m_pListLockerMutex->lock();
			it = m_listClientNotify.erase(it);
			m_pListLockerMutex->unlock();
			break;
		}
		else
		{
			++it;
		}
	}
}

void RIpcQcServicesClientHandler::notifyAuthorizeStatusChanged(::std::function<void()> cob, 
	const std::string& authId)
{
	RSLOG_DEBUG << "RIpcQcServicesClientHandler notifyAuthorizeStatusChanged";

	std::list<rs_client_ipc::IRsIpcClientNotify*>::iterator it = m_listClientNotify.begin();
	for (; it != m_listClientNotify.end(); ++it)
	{
		rs_client_ipc::IRsIpcClientNotify* pNotify = *it;
		pNotify->notifyAuthorInfo(authId);
	}
}
void RIpcQcServicesClientHandler::queryAuthorStatus(::std::function<void(rs::qc::ResAuthorStatus const& _return)> cob, 
	const std::string& reqId)
{
	RSLOG_DEBUG << "RIpcQcServicesClientHandler queryAutorStatus";
}

void RIpcQcServicesClientHandler::userLogin(::std::function<void(int32_t const& _return)> cob,
	const std::string& userName, const std::string& userPwd)
{
	RSLOG_DEBUG << "RIpcQcServicesClientHandler::userLogin";
}

RIpcQcClient::RIpcQcClient(QObject* parent)
	: QObject(parent)
	, m_spCobClient()
	, m_spHandler(new Handler(this))
	, m_spIpcChannel()
	, m_bInitialized(false)
	, m_bConnected(false)
	, m_pListLockerMutex(new QMutex())

{
	// initCobClient();
}

RIpcQcClient::~RIpcQcClient()
{
	m_listClientNotify.clear();

	if (m_pListLockerMutex)
	{
		delete m_pListLockerMutex;
		m_pListLockerMutex = nullptr;
	}
}

void RIpcQcClient::initCobClient()
{
	RSLOG_DEBUG << "RIpcQcClient initCobClient";
	if (m_bInitialized)
		return;

	if (!m_spCobClient)
	{
		::std::shared_ptr<apache::thrift::protocol::TCompactProtocolFactory> protocolFactory(
			new apache::thrift::protocol::TCompactProtocolFactory);

		m_spSrvClHandler.reset(new RIpcQcServicesClientHandler(this));
		::std::shared_ptr<apache::thrift::async::TAsyncProcessor>
			asyncProcessor(new rs::qc::ripcqcservicesAsyncProcessor(m_spSrvClHandler));
		std::shared_ptr<TxAsyncProtocolProcessor> processor(
			new TxAsyncProtocolProcessor(asyncProcessor, protocolFactory));
		m_spIpcChannel.reset(new RIPCClientChannel(m_spHandler));
		m_spIpcChannel->setServiceProcessor(processor);;
		m_spCobClient.reset(new RIpcQcServicesCobClient(m_spIpcChannel, protocolFactory.get()));
		m_bInitialized = true;
	}
	
}

std::shared_ptr<RIpcQcServicesCobClient> RIpcQcClient::cobClient()
{
	return m_spCobClient;
}

void RIpcQcClient::connectToServer(const QString& serverName)
{
	if (m_bConnected)
		return;

	m_serverName = serverName;
	if (m_spIpcChannel)
		m_spIpcChannel->connectToServer(m_serverName.toStdWString());
}

void RIpcQcClient::resgisterIpcClientNotify(rs_client_ipc::IRsIpcClientNotify* clientNotify)
{
	if (clientNotify)
	{
		bool bExist = false;
		std::list<rs_client_ipc::IRsIpcClientNotify*>::iterator it = m_listClientNotify.begin();
		while (it != m_listClientNotify.end())
		{
			if (*it == clientNotify)
			{
				bExist = true;
				break;
			}
			it++;
		}

		if (!bExist)
		{
			m_pListLockerMutex->lock();
			m_listClientNotify.push_back(clientNotify);
			m_pListLockerMutex->unlock();
			m_spSrvClHandler->resgisterIpcClientNotify(clientNotify);
		}
	}
}

void RIpcQcClient::unresgisterIpcClientNotify(rs_client_ipc::IRsIpcClientNotify* clientNotify)
{
	std::list<rs_client_ipc::IRsIpcClientNotify*>::iterator it = m_listClientNotify.begin();
	while (it != m_listClientNotify.end())
	{
		if (*it == clientNotify)
		{
			m_pListLockerMutex->lock();
			it = m_listClientNotify.erase(it);
			m_pListLockerMutex->unlock();

			m_spSrvClHandler->unresgisterIpcClientNotify(clientNotify);
			break;
		}
		else
		{
			++it;
		}
	}
}


void RIpcQcClient::connectedSucceed()
{
	m_bConnected = true;
	std::list<rs_client_ipc::IRsIpcClientNotify*>::iterator it = m_listClientNotify.begin();
	for (; it != m_listClientNotify.end(); ++it)
	{
		rs_client_ipc::IRsIpcClientNotify* pNotify = *it;
		pNotify->notifyConnected();
	}
}

void RIpcQcClient::disconnected()
{
	m_bConnected = false;
	m_bInitialized = false;
	std::list<rs_client_ipc::IRsIpcClientNotify*>::iterator it = m_listClientNotify.begin();
	for (; it != m_listClientNotify.end(); ++it)
	{
		rs_client_ipc::IRsIpcClientNotify* pNotify = *it;
		pNotify->notifyDisconnected();
	}
}

void RIpcQcClient::connectingError(LocalSocketError errorCode)
{
	m_bConnected	= false;
	m_bInitialized	= false;
	std::list<rs_client_ipc::IRsIpcClientNotify*>::iterator it = m_listClientNotify.begin();
	for (; it != m_listClientNotify.end(); ++it)
	{
		rs_client_ipc::IRsIpcClientNotify* pNotify = *it;
        pNotify->notifyConnectErr();
	}
}

QString RIpcQcClient::getServerName() const
{
	return m_serverName;
}


