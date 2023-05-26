
#ifndef __R_MSG_LOCAL_SERVER_H__
#define __R_MSG_LOCAL_SERVER_H__

#include "QObject"
#include "QLocalServer"
#include "QLocalSocket"
class RMsgLocalServer : public QObject
{
	Q_OBJECT

public:
	enum { MAX_CLIENTS = 1 };
public:
    RMsgLocalServer(QObject *parent = 0);
    ~RMsgLocalServer();

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
