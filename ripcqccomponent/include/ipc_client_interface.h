#ifndef __CLIENT_INTERFACE_H__
#define __CLIENT_INTERFACE_H__
#include "ipc_declare.h"
#include "ipc_error.h"
#include <string>
#include <vector>
#include <map>
#include <list>

namespace rs_client_ipc{

	typedef struct AuthorInfo{
		int authorStatus;
		std::string authorInfo;

		AuthorInfo()
			: authorStatus(-1)
		{
		}

		AuthorInfo& operator=(const AuthorInfo& info)//重载运算符
		{
			set(this, (AuthorInfo*)&info);

			return *this;
		}

		AuthorInfo(const AuthorInfo& s)//复制构造函数
		{
			*this = s;
		}

	private:
		void set(AuthorInfo* info1, AuthorInfo* info2)//赋值函数
		{
			info1->authorStatus = info2->authorStatus;
			info1->authorInfo = info2->authorInfo;
		}
	}AuthorInfo,*PAuthorInfo;


	// 客户端回调通知，需要客户端实现回调通知
	class IRsIpcClientNotify
	{
	public:
		virtual void notifyConnected() = 0;
		virtual void notifyDisconnected() = 0;
        virtual void notifyConnectErr() = 0;

		// 广播通知
		virtual void notifyAuthorInfo(const std::string& autorDes) = 0;
		virtual void notifyUserLoginStatus(LoginErrCode err) = 0;
		virtual void notifyGetAuthorInfo(const rs_client_ipc::AuthorInfo& authorInfo) = 0;
	};

}

// 客户请求接口，无需客户
class RIPC_QC_EXPORT IRsIpcClientInterface
{
public:
	IRsIpcClientInterface();
	virtual ~IRsIpcClientInterface();

public:
	virtual void initClient() = 0;
	virtual void setClientNotify(rs_client_ipc::IRsIpcClientNotify* pIpcClientNotify) = 0;
	virtual void userLogin(const std::string& userName, const std::string& userPwd) = 0;
	virtual std::string getAuthorInfo() = 0;
};

// 一个client对应一个notify,客户端可以使用列表缓存进行管理
extern "C" RIPC_QC_EXPORT void initIpcClientInterfaceInstance(rs_client_ipc::IRsIpcClientNotify * pIpcClientNotify);
extern "C" RIPC_QC_EXPORT IRsIpcClientInterface* getIpcClientInterfaceInstance();
extern "C" RIPC_QC_EXPORT void releaseIpcClientInterfaceInstance();

#endif /* __CLIENT_INTERFACE_H__ */
