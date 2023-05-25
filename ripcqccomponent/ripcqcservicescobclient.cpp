#include "stdafx.h"
#include "ripcqcservicescobclient.h"
#include <thrift/async/TAsyncChannel.h>

namespace {
	struct CallBackWrap
	{
		::std::function<void()> m_cob;

		CallBackWrap(::std::function<void(RIpcQcServicesCobClient* client)>& cob, RIpcQcServicesCobClient* client)
			: m_cob(::std::bind(cob, client)) { }

		void operator()(rs::qc::ripcqcservicesCobClient* client)
		{
			m_cob();
		}
	};
};

//////////////////////////////////////////////////////////////////////////
#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_0(func_name, call_back) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob \
	) { \
	if (m_handler) { \
	m_handler->func_name(call_back); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this)); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_1(func_name, call_back, arg1_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_2(func_name, call_back, arg1_type, arg2_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1, arg2_type arg2) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1, arg2); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1, arg2); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_4(func_name, call_back, arg1_type, arg2_type, arg3_type, arg4_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1, arg2, arg3, arg4); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1, arg2, arg3, arg4); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_5(func_name, call_back, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1, arg2, arg3, arg4, arg5); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1, arg2, arg3, arg4, arg5); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_6(func_name, call_back, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1, arg2, arg3, arg4, arg5, arg6); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1, arg2, arg3, arg4, arg5, arg6); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_7(func_name, call_back, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6, arg7_type arg7) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1, arg2, arg3, arg4, arg5, arg6, arg7); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1, arg2, arg3, arg4, arg5, arg6, arg7); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_8(func_name, call_back, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_9(func_name, call_back, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, arg9_type arg9) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_10(func_name, call_back, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type) \
	void RIpcQcServicesCobClient::func_name(::std::function<void(RIpcQcServicesCobClient* client)> cob, \
	arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, arg9_type arg9, arg10_type arg10) { \
	if (m_handler) { \
	m_handler->func_name(call_back, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10); return cob(this); } \
	else if (m_client.get()) \
	return m_client->func_name(CallBackWrap(cob, this), arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10); \
	ASSERT(0); }


#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_0(func_name) \
	void RIpcQcServicesCobClient::send_##func_name() { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(); \
    ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_1(func_name, arg1_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_2(func_name, arg1_type, arg2_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1, arg2_type arg2) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1, arg2); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_RECV_PARAMS_0(func_name) \
	void RIpcQcServicesCobClient::recv_##func_name() { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->recv_##func_name(); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_4(func_name, arg1_type, arg2_type, arg3_type, arg4_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1, arg2, arg3, arg4); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_5(func_name, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1, arg2, arg3, arg4, arg5); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_6(func_name, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1, arg2, arg3, arg4, arg5, arg6); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_7(func_name, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6, arg7_type arg7) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_8(func_name, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_9(func_name, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, arg9_type arg9) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_10(func_name, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, arg6_type, arg7_type, arg8_type, arg9_type, arg10_type) \
	void RIpcQcServicesCobClient::send_##func_name(arg1_type arg1, arg2_type arg2, arg3_type arg3, arg4_type arg4, arg5_type arg5, arg6_type arg6, arg7_type arg7, arg8_type arg8, arg9_type arg9, arg10_type arg10) { \
	if (m_handler) return; \
	else if (m_client.get()) \
	return m_client->send_##func_name(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10); \
	ASSERT(0); }

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_RECV_PARAMS_1(func_name, arg1_type, arg1_val) \
	void RIpcQcServicesCobClient::recv_##func_name(arg1_type arg1) { \
	if (m_handler) { arg1 = arg1_val; return; } \
	else if (m_client.get()) \
	return m_client->recv_##func_name(arg1); \
	ASSERT(0); }


#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_PARAMS_0(func_name) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_0(func_name, [](){}) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_0(func_name) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_RECV_PARAMS_0(func_name)

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_PARAMS_1(func_name, arg1_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_1(func_name, [](){}, arg1_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_1(func_name, arg1_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_RECV_PARAMS_0(func_name)

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_PARAMS_1_RET_PARAMS(func_name, call_back, return_val, arg1_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_1(func_name, call_back, arg1_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_1(func_name, arg1_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_RECV_PARAMS_1(func_name, return_val)

#define IMPL_IPC_SERVICES_COB_CLIENT_CALL_PARAMS_2_RET_PARAMS(func_name, call_back, return_val, arg1_type, arg2_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_2(func_name, call_back, arg1_type, arg2_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_2(func_name, arg1_type, arg2_type) \
	IMPL_IPC_SERVICES_COB_CLIENT_CALL_RECV_PARAMS_1(func_name, return_val)

//////////////////////////////////////////////////////////////////////////
class TAsyncChannelDummy : public apache::thrift::async::TAsyncChannel
{
	bool m_good;
public:
	explicit TAsyncChannelDummy(bool good) : m_good(good) { }
	virtual bool good() const override { return m_good; }
	virtual bool error() const override { return !m_good; }
	virtual bool timedOut() const override { return !m_good; };
	virtual void sendMessage(const TAsyncChannel::VoidCallback& cob,
		apache::thrift::transport::TMemoryBuffer* message) override
	{
		ASSERT(false);
	}
	virtual void recvMessage(const TAsyncChannel::VoidCallback& cob,
		apache::thrift::transport::TMemoryBuffer* message) override
	{
		ASSERT(false);
	}
	virtual void sendAndRecvMessage(const TAsyncChannel::VoidCallback& cob,
		apache::thrift::transport::TMemoryBuffer* sendBuf,
		apache::thrift::transport::TMemoryBuffer* recvBuf) override
	{
		ASSERT(false);
	}
};


RIpcQcServicesCobClient::RIpcQcServicesCobClient()
	: m_handler(NULL)
	, m_login_err(rs::qc::ELoginErrCode::ERR_USER_UNDEFINED)
{
}

RIpcQcServicesCobClient::RIpcQcServicesCobClient(::std::shared_ptr<::apache::thrift::async::TAsyncChannel> channel, ::apache::thrift::protocol::TProtocolFactory* factory)
	: m_client(new rs::qc::ripcqcservicesCobClient(channel, factory))
	, m_handler(NULL)
	, m_login_err(rs::qc::ELoginErrCode::ERR_USER_UNDEFINED)
{
	resetClientData();
}

RIpcQcServicesCobClient::~RIpcQcServicesCobClient()
{

}

::std::shared_ptr<apache::thrift::async::TAsyncChannel> RIpcQcServicesCobClient::getChannel()
{
	if (m_handler)
	{
		static ::std::shared_ptr<apache::thrift::async::TAsyncChannel> s_dummyGood(new TAsyncChannelDummy(true));
		return s_dummyGood;
	}
	else if (m_client)
	{
		return m_client->getChannel();
	}

	static ::std::shared_ptr<apache::thrift::async::TAsyncChannel> s_dummyBad(new TAsyncChannelDummy(false));
	return s_dummyBad;
}

void RIpcQcServicesCobClient::setDirectHandler(rs::qc::ripcqcservicesCobSvIf* handler)
{
	ASSERT(handler);
	m_handler = handler;
}

void RIpcQcServicesCobClient::resetClientData()
{
	m_recv_queryAuthorStatus.authorStatus = -1;
	m_recv_queryAuthorStatus.strAutorInfo = "";
}


void RIpcQcServicesCobClient::setAuthorStatus(const rs::qc::ResAuthorStatus& resAuthorStatus)
{
	m_recv_queryAuthorStatus.authorStatus = resAuthorStatus.authorStatus;
	m_recv_queryAuthorStatus.strAutorInfo = resAuthorStatus.strAutorInfo;
}

const rs::qc::ResAuthorStatus& RIpcQcServicesCobClient::getAuthorStatus()
{
	return m_recv_queryAuthorStatus;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IMPL_IPC_SERVICES_COB_CLIENT_CALL_PARAMS_1(notifyAuthorizeStatusChanged, const std::string&)

IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_1(queryAuthorStatus, [&](const rs::qc::ResAuthorStatus& resAuthorStatus) { m_recv_queryAuthorStatus = resAuthorStatus; }, const std::string&)
IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_1(queryAuthorStatus, const std::string&)
IMPL_IPC_SERVICES_COB_CLIENT_CALL_RECV_PARAMS_1(queryAuthorStatus, rs::qc::ResAuthorStatus&, m_recv_queryAuthorStatus)


IMPL_IPC_SERVICES_COB_CLIENT_CALL_FUNC_PARAMS_2(userLogin, [&](const int32_t& err) {m_login_err = err; }, const std::string&, const std::string&)
IMPL_IPC_SERVICES_COB_CLIENT_CALL_SEND_PARAMS_2(userLogin, const std::string&, const std::string&)
// IMPL_IPC_SERVICES_COB_CLIENT_CALL_RECV_PARAMS_1(userLogin, rs::qc::ELoginErrCode::type, m_login_err)

/*
void RIpcQcServicesCobClient::notifyAuthorizeStatusChanged(::std::function<void(RIpcQcServicesCobClient* client)> cob,
	const std::string& authId)
{
	if (m_handler) 
	{
		m_handler->notifyAuthorizeStatusChanged([]() {}, authId);
		return cob(this);
	}
	else if (m_client.get())
		return m_client->notifyAuthorizeStatusChanged(CallBackWrap(cob, this), authId);
}

void RIpcQcServicesCobClient::send_notifyAuthorizeStatusChanged(const std::string& authId) 
{
	if (m_handler) 
		return;
	else if (m_client.get())
		return m_client->send_notifyAuthorizeStatusChanged(authId);
}

void RIpcQcServicesCobClient::recv_notifyAuthorizeStatusChanged()
{
	if (m_handler)
		return;
	else if (m_client.get())
		return m_client->recv_notifyAuthorizeStatusChanged();
}
*/

// test implement
/*
void RIpcQcServicesCobClient::queryAuthorStatus(::std::function<void(RIpcQcServicesCobClient* client)> cob)
{
	if (m_handler) 
	{ 
		m_handler->queryAuthorStatus([&](const rs::qc::ResAuthorStatus& _r) { m_recv_queryAuthorStatus = _r; });
		return cob(this); 
	} 
	else if (m_client.get()) 
	{
		return m_client->queryAuthorStatus(CallBackWrap(cob, this));
	}
}

void RIpcQcServicesCobClient::send_queryAuthorStatus()
{

}

void RIpcQcServicesCobClient::recv_queryAuthorStatus(rs::qc::ResAuthorStatus& _return)
{
	if (m_handler)
	{
		_return = m_recv_queryAuthorStatus;
		return; 
	}
	else if (m_client.get())
	{
		return m_client->recv_queryAuthorStatus(_return);
	}
}
*/
/*
void RIpcQcServicesCobClient::userLogin(::std::function<void(RIpcQcServicesCobClient* client)> cob,
	const std::string& userName, const std::string& userPwd)
{
	if (m_handler)
	{
		m_handler->userLogin([&](rs::qc::ELoginErrCode::type const& _r) { m_login_err = _r; }, userName, userPwd);
		return cob(this);
	}
	else if (m_client.get())
	{
		return m_client->userLogin(CallBackWrap(cob, this), userName, userPwd);
	}
}


void RIpcQcServicesCobClient::send_userLogin(const std::string& userName, const std::string& userPwd)
{
	if (m_handler) 
		return;
	else if (m_client.get())
		return m_client->send_userLogin(userName, userPwd);
}
*/
int32_t RIpcQcServicesCobClient::recv_userLogin()
{
	if (m_handler)
	{
		return m_login_err;
	}
	else if (m_client.get())
	{
		return m_client->recv_userLogin();
	}

	return m_login_err;
}
