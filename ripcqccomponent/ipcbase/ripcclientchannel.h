#ifndef _RIPCCLIENTCHANNEL_H__
#define _RIPCCLIENTCHANNEL_H__

#include <memory>

// #include <boost/shared_ptr.hpp>
#include <thrift/async/TAsyncChannel.h>
#include <thrift/async/TAsyncProtocolProcessor.h>

#include "ripc.h"
#include "rlocalsocketdef.h"


class RChannelNotifyHandler
{
public:
	RChannelNotifyHandler()
	{
	}
	virtual ~RChannelNotifyHandler()
	{
	}
	virtual void onConnected() {}
	virtual void onDisconnected() {}
	virtual void onError(LocalSocketError err) {}
};

typedef apache::thrift::async::TAsyncProtocolProcessor TxAsyncProtocolProcessor;
class RIPCClientChannelPrivate;
class RIPCClientChannel
	: public IRLocalSocketNotify, public apache::thrift::async::TAsyncChannel
{
public:
	explicit RIPCClientChannel(std::shared_ptr<RChannelNotifyHandler> statuHandler);
	virtual ~RIPCClientChannel();
	virtual bool good() const override;
	virtual bool error() const override;
	virtual bool timedOut() const override;

	virtual void sendMessage(const VoidCallback& cob,
							apache::thrift::transport::TMemoryBuffer* message) override;
	virtual void recvMessage(const VoidCallback& cob,
							apache::thrift::transport::TMemoryBuffer* message) override;
	virtual void sendAndRecvMessage(const VoidCallback& cob,
									apache::thrift::transport::TMemoryBuffer* sendBuf,
									apache::thrift::transport::TMemoryBuffer* recvBuf) override;
	// todo liangyongkang unused parameter <msecs>
	// todo liangyongkang blocking connect ?
	void connectToServer(const std::wstring &serverName, uint32_t msecs = 3000);
	void setServiceProcessor(std::shared_ptr<TxAsyncProtocolProcessor> serviceProcessor);
	LocalSocketState state() const;

private: //IKxLocalSocketNotify
	virtual void onConnected() override;
	virtual void onDisconnected() override;
	virtual void onStateChanged(LocalSocketState socketState) override;
	virtual void onError(LocalSocketError) override;
	virtual void onRecvData(char*, unsigned int) override;
	void close();
	void sayGoodBye();
private:
	RIPCClientChannelPrivate *m_d;
};

#endif // KCLOUDCHANNEL_H
