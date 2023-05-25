#include "stdafx.h"
#include "ripcservertoclientchannel.h"

#include <QBuffer>
#include <QtCore> // required by common/common.h

#include "ripcconstant.h"

#ifdef Q_OS_WIN
#include "rlocalsocket.h"
#else
#include "kqtlocalsocket.h"
#endif

#include "ripcservertoclientchannelprivate.h"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::async::TAsyncChannel;

namespace
{
const QByteArray byeMessage(RIPC_BYE_MESSAGE);
const QByteArray frameFlag(RIPC_FRAME_FLAGS);
const static quint32 s_clientModeVersion = RIPC_CLIENT_MODE_VERSION;
}

RIPCServerConnection::RIPCServerConnection(
	ProtocolProcessorPointer protocolProcessor, quintptr socketDescriptor)
	: QObject(nullptr)
	, m_d(new RIPCServerConnectionPrivate(this, protocolProcessor, socketDescriptor))
{
	this->moveToThread(&m_d->m_thread);
	m_d->m_thread.start();
	QString msg = QString("RIPCServerConnection: thread is running %1").arg(m_d->m_thread.isRunning());
	RSLOG_DEBUG << msg.toLocal8Bit().data();
}

RIPCServerConnection::~RIPCServerConnection()
{
	m_d->m_thread.quit();
	m_d->m_thread.wait();
	delete m_d;
	RSLOG_DEBUG << "KIPCServerConnection::~KIPCServerConnection";
}


void RIPCServerConnection::sendMessage(::std::shared_ptr<TMemoryBuffer> obuf, quint32 id)
{
	m_d->sendMessage(obuf, id);
}


void RIPCServerConnection::initialize()
{
	m_d->initialize();
}

void RIPCServerConnection::terminate()
{
	m_d->terminate();
}

bool RIPCServerConnection::good() const
{
	return !(!m_d->m_pSocket || m_d->m_pSocket->state() != ConnectedState);
}

bool RIPCServerConnection::error() const
{
	return !m_d->m_pSocket || m_d->m_pSocket->state() != ConnectedState;
}

void RIPCServerConnection::sendBuffer(const QByteArray buffer, quint32 cid)
{
	m_d->sendBuffer(buffer, cid);
}

quintptr RIPCServerConnection::getSocketDescriptor()
{
	return m_d->getSocketDescriptor();
}

// ------------------
// RIPCServerToClientChannel
// ------------------
RIPCServerToClientChannel::RIPCServerToClientChannel(RIPCServerConnection *channel)
	: TAsyncChannel()
	, m_d(new RIPCServerToClientChannelPrivate(this, channel))
{
	connect(channel, SIGNAL(connectionFinished()),
			this, SIGNAL(connectionFinished()));
}

RIPCServerToClientChannel::~RIPCServerToClientChannel()
{
	delete m_d;
}
bool RIPCServerToClientChannel::good() const
{
	return m_d && m_d->m_connection && m_d->m_connection->good();
}

bool RIPCServerToClientChannel::error() const
{
	return !m_d || !m_d->m_connection || m_d->m_connection->error();
}

bool RIPCServerToClientChannel::timedOut() const
{
	return false;
}

void RIPCServerToClientChannel::sendMessage(const VoidCallback &cob, TMemoryBuffer *message)
{
	uint8_t *obuf = nullptr;
	uint32_t sz = 0;
	message->getBuffer(&obuf, &sz);
	RIPCMessageHeader messageHeader;
	messageHeader.m_length = sz + sizeof(RIPCMessageHeader);
	messageHeader.m_version = 0;
	messageHeader.m_cid = 0;
	messageHeader.m_sid = 0;
	char *messageBytes = new char[messageHeader.m_length];
	memcpy(messageBytes, frameFlag.constData(), 4);
	memcpy(messageBytes + 4, reinterpret_cast<const char*>(&messageHeader.m_length), 4);
	memcpy(messageBytes + 8, reinterpret_cast<const char*>(&messageHeader.m_version), 4);
	memcpy(messageBytes + 12, reinterpret_cast<const char*>(&messageHeader.m_cid), 4);
	memcpy(messageBytes + 16, reinterpret_cast<const char*>(&messageHeader.m_sid), 4);
	memcpy(messageBytes + 20, reinterpret_cast<const char*>(obuf), sz);
	message->resetBuffer(); // Cleanup for next request, see THRIFT-3496
	QMetaObject::invokeMethod(m_d->m_connection.data(),
							"sendBuffer",
							Q_ARG(QByteArray, QByteArray(messageBytes, messageHeader.m_length)),
							Q_ARG(quint32, messageHeader.m_cid));
}

void RIPCServerToClientChannel::recvMessage(const VoidCallback &cob, TMemoryBuffer *message)
{
	(void)(cob);
	(void)(message);
}

void RIPCServerToClientChannel::sendAndRecvMessage(const VoidCallback& cob,
									   TMemoryBuffer* sendBuf, TMemoryBuffer* recvBuf)
{
	if (!good())
	{
		RSLOG_DEBUG << "sendMessage disconnected";
		sendBuf->resetBuffer();  // Cleanup for next request, see THRIFT-3496
		recvBuf->resetBuffer();
		cob();
		return;
	}
	m_d->m_currentSequanceId++;
	if (m_d->m_currentSequanceId == RIPC_ID_ZERO)
	{
		m_d->m_currentSequanceId = 1;
	}
	m_d->m_cobs[m_d->m_currentSequanceId] = cob;
	m_d->m_memoryBuffers[m_d->m_currentSequanceId] = recvBuf;
	uint8_t *obuf = nullptr;
	uint32_t sz = 0;
	sendBuf->getBuffer(&obuf, &sz);
	RIPCMessageHeader messageHeader;
	messageHeader.m_length = sz + sizeof(RIPCMessageHeader);
	messageHeader.m_version = s_clientModeVersion;
	messageHeader.m_cid = m_d->m_currentSequanceId;
	messageHeader.m_sid = 0;
	char *messageBytes = new char[messageHeader.m_length];
	memcpy(messageBytes, frameFlag.constData(), 4);
	memcpy(messageBytes + 4, reinterpret_cast<const char*>(&messageHeader.m_length), 4);
	memcpy(messageBytes + 8, reinterpret_cast<const char*>(&messageHeader.m_version), 4);
	memcpy(messageBytes + 12, reinterpret_cast<const char*>(&messageHeader.m_cid), 4);
	memcpy(messageBytes + 16, reinterpret_cast<const char*>(&messageHeader.m_sid), 4);
	memcpy(messageBytes + 20, reinterpret_cast<const char*>(obuf), sz);
	sendBuf->resetBuffer(); // Cleanup for next request, see THRIFT-3496
	QMetaObject::invokeMethod(m_d->m_connection.data(),
		"sendBuffer", Q_ARG(QByteArray, QByteArray(messageBytes, messageHeader.m_length)),
		Q_ARG(quint32, m_d->m_currentSequanceId));
	delete [] messageBytes;
}

void RIPCServerToClientChannel::onReceivedBuffer(QByteArray buffer, quint32 cid)
{
	std::map<uint32_t, TAsyncChannel::VoidCallback>::iterator
		cob = m_d->m_cobs.find(cid);
	if (cob == m_d->m_cobs.end())
	{
		QString msg = QString("KIPCServerConnectionPrivate: no cob %1").arg(cid);
		RSLOG_DEBUG << msg.toLocal8Bit().data();
		return;
	}
	std::map<uint32_t, TMemoryBuffer *>::iterator mb = m_d->m_memoryBuffers.find(cid);
	if (mb == m_d->m_memoryBuffers.end())
	{
		RSLOG_DEBUG << "KIPCServerConnectionPrivate: no mb";
		return;
	}
	mb->second->resetBuffer(reinterpret_cast<uint8_t *>(buffer.data()),
							buffer.count(), TMemoryBuffer::COPY);
	try
	{
		cob->second();
	}
	catch (std::exception& e)
	{
		QString msg = QString("KIPCServerConnectionPrivate: ") + QString::fromStdString(e.what());
		RSLOG_ERROR << msg.toLocal8Bit().data();
	}
}

RIPCServerConnectionHolder::RIPCServerConnectionHolder()
{

}

RIPCServerConnectionHolder::~RIPCServerConnectionHolder()
{

}

void RIPCServerConnectionHolder::setConnection(
	::std::shared_ptr<RIPCServerConnection> connection)
{
	m_spConnection = connection;
}
