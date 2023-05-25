#include "stdafx.h"
#include "rlocalserver.h"

#include <QtCore>
#include <QBuffer>
#include <QPointer>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/async/TAsyncProtocolProcessor.h>
#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/async/TAsyncChannel.h>

#include "ripcconstant.h"

#ifdef Q_OS_WIN
#include "rlocalsocket.h"
#else 
#include "kqtlocalsocket.h"
#endif

#include "ripcservertoclientchannel.h"
#ifndef Q_OS_WIN
#include "typedef.h"
#endif

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::async::TAsyncChannel;
using apache::thrift::async::TAsyncProcessor;
using apache::thrift::async::TAsyncProtocolProcessor;

namespace
{
typedef ::apache::thrift::protocol::TProtocolFactory TxProtocolFactory;
typedef ::apache::thrift::protocol::TCompactProtocolFactory TxCompactProtocolFactory;
static const uint32_t g_maxConnectionCount = 128;
}

class _QLocalServer
	: public QLocalServer
{
public:
	_QLocalServer(QObject* parent, RLocalServer* q)
		: QLocalServer(parent)
		, m_q(q)
	{}

protected:
	virtual void incomingConnection(quintptr socketDescriptor) override
	{
		m_q->incomingConnection(socketDescriptor);
	}

protected:
	RLocalServer* m_q;
};

struct RLocalServerPrivate
{
	RLocalServerPrivate(const QString& serverName, RLocalServer* q)
		: m_localServer(0, q)
		, m_serverName(serverName)
	{}
	_QLocalServer m_localServer;
	QString m_serverName;
	QList<::std::shared_ptr<RIPCServerConnection>> m_connections;
};

class RNotifyAsyncChannel : public TAsyncChannel
{
public:
	RNotifyAsyncChannel(RLocalServer *server) : m_server(server) {}
	~RNotifyAsyncChannel() {}
	bool good() const override
	{
		return true;
	}

	bool error() const override
	{
		return false;
	}
	
	bool timedOut() const override
	{
		return false;
	}

	void sendMessage(const VoidCallback &cob, TMemoryBuffer *message) override
	{
		if (!m_server)
		{
			message->resetBuffer(); // Cleanup for next request, see THRIFT-3496
			return;
		}
		uint8_t *obuf = nullptr;
		uint32_t sz = 0;
		message->getBuffer(&obuf, &sz);
		::std::shared_ptr<TMemoryBuffer> notifyBuf;
		notifyBuf.reset(new TMemoryBuffer(obuf, sz, TMemoryBuffer::COPY));
		message->resetBuffer(); // Cleanup for next request, see THRIFT-3496
		QMetaObject::invokeMethod(m_server, "broadcast", Qt::QueuedConnection,
								  Q_ARG(::std::shared_ptr<TMemoryBuffer>, notifyBuf));
	}

	void recvMessage(const VoidCallback &cob, TMemoryBuffer *message) override
	{
		Q_UNUSED(cob);
		Q_UNUSED(message);
	}
private:
	QPointer<RLocalServer> m_server;
};

	
RLocalServer::RLocalServer(const QString& serverName, QObject* parent)
	: QObject(parent), m_d(new RLocalServerPrivate(serverName, this))
{
	m_d->m_localServer.removeServer(m_d->m_serverName);
	qRegisterMetaType<::std::shared_ptr<TMemoryBuffer>>(
		"::std::shared_ptr<TMemoryBuffer>");
	m_d->m_connections.reserve(g_maxConnectionCount);
	m_d->m_localServer.setMaxPendingConnections(3);
}

RLocalServer::~RLocalServer()
{
	delete m_d;
	RSLOG_DEBUG << "RLocalServer is destroyed";
}

::std::shared_ptr<TAsyncChannel> RLocalServer::channel()
{
	return ::std::shared_ptr<TAsyncChannel>(new RNotifyAsyncChannel(this));
}

bool RLocalServer::isListening() const
{
	return m_d->m_localServer.isListening();
}

void RLocalServer::serve(const QString& serverName /*= QString()*/)
{
	if (!serverName.isEmpty())
		m_d->m_serverName = serverName;
	if (!m_d->m_localServer.listen(m_d->m_serverName))
	{
		RSLOG_ERROR << "RLocalServer can't listen";
	}
	else
	{
		RSLOG_DEBUG << "Start RLocalServer";
	}
}

void RLocalServer::stop()
{
	RSLOG_DEBUG << "RLocalServer is going to stop";
	m_d->m_localServer.close();

	m_d->m_connections.clear();
	RSLOG_DEBUG << "RLocalServer stops";
}

void RLocalServer::broadcast(::std::shared_ptr<TMemoryBuffer> obuf)
{
	RSLOG_DEBUG << "RLocalServer::broadcast";
	for (auto it = m_d->m_connections.begin(); it != m_d->m_connections.end(); ++it)
	{
		if (!QMetaObject::invokeMethod(it->get(), "sendMessage", Qt::QueuedConnection,
			Q_ARG(::std::shared_ptr<TMemoryBuffer>, obuf), Q_ARG(quint32, RIPC_ID_ZERO)))
		{
			RSLOG_ERROR << "RLocalServer: Fail to sendMessage";
		}
	}
}

void RLocalServer::incomingConnection(quintptr socketDescriptor)
{
	if (m_d->m_connections.size() > g_maxConnectionCount)
	{
		RSLOG_ERROR << "RLocalServer has many connections, reject new socket";
#ifdef Q_OS_WIN
		CloseHandle((HANDLE) socketDescriptor);
#else
		QLocalSocket socket;
		socket.setSocketDescriptor(socketDescriptor);
		socket.abort();
#endif
		return;
	}

	auto connection = newConnection(socketDescriptor);

	m_d->m_connections.append(connection);

	if (!connect(connection.get(), SIGNAL(connectionFinished()), this,
		SLOT(onConnectionFinished()), Qt::QueuedConnection))
	{
		RSLOG_ERROR << "RLocalServer: Fail to bind";
	}
	if (!QMetaObject::invokeMethod(connection.get(), "initialize", Qt::QueuedConnection))
	{
		RSLOG_ERROR << "RLocalServer: Fail to initialize";
	}
	QString msg = QString("RLocalServer: New connection %1").arg((quintptr)connection.get());
	RSLOG_DEBUG << msg.toLocal8Bit().data();
}

void RLocalServer::onConnectionFinished()
{
	RIPCServerConnection* channel = qobject_cast<RIPCServerConnection*>(sender());
	if (!channel)
	{
		return;
	}
	RSLOG_DEBUG << "RLocalServer: Connection finished";
	for (auto it = m_d->m_connections.begin(); it != m_d->m_connections.end(); ++it)
	{
		if (it->get() == channel)
		{
			m_d->m_connections.erase(it);
			break;
		}
	}
	QString msg = QString("RLocalServer: %1 active Connections").arg(m_d->m_connections.size());
	RSLOG_DEBUG << msg.toLocal8Bit().data();
}

