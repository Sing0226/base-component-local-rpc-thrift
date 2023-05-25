#include "stdafx.h"
#include "ripcqcserviceshost.h"
#include "ripcqcserviceshostprivate.h"
#include "ripcqcservicesqcdataproxy.h"
#include "abstractrequestrunqctask.h"
#include "ripcqcservicesnotifytasks.h"


struct CallBackWrap
{
	::std::function<void()> m_cob;

	CallBackWrap(::std::function<void(RIpcQcServicesCobClientWrap* client)>& cob, RIpcQcServicesCobClientWrap* client)
		: m_cob(::std::bind(cob, client)) { }

	void operator()(rs::qc::ripcqcservicesCobClient* client)
	{
		m_cob();
	}
};

RIpcQcServicesHost::RIpcQcServicesHost(QObject* parent, RAbstractRequestRunQcTask* reqRunTask)
	: QObject(parent)
	, m_d(new RIpcQcServicesHostPrivate(this))
	, m_pServicesDataProxy(new RIpcServiceQcDataProxy(this))
	, m_pReqRunTask(reqRunTask)
	, m_bFirstNotifyAuthorInfo(false)
{

	connect(this, SIGNAL(sigNotifyAuthorStatusChanged(const QString&)),
		this, SLOT(onNotifyAuthorStatusChanged(const QString&)));
}

RIpcQcServicesHost::~RIpcQcServicesHost()
{
	if (m_d)
	{
		delete m_d;
		m_d = nullptr;
	}
}

::std::shared_ptr<RIPCServerConnection> 
	RIpcQcServicesHost::incomingConnection(quintptr desc)
{
	m_bFirstNotifyAuthorInfo = false;
	if (m_d)
		return m_d->incomingConnection(desc);
	else
		return ::std::shared_ptr<RIPCServerConnection>();
}

std::shared_ptr<RIpcQcServicesCobClientWrap>
	RIpcQcServicesHost::cobClient(quintptr desc)
{
	if (m_d)
		return m_d->cobClient(desc);
	else
		return std::shared_ptr<RIpcQcServicesCobClientWrap>();
}

void RIpcQcServicesHost::onNotifyAuthorStatusChanged(const QString& authorMsg)
{
	if (m_d)
	{
		std::string strAuthorMsg = authorMsg.toUtf8().data();
		std::map<quintptr, std::shared_ptr<RIpcQcServicesCobClientWrap>>& clients = m_d->getClients();
		std::map<quintptr, std::shared_ptr<RIpcQcServicesCobClientWrap>>::iterator it = clients.begin();
		for (; it != clients.end(); it++)
		{
			std::shared_ptr<RIpcQcServicesCobClientWrap> colCl = it->second;
			try
			{
				colCl->notifyAuthorizeStatusChanged([=](RIpcQcServicesCobClientWrap* client) {}, strAuthorMsg);
			}
			catch (std::exception& e)
			{
				RSLOG_ERROR << QString("onNotifyAuthorStatusChanged exception: %1").arg(e.what()).toLocal8Bit().data();
			}
		}
	}
}

void RIpcQcServicesHost::onServerAuthorStatus(const ripcqc::AuthorInfo& authorInfo)
{
	rs::qc::ResAuthorStatus resAuthorStatus;

	if (authorInfo.authorStatus)
	{
		resAuthorStatus.authorStatus = 1;
	}
	else
	{
		resAuthorStatus.authorStatus = 0;
	}
	QTextCodec* utf8Codec = QTextCodec::codecForName("utf-8");
	QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");
	QString strUnicode = gb2312Codec->toUnicode(authorInfo.authorInfo.c_str());
	QByteArray byteUTF8AuthorId = utf8Codec->fromUnicode(strUnicode);
	resAuthorStatus.strAutorInfo = byteUTF8AuthorId.data();


	if(!m_bFirstNotifyAuthorInfo /*|| 
		(resAuthorStatus.authorStatus != m_pServicesDataProxy->getAuthorStatus()->authorStatus)*/)
	{
		m_pServicesDataProxy->setServiceAuthorisationStatus(resAuthorStatus);
		emit sigNotifyAuthorStatusChanged(QString::fromUtf8(resAuthorStatus.strAutorInfo.c_str()));
		m_bFirstNotifyAuthorInfo = true;
	}	
}


void RIpcQcServicesHost::reqAuthorStatus(const std::string& reqId, RIpcQcResAuthorStatusCob* handler)
{
	int ret = m_pServicesDataProxy->setServiceAuthorInfoHandlerMap(reqId, handler);
	if (ret < 0)
	{
		RSLOG_DEBUG << "RIpcQcServicesHost::reqAuthorStatus setServiceAuthorInfoHandlerMap failed!";
		const rs::qc::ResAuthorStatus* pAuthorStatus = m_pServicesDataProxy->getAuthorStatus();
		handler->m_result.authorStatus = pAuthorStatus->authorStatus;
		handler->m_result.strAutorInfo = pAuthorStatus->strAutorInfo;
		handler->invoke();
	}
	else
	{
		if (m_pReqRunTask)
		{
			m_pReqRunTask->requestAuthorizedInfo(reqId);
		}
		else
		{
			RSLOG_ERROR << "RIpcQcServicesHost::reqAuthorStatus failed, req run task is nullptr";
		}
	}
}

void RIpcQcServicesHost::reqUserLogin(const std::string& userName, const std::string& userPwd, RIpcQcLoginCob* handler)
{

	std::string reqId = _createUUID();
	int ret = m_pServicesDataProxy->setServiceLoginInfoHandlerMap(reqId, handler);
	if (ret < 0)
	{
		RSLOG_DEBUG << "RIpcQcServicesHost::onGetAuthorStatus setServiceAnaDicomFileHandlerMap failed!";
		handler->m_result = (int32_t) rs::qc::ELoginErrCode::ERR_USER_UNDEFINED;
		handler->invoke();
	}
	else
	{
		if (m_pReqRunTask)
		{
			m_pReqRunTask->requestUserLogin(reqId, userName, userPwd);
		}
		else
		{
			RSLOG_ERROR << "RIpcQcServicesHost::onGetAuthorStatus failed, req run task is nullptr";
		}
	}
}

void RIpcQcServicesHost::onAuthorStatus(const std::string& reqId, const ripcqc::AuthorInfo& authorInfo)
{
	rs::qc::ResAuthorStatus resAuthorStatus;

	if (authorInfo.authorStatus)
	{
		resAuthorStatus.authorStatus = 1;
	}
	else
	{
		resAuthorStatus.authorStatus = 0;
	}
	QTextCodec* utf8Codec = QTextCodec::codecForName("utf-8");
	QTextCodec* gb2312Codec = QTextCodec::codecForName("gb2312");
	QString strUnicode = gb2312Codec->toUnicode(authorInfo.authorInfo.c_str());
	QByteArray byteUTF8AuthorId = utf8Codec->fromUnicode(strUnicode);
	resAuthorStatus.strAutorInfo = byteUTF8AuthorId.data();
	m_pServicesDataProxy->setServiceAuthorisationStatus(resAuthorStatus);

	RIpcQcResAuthorStatusCob* handler = m_pServicesDataProxy->getAuthorInfoCob(reqId);
	if (handler)
	{
		const rs::qc::ResAuthorStatus* pAuthorStatus = m_pServicesDataProxy->getAuthorStatus();
		handler->m_result.authorStatus = pAuthorStatus->authorStatus;
		handler->m_result.strAutorInfo = pAuthorStatus->strAutorInfo;
		handler->invoke();
	}
	else
	{
		RSLOG_ERROR << "RIpcQcServicesHost::onLoginResult hanlder is null pointer";
	}
}

void RIpcQcServicesHost::onLoginResult(const std::string& reqId, int loginStatus)
{
	RIpcQcLoginCob* handler = m_pServicesDataProxy->getLoginInfoCob(reqId);
	if (handler)
	{
		handler->m_result = (rs::qc::ELoginErrCode::type)loginStatus;
		handler->invoke();
	}
	else
	{
		RSLOG_ERROR << "RIpcQcServicesHost::onLoginResult hanlder is null pointer";
	}
}

std::string RIpcQcServicesHost::_createUUID()
{
	QString guid = QUuid::createUuid().toString().remove(QRegExp("\\{|\\}|\\-")).toUpper();
	return guid.toLocal8Bit().data();
}
