#ifndef _RIPCSERVERTOCLIENTCHANNEL_H_
#define _RIPCSERVERTOCLIENTCHANNEL_H_

#include <boost/shared_ptr.hpp>
#include <thrift/async/TAsyncChannel.h>
#include <QObject>
#include <QPointer>

#include "ripc.h"

namespace apache{ namespace thrift{ namespace transport{
class TMemoryBuffer;
}}}

namespace apache{ namespace thrift{ namespace async{
class TAsyncProtocolProcessor;
}}}

typedef ::std::shared_ptr<apache::thrift::async::TAsyncProtocolProcessor> ProtocolProcessorPointer;

class RIPCServerConnectionPrivate;
class RIPCServerConnection : public QObject
{
	Q_OBJECT
public:
	RIPCServerConnection(
		ProtocolProcessorPointer protocolProcessor, quintptr socketDescriptor);
	~RIPCServerConnection();
	bool good() const;
	bool error() const;

public slots:
	void sendMessage(
		::std::shared_ptr<apache::thrift::transport::TMemoryBuffer> obuf, quint32 id);
	void initialize();
	void terminate();
	void sendBuffer(const QByteArray buffer, quint32 cid);
	quintptr getSocketDescriptor();

signals:
	void connectionFinished();
	void receivedBuffer(QByteArray buffer, quint32 cid);

private:
	friend class RIPCServerConnectionPrivate;
	RIPCServerConnectionPrivate* m_d;
};

class RIPCServerToClientChannelPrivate;
class RIPCServerToClientChannel
	: public QObject, public apache::thrift::async::TAsyncChannel
{
	Q_OBJECT
public:
	RIPCServerToClientChannel(RIPCServerConnection *connection);
	~RIPCServerToClientChannel();
	bool good() const override;
	bool error() const override;
	bool timedOut() const override;
	void sendMessage(
		const VoidCallback &cob,
		apache::thrift::transport::TMemoryBuffer *message) override;
	void recvMessage(
		const VoidCallback &cob,
		apache::thrift::transport::TMemoryBuffer *message) override;
	void sendAndRecvMessage(
		const VoidCallback& cob,
		apache::thrift::transport::TMemoryBuffer* sendBuf,
		apache::thrift::transport::TMemoryBuffer* recvBuf) override;

public slots:
	void onReceivedBuffer(QByteArray buffer, quint32 cid);

signals:
	void connectionFinished();

private:
	friend class RIPCServerToClientChannelPrivate;
	RIPCServerToClientChannelPrivate* m_d;
};

class RIPCServerConnectionHolder {
public:
	RIPCServerConnectionHolder();
	virtual ~RIPCServerConnectionHolder();
	void setConnection(::std::shared_ptr<RIPCServerConnection> connection);

protected:
	::std::shared_ptr<RIPCServerConnection> m_spConnection;
};

#endif // KIPCSERVERTOCLIENTCHANNEL_H_
