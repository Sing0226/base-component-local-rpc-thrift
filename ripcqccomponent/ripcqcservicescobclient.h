#ifndef __RIPC_QC_SERVICES_COB_CLIENT_H_
#define __RIPC_QC_SERVICES_COB_CLIENT_H_

#include "gen-cpp/ripcqcservices.h"
#include "gen-cpp/ripcqcservices_types.h"


class RIpcQcServicesCobClient
{

public:
	RIpcQcServicesCobClient();
	RIpcQcServicesCobClient(::std::shared_ptr<::apache::thrift::async::TAsyncChannel> channel , ::apache::thrift::protocol::TProtocolFactory* factory);
	~RIpcQcServicesCobClient();

	::std::shared_ptr<::apache::thrift::async::TAsyncChannel> getChannel();

public:
	void notifyAuthorizeStatusChanged(::std::function<void(RIpcQcServicesCobClient* client)> cob, const std::string& authId);
	void send_notifyAuthorizeStatusChanged(const std::string& userId);
	void recv_notifyAuthorizeStatusChanged();

	void queryAuthorStatus(::std::function<void(RIpcQcServicesCobClient* client)> cob, const std::string& reqId);
	void send_queryAuthorStatus(const std::string& reqId);
	void recv_queryAuthorStatus(rs::qc::ResAuthorStatus& _return);

	void userLogin(::std::function<void(RIpcQcServicesCobClient* client)> cob, const std::string& userName, const std::string& userPwd);
	void send_userLogin(const std::string& userName, const std::string& userPwd);
	int32_t recv_userLogin();

	void setAuthorStatus(const rs::qc::ResAuthorStatus& resAuthorStatus);
	const rs::qc::ResAuthorStatus& getAuthorStatus();

protected:
	void setDirectHandler(rs::qc::ripcqcservicesCobSvIf* handler);
	void resetClientData();	

private:
	std::unique_ptr<rs::qc::ripcqcservicesCobClient> m_client;			// 客户端
	rs::qc::ripcqcservicesCobSvIf* m_handler;							// 服务端

	rs::qc::ResAuthorStatus m_recv_queryAuthorStatus;
	int32_t m_login_err;
};


#endif