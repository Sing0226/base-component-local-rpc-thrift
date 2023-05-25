#ifndef __RIPC_QC_INTERFACE_PROXY_H_
#define __RIPC_QC_INTERFACE_PROXY_H_

#include "ipc_declare.h"
#include "ipc_client_interface.h"

class IpcQcInterfaceProxy : public IRsIpcClientInterface
{
public:
	explicit IpcQcInterfaceProxy(IRsIpcClientInterface* ipcClientImpl = nullptr);
	virtual ~IpcQcInterfaceProxy();

	virtual void initClient() override;
	void setClientNotify(rs_client_ipc::IRsIpcClientNotify* pIpcClientNotify) override;

	virtual std::string getAuthorInfo() override;
	virtual void userLogin(const std::string& userName, const std::string& userPwd) override;

private:
	IRsIpcClientInterface* m_pClientImpl;
};


#endif	// __RIPC_QC_INTERFACE_PROXY_H_