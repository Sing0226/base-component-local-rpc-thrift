#include "stdafx.h"
#include "ripcqcserviceshostprivate.h"
#include "ripcqcserviceshost.h"
#include "gen-cpp/ripcqcservices_constants.h"
#include "gen-cpp/ripcqcservices.h"
#include <thrift/transport/TBufferTransports.h>
#include <thrift/async/TAsyncProtocolProcessor.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/async/TAsyncChannel.h>

namespace
{
typedef ::apache::thrift::protocol::TCompactProtocolFactory TxCompactProtocolFactory;
}

class RIPCQcServer : public RLocalServer
{
public:
	RIPCQcServer(const QString& serverName, RIpcQcServicesHost* parent)
		: RLocalServer(serverName, parent)
		, m_host(parent)
	{}

protected:
	virtual ::std::shared_ptr<RIPCServerConnection> newConnection(
		quintptr socketDescriptor) override
	{
		return m_host->incomingConnection(socketDescriptor);
	}

private:
	RIpcQcServicesHost* m_host;
};


RIpcQcServicesCobClientWrap::RIpcQcServicesCobClientWrap(
	::std::shared_ptr<::apache::thrift::async::TAsyncChannel> channel,
	apache::thrift::protocol::TProtocolFactory* protocolFactory)
	: rs::qc::ripcqcservicesCobClient(channel, protocolFactory)
{
}

void RIpcQcServicesCobClientWrap::notifyAuthorizeStatusChanged(
	::std::function<void(RIpcQcServicesCobClientWrap* client)> cob,
	const std::string& authId)
{
	send_notifyAuthorizeStatusChanged(authId);
	getChannel()->sendMessage(::std::bind(cob, this), otrans_.get());
}

void RIpcQcServicesCobClientWrap::send_notifyAuthorizeStatusChanged(const std::string& authId)
{
	int32_t cseqid = 0;
	oprot_->writeMessageBegin("notifyAuthorizeStatusChanged", ::apache::thrift::protocol::T_CALL, cseqid);

	rs::qc::ripcqcservices_notifyAuthorizeStatusChanged_pargs args;
	args.authId = &authId;
	args.write(oprot_);

	oprot_->writeMessageEnd();
	oprot_->getTransport()->writeEnd();
	oprot_->getTransport()->flush();
}


class RIpcQcServicesHandler : public rs::qc::ripcqcservicesCobSvIf
{
public:
	RIpcQcServicesHandler(RIpcQcServicesHost* host, quintptr desc,
		::std::shared_ptr<apache::thrift::protocol::TProtocolFactory> protocolFactory);
	~RIpcQcServicesHandler();

	// to client
	virtual void notifyAuthorizeStatusChanged(::std::function<void()> cob, const std::string& authId) override;

	// to server
	virtual void queryAuthorStatus(::std::function<void(rs::qc::ResAuthorStatus const& _return)> cob,
		const std::string& reqId) override;
	virtual void userLogin(::std::function<void(int32_t const& _return)> cob, 
		const std::string& userName, const std::string& userPwd) override;

private:
	RIpcQcServicesHost* m_processHost;
	quintptr m_desc;
};

//////////////////////////////////////////////////////////////////////////
// handler 虚函数实现
void RIpcQcServicesHandler::notifyAuthorizeStatusChanged(::std::function<void()> cob,
	const std::string& authId)
{
	RSLOG_DEBUG << "RIpcQcServicesHandler, notifyAuthorizeStatusChanged";
}
void RIpcQcServicesHandler::queryAuthorStatus(::std::function<void(rs::qc::ResAuthorStatus const& _return)> cob,
	const std::string& reqId)
{
	RSLOG_DEBUG << "RIpcQcServicesHandler, queryAuthorStatus";
	RIpcQcResAuthorStatusCob* handler = new RIpcQcResAuthorStatusCob(cob);
	m_processHost->reqAuthorStatus(reqId, handler);
}
void RIpcQcServicesHandler::userLogin(::std::function<void(int32_t const& _return)> cob,
	const std::string& userName, const std::string& userPwd)
{
	RSLOG_DEBUG << "RIpcQcServicesHandler::userLogin";
	RIpcQcLoginCob* handler = new RIpcQcLoginCob(cob);
	m_processHost->reqUserLogin(userName, userPwd, handler);
}

//////////////////////////////////////////////////////////////////////////
// RIpcQcServicesHostPrivate
RIpcQcServicesHandler::RIpcQcServicesHandler(
	RIpcQcServicesHost* host, quintptr desc,
	::std::shared_ptr<apache::thrift::protocol::TProtocolFactory> protocolFactory)
	: m_processHost(host)
	, m_desc(desc)
{
}

RIpcQcServicesHandler::~RIpcQcServicesHandler()
{
}

RIpcQcServicesHostPrivate::RIpcQcServicesHostPrivate(
	RIpcQcServicesHost* q)
	: QObject(nullptr)
	, m_q(q)
	, m_pServer(nullptr)
{
	RSLOG_DEBUG << "RIpcQcServicesHostPrivate::RIpcQcServicesHostPrivate";
	initServer();
}

RIpcQcServicesHostPrivate::~RIpcQcServicesHostPrivate()
{
	RSLOG_DEBUG << "RIpcQcServicesHostPrivate::~RIpcQcServicesHostPrivate";
	if (m_pServer)
	{
		m_pServer->stop();
		delete m_pServer;
		m_pServer = nullptr;
	}
}

std::shared_ptr<RIpcQcServicesCobClientWrap>
	RIpcQcServicesHostPrivate::cobClient(quintptr desc)
{
	if (!m_connections[desc])
	{
		return std::shared_ptr<RIpcQcServicesCobClientWrap>();
	}
	if (!m_protocalFactorys[desc])
	{
		return std::shared_ptr<RIpcQcServicesCobClientWrap>();
	}
	if (!m_clients[desc])
	{
		::std::shared_ptr<RIPCServerToClientChannel> channel(
			new RIPCServerToClientChannel(m_connections[desc].get()));
		m_clients[desc].reset(
			new RIpcQcServicesCobClientWrap(channel, m_protocalFactorys[desc].get()));
	}
	return m_clients[desc];
}

::std::shared_ptr<RIPCServerConnection>
	RIpcQcServicesHostPrivate::incomingConnection(quintptr desc)
{
	RSLOG_DEBUG << "RIpcQcServicesHostPrivate::incomingConnection";
	if (!m_protocolProcessors[desc])
	{
		m_protocalFactorys[desc].reset(new TxCompactProtocolFactory);

		::std::shared_ptr<RIpcQcServicesHandler> handler(
			new RIpcQcServicesHandler(m_q, desc, m_protocalFactorys[desc]));
		::std::shared_ptr<apache::thrift::async::TAsyncProcessor> processor(
			new rs::qc::ripcqcservicesAsyncProcessor(handler));

		m_protocolProcessors[desc].reset(
			new apache::thrift::async::TAsyncProtocolProcessor(processor, m_protocalFactorys[desc]));
	}
	m_connections[desc].reset(new RIPCServerConnection(m_protocolProcessors[desc], desc));
	cobClient(desc);
	connect(m_connections[desc].get(), SIGNAL(connectionFinished()), this,
		SLOT(onConnectionFinished()));
	return m_connections[desc];
}

void RIpcQcServicesHostPrivate::onConnectionFinished()
{
	QObject* sender = QObject::sender();
	RIPCServerConnection* connection = qobject_cast<RIPCServerConnection*>(sender);
	if (connection)
	{
		connectionFinished(connection->getSocketDescriptor());
	}
}

void RIpcQcServicesHostPrivate::initServer()
{
	RSLOG_DEBUG << "RIpcQcServicesHostPrivate::initServer()";
	if (!m_pServer)
		m_pServer = new RIPCQcServer(QC_IPC_SERVER_NAME, m_q);

	if (m_pServer->isListening())
		return;
	m_pServer->serve(QC_IPC_SERVER_NAME);
}

void RIpcQcServicesHostPrivate::connectionFinished(quintptr desc)
{
	RSLOG_DEBUG << "RIpcQcServicesHostPrivate::connectionFinished";
	auto iter = m_connections.find(desc);
	if (iter != m_connections.cend())
	{
		m_connections.erase(desc);
		m_clients.erase(desc);
		m_protocolProcessors.erase(desc);
		m_protocalFactorys.erase(desc);
	}
}

std::map<quintptr, std::shared_ptr<RIpcQcServicesCobClientWrap>>& 
	RIpcQcServicesHostPrivate::getClients()
{
	return m_clients;
}