// -----------------------------------------------------------------------
//
// 文 件 名 ： kxmsglocalserver.h
// 创建时间 ：2018-12
// 功能描述 ：用于替代原来的wps客户端之间的共享内存机制通信来保持唯一客户端的功能,提升启动效率
//
// -----------------------------------------------------------------------
#ifndef __KWPSDSPS_KX_MSG_LOCAL_SERVER_H__2018_
#define __KWPSDSPS_KX_MSG_LOCAL_SERVER_H__2018_

#include "QObject"
#include "QLocalServer"
#include "QLocalSocket"
class Q_DECL_EXPORT kxMsgLocalServer : public QObject
{
	Q_OBJECT

public:
	enum { MAX_CLIENTS = 1 };
public:
	kxMsgLocalServer(QObject *parent = 0);
	~kxMsgLocalServer();

	bool startServer(const QString &port);
	void stopServer();
	bool isStarted();
	bool makeDir(const char* path, mode_t mode);
private:
	int sendRequest(QLocalSocket * client, const QByteArray& byteData);
	void doParserRequest(QLocalSocket *socket, const QByteArray& byte);

private slots:
	// QLocalServer
	void onNewConnection();
	// QLocalSocket
	void onReadyRead();
	void onDisconnected();

signals:
	void clientHeaderRequest(const QString& cmd);
	void clientRequest(const QString& cmd);
private:
	QLocalServer *m_server;
	QString m_port;
};

#endif // __KWPSDSPS_KX_MSG_LOCAL_SERVER_H__2018_
