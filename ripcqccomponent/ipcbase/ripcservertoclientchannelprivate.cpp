#include "stdafx.h"
#include "ripcservertoclientchannelprivate.h"

#include <QBuffer>
#include <QtCore> // required by common/common.h

#include "ripcconstant.h"

#ifdef Q_OS_WIN
#include "rlocalsocket.h"
#else
#include "rqtlocalsocket.h"
#endif

#include "ripcservertoclientchannel.h"

using apache::thrift::transport::TMemoryBuffer;
using apache::thrift::async::TAsyncChannel;

namespace
{
const QByteArray byeMessage(RIPC_BYE_MESSAGE);
const QByteArray frameFlag(RIPC_FRAME_FLAGS);
const static quint32 s_clientModeVersion = RIPC_CLIENT_MODE_VERSION;
}

RIPCServerConnectionPrivate::RIPCServerConnectionPrivate(
	RIPCServerConnection *q, ProtocolProcessorPointer protocolProcessor,
	quintptr socketDescriptor )
	: IRLocalSocketNotify()
	, m_q(q)
	, m_pSocket(nullptr)
	, m_protocolProcessor(protocolProcessor)
	, m_socketDescriptor(socketDescriptor)
	, m_serverSequanceId(0)
	, m_cobs()
	, m_memoryBuffers()
	, m_currentSequanceId(1)
	, m_thread()
	, m_buffer()
{

}

RIPCServerConnectionPrivate::~RIPCServerConnectionPrivate()
{
	if (m_pSocket)
	{
		terminate();
	}
}

void RIPCServerConnectionPrivate::initialize()
{
	m_pSocket = new RLocalSocket(this);
	m_pSocket->init();
#ifdef Q_OS_WIN
	m_pSocket->setSocketDescriptor((HANDLE)m_socketDescriptor);
	m_pSocket->read();
#else
	m_pSocket->setSocketDescriptor(m_socketDescriptor);
#endif
}

void RIPCServerConnectionPrivate::terminate()
{
	m_pSocket->disconnectFromServer();
	m_pSocket->term();
	delete m_pSocket;
	m_pSocket = nullptr;
}

void RIPCServerConnectionPrivate::onConnected()
{
}

void RIPCServerConnectionPrivate::onDisconnected()
{
}

void RIPCServerConnectionPrivate::onStateChanged(LocalSocketState socketState)
{
	(void) socketState;
}

void RIPCServerConnectionPrivate::onError(LocalSocketError socketError)
{
	if (socketError == ConnectionError || socketError == UnknownSocketError || socketError == PeerClosedError)
	{
		QString msg = QString("KIPCServerConnectionPrivate on Error %1").arg(socketError);
		RSLOG_ERROR << msg.toLocal8Bit().data();
		emit m_q->connectionFinished();
	}
}

void RIPCServerConnectionPrivate::onRecvData(char* bytes, unsigned int len)
{
	m_buffer += QByteArray(bytes, len);
	process();
}

void RIPCServerConnectionPrivate::process()
{
	if (m_buffer.size() > (1 << 25)) // 32 MB
	{
		RSLOG_DEBUG << "Client sent too large data";
		emit m_q->connectionFinished();
		return;
	}

	while (m_buffer.size() > sizeof(RIPCMessageHeader))
	{
		QBuffer qBuffer(&m_buffer); // To use QIODevice interface
		qBuffer.open(QIODevice::ReadOnly);
		RIPCMessageHeader messageHeader;
		if (!qBuffer.read((char *)&messageHeader, sizeof(RIPCMessageHeader)))
		{
			RSLOG_DEBUG << "Fail to read m_magicFlags";
			emit m_q->connectionFinished();
			return;
		}
		if (memcmp(messageHeader.m_magicFlags, frameFlag.constData(), 4))
		{
			QString msg = QString("Wrong m_magicFlags") + QString::fromLatin1(messageHeader.m_magicFlags, 4);
			RSLOG_ERROR << msg.toLocal8Bit().data();
			emit m_q->connectionFinished();
			return;
		}
		if (messageHeader.m_length < sizeof(RIPCMessageHeader))
		{
			RSLOG_DEBUG << "Bad Length";
			emit m_q->connectionFinished();
			return;
		}
		// There are no enough data
		if (qBuffer.size() < messageHeader.m_length)
		{
			return;
		}

		// For future, m_version may enable extra bytes to read.
		qint64 payloadLength = messageHeader.m_length - sizeof(RIPCMessageHeader);
		QByteArray payload = qBuffer.read(payloadLength);
		if (payloadLength == byeMessage.count() && messageHeader.m_cid == RIPC_ID_ZERO)
		{
			if (payload == byeMessage)
			{
				RSLOG_DEBUG << "KIPCServerConnectionPrivate: Peer requests to close";
				emit m_q->connectionFinished();
				return;
			}
		}

		if (messageHeader.m_version == s_clientModeVersion)
		{
			emit m_q->receivedBuffer(payload, messageHeader.m_cid);
		}
		else
		{
			::std::shared_ptr<TMemoryBuffer> ibuf;
			::std::shared_ptr<TMemoryBuffer> obuf;
			ibuf.reset(new TMemoryBuffer(reinterpret_cast<uint8_t*>(payload.data()),
										 payload.count(), TMemoryBuffer::COPY));
			obuf.reset(new TMemoryBuffer());
			try
			{
				m_protocolProcessor->process([=](bool success)
				{
					this->sendMessage(obuf, messageHeader.m_cid);
				}
				, ibuf, obuf);
			}
			catch (std::exception& e)
			{
				QString msg = QString("KIPCServerConnectionPrivate: %s").arg(e.what());
				RSLOG_ERROR << msg.toLocal8Bit().data();
				printf("%s", e.what());
			}
			
		}
		qBuffer.close();
		m_buffer.remove(0, messageHeader.m_length);
	}
}

void RIPCServerConnectionPrivate::sendMessage(::std::shared_ptr<TMemoryBuffer> obuf, quint32 id)
{
	if (!m_pSocket || m_pSocket->state() != ConnectedState)
	{
		RSLOG_DEBUG << "sendMessage disconnected";
		return;
	}

	uint8_t *buf = nullptr;
	uint32_t sz = 0;
	obuf->getBuffer(&buf, &sz);
	RIPCMessageHeader messageHeader;
	messageHeader.m_length = sz + sizeof(RIPCMessageHeader);
	messageHeader.m_version = 0;
	messageHeader.m_cid = id;
	messageHeader.m_sid = m_serverSequanceId++;
	
	char *message = new char[messageHeader.m_length];
	memcpy(message, frameFlag.constData(), 4);
	memcpy(message + 4, reinterpret_cast<const char*>(&messageHeader.m_length), 4);
	memcpy(message + 8, reinterpret_cast<const char*>(&messageHeader.m_version), 4);
	memcpy(message + 12, reinterpret_cast<const char*>(&messageHeader.m_cid), 4);
	memcpy(message + 16, reinterpret_cast<const char*>(&messageHeader.m_sid), 4);
	memcpy(message + 20, reinterpret_cast<const char*>(buf), sz);

	int sliceCount = messageHeader.m_length / RsLocalSocketBufferLength;
	int rest = messageHeader.m_length % RsLocalSocketBufferLength;
	bool done = true;
	for (int i = 0; i < sliceCount && done; ++i)
	{
		done = (m_pSocket->write(message + i * RsLocalSocketBufferLength,
			RsLocalSocketBufferLength) != -1);
	}

	if (done && rest)
	{
		done = (m_pSocket->write(message + sliceCount * RsLocalSocketBufferLength,
								rest) != -1);
	}

	delete [] message;
	if (!done)
	{
		RSLOG_DEBUG << "KIPCServerConnectionPrivate::sendMessage: Not enqueued";
		emit m_q->connectionFinished();
	}
}

void RIPCServerConnectionPrivate::sendBuffer(const QByteArray buffer, quint32 cid)
{
	if (!m_pSocket || m_pSocket->state() != ConnectedState)
	{
		RSLOG_DEBUG << "sendMessage disconnected";
		return;
	}

	int sliceCount = buffer.count() / RsLocalSocketBufferLength;
	int rest = buffer.count() % RsLocalSocketBufferLength;
	bool done = true;
	for (int i = 0; i < sliceCount && done; ++i)
	{
		done = (m_pSocket->write(buffer.constData() + i * RsLocalSocketBufferLength,
			RsLocalSocketBufferLength) != -1);
	}

	if (done && rest)
	{
		done = (m_pSocket->write(buffer.constData() + sliceCount * RsLocalSocketBufferLength,
								rest) != -1);
	}
	if (!done)
	{
		RSLOG_DEBUG << "KIPCServerConnectionPrivate::sendMessage: Not enqueued";
		auto it = m_cobs.find(cid);
		if (it != m_cobs.end())
		{
			it->second();
			m_cobs.erase(it);
			m_memoryBuffers.erase(m_memoryBuffers.find(cid));
		}
		emit m_q->connectionFinished();
	}
}

quintptr RIPCServerConnectionPrivate::getSocketDescriptor()
{
	return m_socketDescriptor;
}

// ------------------
// RIPCServerToClientChannelPrivate
// ------------------

RIPCServerToClientChannelPrivate::RIPCServerToClientChannelPrivate(
	RIPCServerToClientChannel *q, RIPCServerConnection *channel)
	: m_q(q)
	, m_connection(channel)
	, m_cobs()
	, m_memoryBuffers()
	, m_currentSequanceId(1)
{
	QObject::connect(channel, SIGNAL(receivedBuffer(QByteArray, quint32)),
		q, SLOT(onReceivedBuffer(QByteArray, quint32)));
}

RIPCServerToClientChannelPrivate::~RIPCServerToClientChannelPrivate()
{

}

