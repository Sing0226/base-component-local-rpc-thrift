#include "stdafx.h"
#include "ripcclientchannel.h"

#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <thrift/transport/TBufferTransports.h>

#ifdef Q_OS_WIN
#include "rlocalsocket.h"
#else
#include "rqtlocalsocket.h"
#endif

#include "ripcconstant.h"


using apache::thrift::transport::TMemoryBuffer;

namespace
{
static const std::string s_byeMessage(RIPC_BYE_MESSAGE);
static const std::string s_thriftMessageHead(RIPC_FRAME_FLAGS);
static const std::string s_heartbeatHeader(RIPC_HEARTBEAT_HEADER);
const uint32_t s_idZero = RIPC_ID_ZERO;
const static uint32_t s_clientModeVersion = RIPC_CLIENT_MODE_VERSION;

class RIPCClientChannelDebug
{
public:
	RIPCClientChannelDebug()
	{
	}
	~RIPCClientChannelDebug()
	{
		std::wstring s = stream.str();
		QString msg = QString("RIPCClientChannel, s = %1").arg(QString::fromStdWString(s));
		RSLOG_DEBUG << msg.toLocal8Bit().data();
	}

	RIPCClientChannelDebug& operator<< (const wchar_t* t)
	{
		stream << std::wstring(t);
		stream << L" ";
		return *this;
	}

	RIPCClientChannelDebug& operator<< (const std::wstring& t)
	{
		stream << t;
		stream << L" ";
		return *this;
	}

	RIPCClientChannelDebug& operator<< (uint64_t t)
	{
		stream << t;
		stream << L" ";
		return *this;
	}
	std::wstringstream stream;
};
}

class RIPCBuffer
{
public:
	RIPCBuffer()
	{
		m_pos = 0;
		m_size = 0;
		m_capacity = 8192;
		m_d = new char[m_capacity];
		memset(m_d, 0, m_capacity);
	}
	~RIPCBuffer()
	{
		delete [] m_d;
	}
	void append(const char* data, uint32_t len)
	{
		if (m_capacity < m_size + len)
		{
			m_capacity = 4096 * (1 + (m_size + len) / 4096);
			char *d = new char[m_capacity];
			memset(d, 0, m_capacity);
			memcpy(d, m_d, m_size);
			delete [] m_d;
			m_d = d;
		}
		memcpy(m_d + m_size, data, len);
		m_size += len;
	}
	uint32_t size()
	{
		return m_size;
	}
	// NOTE 返回buffer的地址, 注意, 对data的引用前, 切勿进行修改操作
	bool read(char*& data, uint32_t len)
	{
		if (m_size - m_pos < len)
		{
			data = nullptr;
			return false;
		}
		data = m_d + m_pos;
		m_pos += len;
		return true;
	}
	void unread()
	{
		m_pos = 0;
	}
	void commit(uint32_t len)
	{
		m_pos = 0;
		if (len > m_size)
		{
			memset(m_d, 0, m_capacity);
			m_size = 0;
		}
		else
		{
			memmove(m_d, m_d + len, m_size - len);
			m_size -= len;
		}
	}

	void clear()
	{
		m_size = 0;
		m_pos = 0;
		memset(m_d, 0, m_capacity);
	}

	uint32_t dumpData(char*& data)
	{
		if (m_size - m_pos <= 0)
		{
			data = nullptr;
			return 0;
		}

		data = m_d + m_pos;
		return m_size;
	}

private:
	volatile uint32_t m_pos; // 当前未读数据首位置
	volatile uint32_t m_size; // 当前有效数据长度
	uint32_t m_capacity; // 预留空间长度
	char* m_d; // 数据
};

class RIPCClientChannelPrivate
{
public:
	explicit RIPCClientChannelPrivate()
	: m_socket(nullptr)
	, m_currentSequanceId(0)
	, m_serverSequanceId(0)
	{
	}
	~RIPCClientChannelPrivate() {}
	void sendMessage(::std::shared_ptr<TMemoryBuffer> message, uint32_t version, uint32_t cid);
	RLocalSocket* m_socket;
	int m_currentSequanceId;
	int m_serverSequanceId;
	std::weak_ptr<RChannelNotifyHandler> m_statusHandler;
	std::shared_ptr<TxAsyncProtocolProcessor> m_serviceProcessor;
	std::map<uint32_t, RIPCClientChannel::VoidCallback> m_cobs;
	std::map<uint32_t, TMemoryBuffer *> m_memoryBuffers;
	std::wstring m_serverName;
	RIPCBuffer m_buffer;
};

void RIPCClientChannelPrivate::sendMessage(::std::shared_ptr<TMemoryBuffer> message, uint32_t version, uint32_t cid)
{
	uint8_t *obuf = nullptr;
	uint32_t sz = 0;
	message->getBuffer(&obuf, &sz);
	RIPCMessageHeader messageHeader;
	messageHeader.m_length = sz + sizeof(RIPCMessageHeader);
	messageHeader.m_version = version;
	messageHeader.m_cid = cid;
	messageHeader.m_sid = 0;
	char *messageBytes = new char[messageHeader.m_length];
	memcpy(messageBytes, s_thriftMessageHead.c_str(), 4);
	memcpy(messageBytes + 4, reinterpret_cast<const char*>(&messageHeader.m_length), 4);
	memcpy(messageBytes + 8, reinterpret_cast<const char*>(&messageHeader.m_version), 4);
	memcpy(messageBytes + 12, reinterpret_cast<const char*>(&messageHeader.m_cid), 4);
	memcpy(messageBytes + 16, reinterpret_cast<const char*>(&messageHeader.m_sid), 4);
	memcpy(messageBytes + 20, reinterpret_cast<const char*>(obuf), sz);
	message->resetBuffer(); // Cleanup for next request, see THRIFT-3496

	int sliceCount = messageHeader.m_length / RsLocalSocketBufferLength;
	int rest = messageHeader.m_length % RsLocalSocketBufferLength;
	bool writtenToQueue = true;
	for (int i = 0; i < sliceCount && writtenToQueue; ++i)
	{
		writtenToQueue = (m_socket->write(messageBytes + i * RsLocalSocketBufferLength,
			RsLocalSocketBufferLength) != -1);
	}

	if (writtenToQueue && rest)
	{
		writtenToQueue = (m_socket->write(messageBytes + sliceCount * RsLocalSocketBufferLength,
											   rest) != -1);
	}

	delete [] messageBytes;
	if (!writtenToQueue)
	{
		RIPCClientChannelDebug() << L"Fail on sendMessage";
	}
}

RIPCClientChannel::RIPCClientChannel(std::shared_ptr<RChannelNotifyHandler> statusHandler)
	: m_d(new RIPCClientChannelPrivate())
{
	m_d->m_statusHandler = statusHandler;
}

RIPCClientChannel::~RIPCClientChannel()
{
	if (m_d->m_socket)
	{
		close();
		delete m_d->m_socket;
		m_d->m_socket = nullptr;
	}
	delete m_d;
}

void RIPCClientChannel::setServiceProcessor(
	std::shared_ptr<TxAsyncProtocolProcessor> serviceProcessor)
{
	m_d->m_serviceProcessor = serviceProcessor;
}

LocalSocketState RIPCClientChannel::state() const
{
	if (m_d->m_socket)
		return m_d->m_socket->state();
	return UnconnectedState;
}

bool RIPCClientChannel::good() const
{
	return m_d->m_socket && (m_d->m_socket->state() == ConnectedState
		||  m_d->m_socket->state() == ConnectingState);
}

bool RIPCClientChannel::error() const
{
	return (!m_d->m_socket) || m_d->m_socket->state() == UnconnectedState
		||  m_d->m_socket->state() == ClosingState;
}

bool RIPCClientChannel::timedOut() const
{
	return false;
}

void RIPCClientChannel::sendMessage(const VoidCallback &cob, TMemoryBuffer *message)
{
	uint8_t *obuf = nullptr;
	uint32_t sz = 0;
	message->getBuffer(&obuf, &sz);
	RIPCMessageHeader messageHeader;
	messageHeader.m_length = sz + sizeof(RIPCMessageHeader);
	messageHeader.m_version = 0;
	messageHeader.m_cid = m_d->m_currentSequanceId;
	messageHeader.m_sid = 0;
	char *messageBytes = new char[messageHeader.m_length];
	memcpy(messageBytes, s_thriftMessageHead.c_str(), 4);
	memcpy(messageBytes + 4, reinterpret_cast<const char*>(&messageHeader.m_length), 4);
	memcpy(messageBytes + 8, reinterpret_cast<const char*>(&messageHeader.m_version), 4);
	memcpy(messageBytes + 12, reinterpret_cast<const char*>(&messageHeader.m_cid), 4);
	memcpy(messageBytes + 16, reinterpret_cast<const char*>(&messageHeader.m_sid), 4);
	memcpy(messageBytes + 20, reinterpret_cast<const char*>(obuf), sz);
	message->resetBuffer(); // Cleanup for next request, see THRIFT-3496

	int sliceCount = messageHeader.m_length / RsLocalSocketBufferLength;
	int rest = messageHeader.m_length % RsLocalSocketBufferLength;
	bool writtenToQueue = true;
	for (int i = 0; i < sliceCount && writtenToQueue; ++i)
	{
		writtenToQueue = (m_d->m_socket->write(messageBytes + i * RsLocalSocketBufferLength,
			RsLocalSocketBufferLength) != -1);
	}

	if (writtenToQueue && rest)
	{
		writtenToQueue = (m_d->m_socket->write(messageBytes + sliceCount * RsLocalSocketBufferLength,
											   rest) != -1);
	}

	delete [] messageBytes;
	if (!writtenToQueue)
	{
		RIPCClientChannelDebug() << L"Fail on sendMessage";
		m_d->m_cobs.erase(m_d->m_cobs.find(m_d->m_currentSequanceId));
		m_d->m_memoryBuffers.erase(m_d->m_memoryBuffers.find(m_d->m_currentSequanceId));
		cob();
	}
}

void RIPCClientChannel::recvMessage(const VoidCallback &cob, TMemoryBuffer *message)
{
	(void)(cob);
	(void)(message);
}

void RIPCClientChannel::sendAndRecvMessage(const VoidCallback &cob, TMemoryBuffer *sendBuf, TMemoryBuffer *recvBuf)
{
	if (!good())
	{
		RIPCClientChannelDebug() << L"On sendAndRecvMessage";
		sendBuf->resetBuffer();  // Cleanup for next request, see THRIFT-3496
		recvBuf->resetBuffer();
		cob();
		return;
	}

	m_d->m_currentSequanceId++;
	if (m_d->m_currentSequanceId == s_idZero)
	{
		m_d->m_currentSequanceId = 1;
	}
	m_d->m_cobs[m_d->m_currentSequanceId] = cob;
	m_d->m_memoryBuffers[m_d->m_currentSequanceId] = recvBuf;
	sendMessage(cob, sendBuf);
}

// When to say goodbye to server, we don't care if data are written

void RIPCClientChannel::sayGoodBye()
{
	if (!good())
	{
		return;
	}
	RIPCMessageHeader messageHeader;
	messageHeader.m_length = (uint32_t)s_byeMessage.size() + sizeof(RIPCMessageHeader);
	messageHeader.m_version = 0;
	messageHeader.m_cid = 0;
	messageHeader.m_sid = 0;

	char *message = new char[messageHeader.m_length];
	memcpy(message, s_thriftMessageHead.c_str(), 4);
	memcpy(message + 4, reinterpret_cast<const char*>(&messageHeader.m_length), 4);
	memcpy(message + 8, reinterpret_cast<const char*>(&messageHeader.m_version), 4);
	memcpy(message + 12, reinterpret_cast<const char*>(&messageHeader.m_cid), 4);
	memcpy(message + 16, reinterpret_cast<const char*>(&messageHeader.m_sid), 4);
	memcpy(message + 20, reinterpret_cast<const char*>(s_byeMessage.c_str()), s_byeMessage.size());
	bool writtenToQueue = (m_d->m_socket->write(message, messageHeader.m_length) != -1);
	delete [] message;
	if (!writtenToQueue)
	{
		RIPCClientChannelDebug() << L"Fail on sayGoodBye";
	}
}

void RIPCClientChannel::close()
{
	m_d->m_buffer.clear();
	m_d->m_memoryBuffers.clear();
	m_d->m_cobs.clear();
	m_d->m_serverSequanceId = 0;
	if (m_d->m_socket)
	{
		RIPCClientChannelDebug() << L"RIPCClientChannel::close disconnectFromServer and term";
		m_d->m_socket->disconnectFromServer();
		m_d->m_socket->term();
	}
}

void RIPCClientChannel::connectToServer(const std::wstring &serverName, uint32_t msecs)
{
	RIPCClientChannelDebug() << L"connectToServer";
	if (serverName.empty())
	{
		return ;
	}
	m_d->m_serverName = serverName;
	if (!m_d->m_socket)
	{
		m_d->m_socket = new RLocalSocket(this);
	}
	else if (m_d->m_socket->state() == ConnectingState || m_d->m_socket->state() == ConnectedState)
	{
		RIPCClientChannelDebug() << L"RIPCClientChannel::connectToServer";
		return ;
	}
	else
	{
		RIPCClientChannelDebug() << L"Server is to close";
		close(); // close previous connection
	}
	m_d->m_socket->init();
	m_d->m_socket->connectToServer(serverName.c_str(), ReadWrite);
	if (m_d->m_socket->state() == UnconnectedState)
	{
		RIPCClientChannelDebug() << L"Server is not connected";
		return ;
	}
#ifdef Q_OS_WIN
	m_d->m_socket->read();
#endif
}

void RIPCClientChannel::onRecvData(char* data, unsigned int len)
{
	m_d->m_buffer.append(data, len);

	while (m_d->m_buffer.size() > sizeof(RIPCMessageHeader))
	{
#ifdef RS_IPC_CLIENT_CHANNEL_DEBUG_DUMP
		char* dumpBuffer = nullptr;
		uint32_t dataSize = m_d->m_buffer.dumpData(dumpBuffer);
		QByteArray bytes(dumpBuffer, dataSize);
		RIPCClientChannelDebug() << L"!!RIPCClientChannel::onRecvData -dumpBuffer " << QString(bytes.toHex()).utf16() << L" -size " << dataSize;
#endif

		RIPCMessageHeader* messageHeader = nullptr;
		if (!m_d->m_buffer.read((char *&)messageHeader, sizeof(RIPCMessageHeader)))
		{
			RIPCClientChannelDebug() << L"!!onRecvData 1";
			close();
			return;
		}
		if (memcmp(messageHeader->m_magicFlags, s_thriftMessageHead.c_str(), 4))
		{
			std::wstringstream hexString;
			for (size_t i = 0; i < sizeof(messageHeader->m_magicFlags); ++i)
			{
				hexString << std::hex << messageHeader->m_magicFlags[i];
			}
			RIPCClientChannelDebug() << L"!!onRecvData 2 " << hexString.str().c_str();
			close();
			return;
		}
		if (messageHeader->m_length < sizeof(RIPCMessageHeader))
		{
			RIPCClientChannelDebug() << L"!!onRecvData 3";
			close();
			return;
		}
		// There are no enough data
		if (m_d->m_buffer.size() < messageHeader->m_length)
		{
			m_d->m_buffer.unread();
			return;
		}

		uint32_t payloadLength = messageHeader->m_length - sizeof(RIPCMessageHeader);

#ifdef RS_IPC_CLIENT_CHANNEL_DEBUG_DUMP
		RIPCClientChannelDebug() << L"!!RIPCClientChannel::onRecvData" << L"messageHeader->m_length" << messageHeader->m_length;
		RIPCClientChannelDebug() << L"!!RIPCClientChannel::onRecvData" << L"payloadLength" << payloadLength;
#endif

		// For future, m_d->m_version may enable extra bytes to read.
		char * payload = nullptr;
		bool ret = m_d->m_buffer.read(payload, payloadLength);
		if (!ret)
		{
			RIPCClientChannelDebug() << L"!!onRecvData 5";
			close();
			return;
		}

		m_d->m_serverSequanceId++;

		if (messageHeader->m_cid == s_idZero)
		{
			if(m_d->m_serviceProcessor)
			{
				try
				{
					::std::shared_ptr<TMemoryBuffer> ibuf;
					ibuf.reset(new TMemoryBuffer(reinterpret_cast<uint8_t*>(payload), payloadLength, TMemoryBuffer::COPY));

					::std::shared_ptr<TMemoryBuffer> obuf;
					obuf.reset(new TMemoryBuffer());

					m_d->m_serviceProcessor->process([=](bool success)
					{
						RSLOG_DEBUG << "On Notified";
					}
					, ibuf, obuf);

					m_d->m_buffer.commit(messageHeader->m_length);
				}
				catch (std::exception& e)
				{
					RIPCClientChannelDebug() << L"!!onRecvData exception "
						+ QString::fromStdString(e.what()).toStdWString();
				}
			}
		}
		else
		{
			if (messageHeader->m_version == s_clientModeVersion)
			{
				if (m_d->m_serviceProcessor)
				{
					try
					{
						// 调用process会进入异步（含回调时），所有涉及到的变量都要在这里变成更局部的变量，
						// 因此要处理m_buffer 和拷贝 messageHeader的数据
						::std::shared_ptr<TMemoryBuffer> ibuf;
						ibuf.reset(new TMemoryBuffer(reinterpret_cast<uint8_t*>(payload),
							payloadLength, TMemoryBuffer::COPY));

						uint32_t version = messageHeader->m_version;
						uint32_t cid = messageHeader->m_cid;

						m_d->m_buffer.commit(messageHeader->m_length);

						::std::shared_ptr<TMemoryBuffer> obuf;
						obuf.reset(new TMemoryBuffer());

						m_d->m_serviceProcessor->process([=](bool success)
						{
							this->m_d->sendMessage(obuf, version, cid);
						}
						, ibuf, obuf);
					}
					catch (std::exception& e)
					{
						RIPCClientChannelDebug() << L"!!onRecvData exception "
							+ QString::fromStdString(e.what()).toStdWString();
					}
				}
				else
				{
					RIPCClientChannelDebug() << L"Request is received but no serviceProcessor is set";
				}
			}
			else
			{
				std::map<uint32_t, VoidCallback>::iterator cob = m_d->m_cobs.find(messageHeader->m_cid);
				if (cob == m_d->m_cobs.end())
				{
					RIPCClientChannelDebug() << L"!!onRecvData 7";
					close();
					return;
				}
				std::map<uint32_t, TMemoryBuffer *>::iterator mb = m_d->m_memoryBuffers.find(messageHeader->m_cid);
				if  (mb == m_d->m_memoryBuffers.end())
				{
					RIPCClientChannelDebug() << L"!!onRecvData 8";
					close();
					return;
				}

				// 因为调用cob（VoidCallback类型）可能会因为进入模态导致notifyWnd会处理下一个message而出现异步，
				// 在这之前要把当前message的一些buffer状态恢复，即允许重入onRecvData()
				mb->second->resetBuffer(reinterpret_cast<uint8_t *>(payload), payloadLength, TMemoryBuffer::COPY);
				m_d->m_buffer.commit(messageHeader->m_length);

				try
				{
					cob->second();
				}
				catch (std::exception& e)
				{
					RIPCClientChannelDebug() << L"!!onRecvData exception "
						+ QString::fromStdString(e.what()).toStdWString();
				}
				m_d->m_cobs.erase(cob);
				m_d->m_memoryBuffers.erase(mb);
			}
		}
		// Try next message
	}
}

void RIPCClientChannel::onError(LocalSocketError socketError)
{
	std::shared_ptr<RChannelNotifyHandler> statusHandler = m_d->m_statusHandler.lock();
	if (statusHandler)
	{
		RIPCClientChannelDebug() << L"onError notify";
		statusHandler->onError(socketError);
	}
	//ToDo:
	if (socketError == ServerNotFoundError)
	{
		RIPCClientChannelDebug() << L"onError ServerNotFoundError";
		close();
		return;
	}
	if (socketError == ConnectionError || socketError == UnknownSocketError || socketError == PeerClosedError)
	{
		close();
	}
	RIPCClientChannelDebug() << L"onError: " << socketError;
}

void RIPCClientChannel::onConnected()
{
	RIPCClientChannelDebug() << L"onConnected";
	std::shared_ptr<RChannelNotifyHandler> statusHandler = m_d->m_statusHandler.lock();
	if (statusHandler)
	{
		RIPCClientChannelDebug() << L"onConnected notify";
		statusHandler->onConnected();
	}
}

// NOTE 不要处理状态量, 相应地, 在onError处理错误
void RIPCClientChannel::onStateChanged(LocalSocketState socketState)
{
	RIPCClientChannelDebug() << L"onStateChanged" << socketState;
}

void RIPCClientChannel::onDisconnected()
{
	RIPCClientChannelDebug() << L"onDisconnected";
	m_d->m_buffer.clear();
	m_d->m_memoryBuffers.clear();
	m_d->m_cobs.clear();
	m_d->m_serverSequanceId = 0;
	std::shared_ptr<RChannelNotifyHandler> statusHandler = m_d->m_statusHandler.lock();
	if (statusHandler)
	{
		RIPCClientChannelDebug() << L"onDisConnected notify";
		statusHandler->onDisconnected();
	}
}
