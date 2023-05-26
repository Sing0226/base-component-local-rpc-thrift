#ifndef _R_QTLOCALSOCKET_H_
#define _R_QTLOCALSOCKET_H_

#include <QLocalSocket>
#include "rlocalsocketdef.h"

class RLocalSocket : public QObject, public RLocalSocketBase
{
	Q_OBJECT

public:
    RLocalSocket(IRLocalSocketNotify* pNotify);
    virtual ~RLocalSocket();

	void connectToServer(const wchar_t* name, OpenMode openMode);
	void disconnectFromServer();

	bool setSocketDescriptor(quintptr socketDescriptor);

	void init();
	void term();

	LocalSocketState state() const;
	bool flush();
	int write(const char* buf, unsigned int len);
	bool waitForBytesWritten(int msecs = 30000);

private slots:
	void close();
	void slotOnReadyRead();
	void slotOnError(QLocalSocket::LocalSocketError socketError);
	void slotOnStateChanged(QLocalSocket::LocalSocketState socketState);
	void slotOnConnected();
	void slotOnDisconnected();

private:
	QLocalSocket* m_pImpl;
};

#endif // _KWPSDSPS_KQTLOCALSOCKET_H_
