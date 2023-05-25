#ifndef __RIPC_QC_SERVICES_HOST_PRIVATE_H__
#define __RIPC_QC_SERVICES_HOST_PRIVATE_H__

#include "ipcbase/ripc.h"
#include "gen-cpp/ripcqcservices.h"

class RIpcQcServicesCobClientWrap
	: public QObject
	, public rs::qc::ripcqcservicesCobClient
{
	Q_OBJECT
public:
	RIpcQcServicesCobClientWrap(
		::std::shared_ptr<::apache::thrift::async::TAsyncChannel> channel,
		apache::thrift::protocol::TProtocolFactory* protocolFactory);


	void notifyAuthorizeStatusChanged(::std::function<void(RIpcQcServicesCobClientWrap* client)> cob, const std::string& authId);
	void send_notifyAuthorizeStatusChanged(const std::string& authId);
	// void recv_notifyAuthorizeStatusChanged();

public:
	::std::shared_ptr< ::apache::thrift::async::TAsyncChannel> getChannel() {
		return channel_;
	}
};

class RIpcQcServicesHost;
class RIpcQcServicesHostPrivate
	: public QObject
{
	Q_OBJECT
public:
	RIpcQcServicesHostPrivate(RIpcQcServicesHost* q);
	~RIpcQcServicesHostPrivate();

	std::shared_ptr<RIpcQcServicesCobClientWrap> cobClient(quintptr desc);
	::std::shared_ptr<RIPCServerConnection> incomingConnection(quintptr socketDescriptor);

public:
	std::map<quintptr, std::shared_ptr<RIpcQcServicesCobClientWrap>>& getClients();

private slots:
	void onConnectionFinished();

private:
	void initServer();
	void connectionFinished(quintptr desc);

private:
	RIpcQcServicesHost* m_q;
	RLocalServer* m_pServer;

	std::map<quintptr, ::std::shared_ptr<apache::thrift::async::TAsyncProtocolProcessor>>
		m_protocolProcessors;
	std::map<quintptr, ::std::shared_ptr<apache::thrift::protocol::TProtocolFactory>>
		m_protocalFactorys;

	std::map<quintptr, std::shared_ptr<RIpcQcServicesCobClientWrap>> m_clients;
	std::map<quintptr, ::std::shared_ptr<RIPCServerConnection>> m_connections;
};


#endif // __RIPC_QC_SERVICES_HOST_PRIVATE_H__
