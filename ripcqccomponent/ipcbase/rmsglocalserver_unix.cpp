#include "stdafx.h"
#include <unistd.h>
#include <sys/un.h>
#include <sys/stat.h>
#include "rmsglocalserver_unix.h"
RMsgLocalServer::RMsgLocalServer(QObject *parent) : m_server(NULL)
{
}

RMsgLocalServer::~RMsgLocalServer()
{
	stopServer();
}

bool RMsgLocalServer::makeDir(const char* path, mode_t mode)
{
	if( access( path, F_OK ) == -1 )
	{
		if( mkdir( path, mode) == -1)
		{
			return false;
		}
		if( chmod( path, mode) == -1)
		{
			return false;
		}
	}
	return true;
}

static QString getTmpPortpath(QString path)
{
	QString timeID = QString("%1").arg(getpid(), 5, 10, QLatin1Char('0')) + QDateTime::currentDateTime().toString("-yyyy-MM-dd-HH-mm-ss");
	return path + QDir::separator() + timeID;
}

bool RMsgLocalServer::startServer(const QString &strPortPath)
{
	if (isStarted())
		return false;
	m_port = getTmpPortpath(strPortPath);
	m_server = new QLocalServer(this);
	m_server->setMaxPendingConnections(MAX_CLIENTS);
	connect(m_server, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
	return m_server->listen(m_port);
}

void RMsgLocalServer::stopServer()
{
	if (isStarted())
	{
		m_server->close();
		m_server->deleteLater();
		m_server = NULL;
		QLocalServer::removeServer(m_port);
		unlink(m_port.toLocal8Bit().data());
	}
}

bool RMsgLocalServer::isStarted()
{
	return m_server != NULL && m_server->isListening();
}

int RMsgLocalServer::sendRequest(QLocalSocket *client, const QByteArray& byteData)
{
	qint64 bytesWritten = client->write(byteData);
	client->flush();
	return bytesWritten;
}

void RMsgLocalServer::onNewConnection()
{
	QLocalSocket *socket = m_server->nextPendingConnection();
	if (socket == NULL)
		return ;
	connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
	connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
}

void RMsgLocalServer::onReadyRead()
{
	QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
	QByteArray byteData = socket->readAll();
	if (byteData.size() > 0)
		doParserRequest (socket, byteData);
}

void RMsgLocalServer::doParserRequest(QLocalSocket *socket, const QByteArray& byte)
{
#pragma pack(1)
	struct CmdMsgData
	{
        unsigned short m_len;       // 整个数据的长度
		unsigned short m_type;
		char m_msgContent[0];
	};
#pragma pack()
	enum CMD_MSGDATA_TYPE
	{
		CMD_MSGDATA_DEFAULT = 0,
		CMD_MSGDATA_HEART_REQUEST = 1,
		CMD_MSGDATA_HEART_REPLY = 2,
		CMD_MSGDATA_CMDMSG_REQUEST = 3,
		CMD_MSGDATA_CMDMSG_REPLY = 4,
	};

	const CmdMsgData * pData = (const CmdMsgData *)byte.data();
	if (byte.size() < sizeof(unsigned short) || byte.size() < pData->m_len)
		return;
	QByteArray byteRepyData(sizeof(CmdMsgData), 0);

	CmdMsgData * pRepyData = (CmdMsgData *)byteRepyData.data();
	pRepyData->m_len = sizeof(CmdMsgData);
	if (pData->m_type == CMD_MSGDATA_HEART_REQUEST)
	{
		pRepyData->m_type = CMD_MSGDATA_HEART_REPLY;
		sendRequest(socket, byteRepyData);
		const QString strCmd = QString::fromLocal8Bit(pData->m_msgContent);
		emit clientHeaderRequest(strCmd);
		return;
	}
	else if (pData->m_type == CMD_MSGDATA_CMDMSG_REQUEST)
	{
		pRepyData->m_type = CMD_MSGDATA_CMDMSG_REPLY;
		sendRequest(socket, byteRepyData);
		const QString strCmd = QString::fromLocal8Bit(pData->m_msgContent);
		emit clientRequest(strCmd);
		return;
	}
}

void RMsgLocalServer::onDisconnected()
{
	QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
	socket->close();
	socket->deleteLater();
}

