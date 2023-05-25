#ifndef __RIPC_QC_INTERFACE_IMPL_H_
#define __RIPC_QC_INTERFACE_IMPL_H_
#include "ipc_client_interface.h"
#include <memory>

class RIpcQcServicesCobClient;
class RIpcQcClient;

namespace rs {
	namespace qc {
		class ResListPlaneInfo;
		class ResListPatientInfo;
	}
}

class RIpcQcInterfaceImpl : public IRsIpcClientInterface
{
public:
	explicit RIpcQcInterfaceImpl(rs_client_ipc::IRsIpcClientNotify* pIpcClientNotify = nullptr);
	virtual ~RIpcQcInterfaceImpl();

public:
	virtual void initClient() override;
	virtual void setClientNotify(rs_client_ipc::IRsIpcClientNotify* pIpcClientNotify) override;
	virtual std::string getAuthorInfo() override;
	virtual void userLogin(const std::string& userName, const std::string& userPwd) override;

protected:
	void _connectToServer();
	QString _createUUID();

public:
	std::shared_ptr<RIpcQcServicesCobClient> cobClient();
	
	rs_client_ipc::IRsIpcClientNotify* getClientNotify();

private:

	rs_client_ipc::IRsIpcClientNotify* m_pClientNotify;
	RIpcQcClient* m_pIpcClient;

	bool m_bConnectStatus;

};


#endif