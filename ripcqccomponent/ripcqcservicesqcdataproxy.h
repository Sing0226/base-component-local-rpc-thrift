#ifndef __RIPC_QC_SERVICES_SCAN_DATA_PROXY_H__
#define __RIPC_QC_SERVICES_SCAN_DATA_PROXY_H__

#include "ripcqcservicesdataproxy.h"

class RIpcServiceQcDataProxy : public RIpcServicesDataProxy
{
	Q_OBJECT
public:
	explicit RIpcServiceQcDataProxy(QObject* parent);
	virtual ~RIpcServiceQcDataProxy();

public:
	virtual void setServiceAuthorisationStatus(const rs::qc::ResAuthorStatus& authorStatus) override;
	virtual int setServiceAuthorInfoHandlerMap(const std::string& reqId, RIpcQcResAuthorStatusCob* handler) override;
	virtual int setServiceLoginInfoHandlerMap(const std::string& reqId, RIpcQcLoginCob* handler) override;
private:


};
#endif