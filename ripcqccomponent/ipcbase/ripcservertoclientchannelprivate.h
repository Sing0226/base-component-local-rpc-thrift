#ifndef _RIPCSERVERTOCLIENTCHANNELPRIVATE_H_
#define _RIPCSERVERTOCLIENTCHANNELPRIVATE_H_

#include <boost/shared_ptr.hpp>
#include <thrift/async/TAsyncChannel.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/async/TAsyncProtocolProcessor.h>
#include <QObject>
#include <QPointer>
#include <QThread>

#include "rlocalsocketdef.h"

class RLocalSocket;
class RIPCServerConnection;
class RIPCServerToClientChannel;

typedef ::std::shared_ptr<apache::thrift::async::TAsyncProtocolProcessor> ProtocolProcessorPointer;

class RIPCServerConnectionPrivate : public IRLocalSocketNotify
{
public:
	RIPCServerConnectionPrivate(
		RIPCServerConnection *q, ProtocolProcessorPointer protocolProcessor,
		quintptr socketDescriptor);

	~RIPCServerConnectionPrivate();
	void initialize();
	void terminate();
	void sendMessage(
		::std::shared_ptr<apache::thrift::transport::TMemoryBuffer> obuf, quint32 id);
	void sendBuffer(const QByteArray buffer, quint32 cid);
	quintptr getSocketDescriptor();
	
private:
	void onConnected() override;
	void onDisconnected() override;
	void onStateChanged(LocalSocketState socketState) override;
	void onError(LocalSocketError) override;
	void onRecvData(char*, unsigned int) override;
private:
	void process();

private:
	friend class RIPCServerConnection;
	RIPCServerConnection *m_q;
	RLocalSocket *m_pSocket;
	ProtocolProcessorPointer m_protocolProcessor;
	quintptr m_socketDescriptor;
	quint32 m_serverSequanceId;
	std::map<uint32_t, apache::thrift::async::TAsyncChannel::VoidCallback> m_cobs;
	std::map<uint32_t, apache::thrift::transport::TMemoryBuffer *> m_memoryBuffers;
	quint32 m_currentSequanceId;
	QThread m_thread;
	QByteArray m_buffer;
};

class RIPCServerToClientChannelPrivate
{
public:
	RIPCServerToClientChannelPrivate(
		RIPCServerToClientChannel *q, RIPCServerConnection *connection);
	~RIPCServerToClientChannelPrivate();

private:
	friend class RIPCServerToClientChannel;
	RIPCServerToClientChannel *m_q;
	QPointer<RIPCServerConnection> m_connection;
	std::map<uint32_t, apache::thrift::async::TAsyncChannel::VoidCallback> m_cobs;
	std::map<uint32_t, apache::thrift::transport::TMemoryBuffer *> m_memoryBuffers;
	quint32 m_currentSequanceId;
};

#endif // RIPCSERVERTOCLIENTCHANNEL_H_
