#ifndef _KLOCALSERVER_H_
#define _KLOCALSERVER_H_

#include <QLocalServer>
// #include <boost/shared_ptr.hpp>

#include "ripc.h"

namespace apache{ namespace thrift{ namespace transport{
class TMemoryBuffer;
}}}

namespace apache{ namespace thrift{ namespace async{
class TAsyncChannel;
}}}

struct RLocalServerPrivate;
class RIPCServerConnection;

class RLocalServer : public QObject
{
	Q_OBJECT
public:
	RLocalServer(const QString& serverName, QObject* parent = nullptr);
	~RLocalServer();
	::std::shared_ptr<apache::thrift::async::TAsyncChannel> channel();
	bool isListening() const;
signals:
	void error(const QString&);

public slots:
	void serve(const QString& serverName /*= QString()*/);
	void stop();
	void broadcast(::std::shared_ptr<apache::thrift::transport::TMemoryBuffer> obuf);

private slots:
	void onConnectionFinished();

protected:
	 virtual void incomingConnection(quintptr socketDescriptor);

protected:
	// 派生类实现，根据socketdescriptor创建 KIPCServerConnection
	virtual ::std::shared_ptr<RIPCServerConnection> newConnection(quintptr socketDescriptor) = 0;

protected:
	friend class RIPCServerConnection;
	friend class _QLocalServer;
	RLocalServerPrivate *m_d;
};

#endif // _KLOCALSERVER_H_
