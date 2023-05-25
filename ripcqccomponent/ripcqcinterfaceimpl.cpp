#include "stdafx.h"
#include "ripcqcinterfaceimpl.h"
#include "ripcqcservicescobclient.h"
#include "ripcqcclient.h"
#include <QUuid>
#include <QRegExp>


RIpcQcInterfaceImpl::RIpcQcInterfaceImpl(rs_client_ipc::IRsIpcClientNotify* pIpcClientNotify)
	: IRsIpcClientInterface()
	, m_pClientNotify(pIpcClientNotify)
	, m_bConnectStatus(false)
{
	m_pIpcClient = new RIpcQcClient();
	m_pIpcClient->initCobClient();
}

RIpcQcInterfaceImpl::~RIpcQcInterfaceImpl()
{
	if (m_pIpcClient)
	{
		m_pIpcClient->unresgisterIpcClientNotify(m_pClientNotify);
		delete m_pIpcClient;
		m_pIpcClient = nullptr;
	}
}

void RIpcQcInterfaceImpl::initClient()
{
	// 连接server
	_connectToServer();
}

std::shared_ptr<RIpcQcServicesCobClient> RIpcQcInterfaceImpl::cobClient()
{
	return m_pIpcClient->cobClient();
}

void RIpcQcInterfaceImpl::_connectToServer()
{
	QString qstrServerName = QC_IPC_SERVER_NAME;
	m_pIpcClient->connectToServer(qstrServerName);
}

QString RIpcQcInterfaceImpl::_createUUID()
{
	return QUuid::createUuid().toString().remove(QRegExp("\\{|\\}|\\-")).toUpper();
}

void RIpcQcInterfaceImpl::setClientNotify(rs_client_ipc::IRsIpcClientNotify* pIpcClientNotify)
{
	if (NULL != pIpcClientNotify)
	{
		m_pClientNotify = pIpcClientNotify;
		m_pIpcClient->resgisterIpcClientNotify(m_pClientNotify);
	}	
}

rs_client_ipc::IRsIpcClientNotify* RIpcQcInterfaceImpl::getClientNotify()
{
	return m_pClientNotify;
}

std::string RIpcQcInterfaceImpl::getAuthorInfo()
{
	RSLOG_DEBUG << "RIpcQcInterfaceImpl, getAuthorInfo";

	auto service = cobClient();

	bool bServiceStatus = service && service->getChannel() && service->getChannel()->good();
	if (!bServiceStatus)
	{
		RSLOG_ERROR << "getAuthorInfo error";
		return "";
	}
	
	QString reqId = _createUUID();
	try
	{
		std::string strReqId = reqId.toLocal8Bit().data();
		RIpcQcInterfaceImpl* pIpcImpl = this;
		service->queryAuthorStatus(
			[=](RIpcQcServicesCobClient* client)
		{
			try {
				rs::qc::ResAuthorStatus resAuthorStatus;
				service->recv_queryAuthorStatus(resAuthorStatus);
				service->setAuthorStatus(resAuthorStatus);
				if (pIpcImpl->getClientNotify())
				{
					rs_client_ipc::AuthorInfo authorInfo;
					authorInfo.authorStatus = resAuthorStatus.authorStatus;
					authorInfo.authorInfo = resAuthorStatus.strAutorInfo;
					pIpcImpl->getClientNotify()->notifyGetAuthorInfo(authorInfo);
				}				
			}
			catch (std::exception& x) {
				QString a = x.what();
				RSLOG_ERROR << QString("recv_queryAuthorStatus exception: %1").arg(x.what()).toLocal8Bit().data();
			}
		}, strReqId);

	}
	catch (std::exception & x)
	{
		QString a = x.what();
		RSLOG_ERROR << QString("getAuthorInfo exception: %1").arg(x.what()).toLocal8Bit().data();
	}

	return reqId.toLocal8Bit().data();
}

void RIpcQcInterfaceImpl::userLogin(const std::string& userName, const std::string& userPwd)
{
	RSLOG_DEBUG << "RIpcQcInterfaceImpl::userLogin";
	auto service = cobClient();

	bool bServiceStatus = service && service->getChannel() && service->getChannel()->good();
	if (!bServiceStatus)
	{
		RSLOG_ERROR << "getAuthorInfo error";
		return;
	}

	try
	{
		RIpcQcInterfaceImpl* pIpcImpl = this;
		service->userLogin(
			[=](RIpcQcServicesCobClient* client)
			{
				try {
					rs::qc::ELoginErrCode::type err;
					err = (rs::qc::ELoginErrCode::type) service->recv_userLogin();
					if (pIpcImpl->getClientNotify())
					{
						LoginErrCode implErr = (LoginErrCode)err;
						pIpcImpl->getClientNotify()->notifyUserLoginStatus(implErr);
					}
				}
				catch (std::exception& x) {
					QString a = x.what();
					RSLOG_ERROR << QString("recv_userLogin exception: %1").arg(x.what()).toLocal8Bit().data();
				}
			}, userName, userPwd);

	}
	catch (std::exception& x)
	{
		QString a = x.what();
		RSLOG_ERROR << QString("getAuthorInfo exception: %1").arg(x.what()).toLocal8Bit().data();
	}
}