#include "stdafx.h"
#include "kqtlocalsocket.h"
#include "../utils/utils.h"

KLocalSocket::KLocalSocket(IKxLocalSocketNotify* pNotify)
	: m_pImpl(new QLocalSocket(NULL))
	, QObject(NULL)
	, KLocalSocketBase(pNotify)
{
}

KLocalSocket::~KLocalSocket()
{
	m_pImpl->deleteLater();
}

void KLocalSocket::init()
{
	connect(m_pImpl, SIGNAL(readyRead()), this, SLOT(slotOnReadyRead()));
	connect(m_pImpl, SIGNAL(connected()), this, SLOT(slotOnConnected()));
	connect(m_pImpl, SIGNAL(disconnected()), this, SLOT(slotOnDisconnected()));
	connect(m_pImpl, SIGNAL(error(QLocalSocket::LocalSocketError)), this, 
		SLOT(slotOnError(QLocalSocket::LocalSocketError)));
	connect(m_pImpl, SIGNAL(stateChanged(QLocalSocket::LocalSocketState)), this, 
		SLOT(slotOnStateChanged(QLocalSocket::LocalSocketState)));	
}

void KLocalSocket::connectToServer(const wchar_t* name, OpenMode openMode)
{
    m_pImpl->connectToServer(QString::fromWCharArray(name), (QLocalSocket::OpenMode)openMode);
}

void KLocalSocket::disconnectFromServer()
{
    m_pImpl->disconnectFromServer();
    if (m_pImpl->state() == QLocalSocket::UnconnectedState
        || m_pImpl->waitForDisconnected(5000)) {
        qDebug("lcoal socket Disconnected!");
    }
}

bool KLocalSocket::setSocketDescriptor(quintptr socketDescriptor)
{
	return m_pImpl->setSocketDescriptor(socketDescriptor);
}

void KLocalSocket::term()
{
	m_pImpl->disconnectFromServer();
    if (m_pImpl->state() == QLocalSocket::UnconnectedState || m_pImpl->waitForDisconnected(5000))
		qDebug("Child Process Socket Disconnected!");

	m_pImpl->close();
	disconnect(m_pImpl, 0, 0, 0);
}

LocalSocketState KLocalSocket::state() const
{
	return (LocalSocketState)m_pImpl->state();
}

bool KLocalSocket::flush()
{
	return m_pImpl->flush();
}

int KLocalSocket::write(const char* buf, unsigned int len)
{
	return m_pImpl->write(buf, len);
}

bool KLocalSocket::waitForBytesWritten(int msecs)
{
	return m_pImpl->waitForBytesWritten();
}

void KLocalSocket::close()
{
	return m_pImpl->close();
}

void KLocalSocket::slotOnReadyRead()
{
	QByteArray buffer = m_pImpl->readAll();
	onRecvData(buffer.data(), buffer.size());
}

void KLocalSocket::slotOnError(QLocalSocket::LocalSocketError socketError)
{
	KWpsDspsIpcUtils::writeLog(QString("KQtLocalSocket::slotOnError socketError is %1").arg(m_pImpl->errorString()));
	onError((LocalSocketError)socketError);
}

void KLocalSocket::slotOnStateChanged(QLocalSocket::LocalSocketState socketState)
{
	onStateChanged((LocalSocketState)socketState);
}

void KLocalSocket::slotOnConnected()
{
	onConnected();
}

void KLocalSocket::slotOnDisconnected()
{
	onDisconnected();
}





