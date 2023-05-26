#include "stdafx.h"
#include "rqtlocalsocket.h"
#include "../utils/utils.h"

RLocalSocket::RLocalSocket(IRLocalSocketNotify* pNotify)
	: m_pImpl(new QLocalSocket(NULL))
	, QObject(NULL)
    , RLocalSocketBase(pNotify)
{
}

RLocalSocket::~RLocalSocket()
{
	m_pImpl->deleteLater();
}

void RLocalSocket::init()
{
	connect(m_pImpl, SIGNAL(readyRead()), this, SLOT(slotOnReadyRead()));
	connect(m_pImpl, SIGNAL(connected()), this, SLOT(slotOnConnected()));
	connect(m_pImpl, SIGNAL(disconnected()), this, SLOT(slotOnDisconnected()));
	connect(m_pImpl, SIGNAL(error(QLocalSocket::LocalSocketError)), this, 
		SLOT(slotOnError(QLocalSocket::LocalSocketError)));
	connect(m_pImpl, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)), this, 
		SLOT(slotOnStateChanged(QLocalSocket::LocalSocketState)));	
}

void RLocalSocket::connectToServer(const wchar_t* name, OpenMode openMode)
{
    m_pImpl->connectToServer(QString::fromWCharArray(name), (QLocalSocket::OpenMode)openMode);
}

void RLocalSocket::disconnectFromServer()
{
    m_pImpl->disconnectFromServer();
    if (m_pImpl->state() == QLocalSocket::UnconnectedState
        || m_pImpl->waitForDisconnected(5000)) {
        qDebug("lcoal socket Disconnected!");
    }
}

bool RLocalSocket::setSocketDescriptor(quintptr socketDescriptor)
{
	return m_pImpl->setSocketDescriptor(socketDescriptor);
}

void RLocalSocket::term()
{
	m_pImpl->disconnectFromServer();
    if (m_pImpl->state() == QLocalSocket::UnconnectedState || m_pImpl->waitForDisconnected(5000))
		qDebug("Child Process Socket Disconnected!");

	m_pImpl->close();
	disconnect(m_pImpl, 0, 0, 0);
}

LocalSocketState RLocalSocket::state() const
{
	return (LocalSocketState)m_pImpl->state();
}

bool RLocalSocket::flush()
{
	return m_pImpl->flush();
}

int RLocalSocket::write(const char* buf, unsigned int len)
{
	return m_pImpl->write(buf, len);
}

bool RLocalSocket::waitForBytesWritten(int msecs)
{
	return m_pImpl->waitForBytesWritten();
}

void RLocalSocket::close()
{
	return m_pImpl->close();
}

void RLocalSocket::slotOnReadyRead()
{
	QByteArray buffer = m_pImpl->readAll();
	onRecvData(buffer.data(), buffer.size());
}

void RLocalSocket::slotOnError(QLocalSocket::LocalSocketError socketError)
{
    RSLOG_ERROR_F2("KQtLocalSocket::slotOnError socketError is %s", m_pImpl->errorString().toLocal8Bit().data());
	onError((LocalSocketError)socketError);
}

void RLocalSocket::slotOnStateChanged(QLocalSocket::LocalSocketState socketState)
{
	onStateChanged((LocalSocketState)socketState);
}

void RLocalSocket::slotOnConnected()
{
	onConnected();
}

void RLocalSocket::slotOnDisconnected()
{
	onDisconnected();
}





