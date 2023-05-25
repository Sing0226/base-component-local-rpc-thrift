#include "ripcqcinterfaceproxy.h"
#include "utils/utils.h"
#include "ripcqcinterfaceimpl.h"
#include <QString>
#include "ripcsingleton.h"



IpcQcInterfaceProxy::IpcQcInterfaceProxy(IRsIpcClientInterface* ipcClientImpl)
	: IRsIpcClientInterface()
    , m_pClientImpl(ipcClientImpl)
{
	if (NULL == m_pClientImpl)
	{
		m_pClientImpl = new RIpcQcInterfaceImpl();
	}
}

IpcQcInterfaceProxy::~IpcQcInterfaceProxy()
{
	if (m_pClientImpl)
	{
		delete m_pClientImpl;
		m_pClientImpl = nullptr;
	}
}

void IpcQcInterfaceProxy::initClient()
{
	if (m_pClientImpl)
	{
		m_pClientImpl->initClient();
	}
}

void IpcQcInterfaceProxy::setClientNotify(rs_client_ipc::IRsIpcClientNotify* pIpcClientNotify)
{
	if (m_pClientImpl && pIpcClientNotify)
	{
		m_pClientImpl->setClientNotify(pIpcClientNotify);
	}
}

void IpcQcInterfaceProxy::userLogin(const std::string& userName, const std::string& userPwd)
{
	RSLOG_DEBUG << "IpcQcInterfaceProxy::userLogin";
	if (m_pClientImpl)
	{
		m_pClientImpl->userLogin(userName, userPwd);
	}
}

std::string IpcQcInterfaceProxy::getAuthorInfo()
{
	RSLOG_DEBUG << "IpcQcInterfaceProxy, getProductInfo";
	if (m_pClientImpl)
	{
		return m_pClientImpl->getAuthorInfo();
	}

	return "";
}

//////////////////////////////////////////////////////////////////////////

void initIpcClientInterfaceInstance(rs_client_ipc::IRsIpcClientNotify* pIpcClientNotify)
{
	if( nullptr == RIPCSingleton<IRsIpcClientInterface, IpcQcInterfaceProxy>::s_pMutex)
	{
		RIPCSingleton<IRsIpcClientInterface, IpcQcInterfaceProxy>::s_pMutex = new QMutex;
	}

	IRsIpcClientInterface* pScanInterfaceProxy = RIPCSingleton<IRsIpcClientInterface, IpcQcInterfaceProxy>::instance();
	pScanInterfaceProxy->setClientNotify(pIpcClientNotify);
	pScanInterfaceProxy->initClient();
}

IRsIpcClientInterface* getIpcClientInterfaceInstance()
{
	IRsIpcClientInterface* pScanInterfaceProxy = RIPCSingleton<IRsIpcClientInterface, IpcQcInterfaceProxy>::instance();

	return pScanInterfaceProxy;
}


void releaseIpcClientInterfaceInstance()
{
	RIPCSingleton<IRsIpcClientInterface, IpcQcInterfaceProxy>::destroy();

	if(RIPCSingleton<IRsIpcClientInterface, IpcQcInterfaceProxy>::s_pMutex)
	{
		delete RIPCSingleton<IRsIpcClientInterface, IpcQcInterfaceProxy>::s_pMutex;
		RIPCSingleton<IRsIpcClientInterface, IpcQcInterfaceProxy>::s_pMutex = nullptr;
	}
}