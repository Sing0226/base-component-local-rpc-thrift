#ifndef __RIPC_QC_IPCCLIENT_H__
#define __RIPC_QC_IPCCLIENT_H__
#include "stdafx.h"
#include <QObject>
#include "gen-cpp/ripcqcservices.h"
#include "ipcbase/ripc.h"
#include "ipc_client_interface.h"
#include <list>
#include <QMutex>


class RIpcQcServicesCobClient;
class RIpcQcServicesClientHandler;

class RIpcQcClient : public QObject
{
	Q_OBJECT

	class Handler : public RChannelNotifyHandler
	{
	public:
		explicit Handler(RIpcQcClient* outer)
			: m_outer(outer) {}
	protected:
		virtual void onConnected() override
		{
			RSLOG_DEBUG << "IPC onStateChanged Connected";
			if (m_outer)
				m_outer->connectedSucceed();
		}

		virtual void onDisconnected() override
		{
			RSLOG_DEBUG << "IPC onStateChanged Disconnected";
			if (m_outer)
				m_outer->disconnected();
		}

		virtual void onError(LocalSocketError errorCode) override
		{
			RSLOG_DEBUG <<
				QString("IPC onStateChanged onError LocalSocketError: %1").arg(errorCode).toLocal8Bit().data();
			if (m_outer && errorCode != UnknownSocketError)
				m_outer->connectingError(errorCode);
		}

	private:
		RIpcQcClient* m_outer;
	};

public:
	explicit RIpcQcClient(QObject* parent = nullptr);
	~RIpcQcClient();

public:
	void initCobClient();
	void connectToServer(const QString& serverName);

	std::shared_ptr<RIpcQcServicesCobClient> cobClient();

	// 处理连接成功消息使用 add by Simone 2023-01-06
	void resgisterIpcClientNotify(rs_client_ipc::IRsIpcClientNotify* clientNotify);
	void unresgisterIpcClientNotify(rs_client_ipc::IRsIpcClientNotify* clientNotify);

	void connectedSucceed();
	void disconnected();
	void connectingError(LocalSocketError errorCode);

	QString getServerName() const;

private:
	::std::shared_ptr<RIPCClientChannel> m_spIpcChannel;
	std::shared_ptr<RChannelNotifyHandler> m_spHandler;
	std::shared_ptr<RIpcQcServicesCobClient> m_spCobClient;

	bool m_bInitialized;
	bool m_bConnected;

	QString m_serverName;

	std::list<rs_client_ipc::IRsIpcClientNotify*> m_listClientNotify;
	QMutex* m_pListLockerMutex;

	::std::shared_ptr<RIpcQcServicesClientHandler> m_spSrvClHandler;
};


#endif //__RIPC_QC_IPCCLIENT_H__
