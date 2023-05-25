#include "stdafx.h"
#include "ripcqcservicesnotifytasks.h"
#include "ripcqcserviceshostprivate.h"

NotfiyAuthorInfoTask::NotfiyAuthorInfoTask(std::shared_ptr<RIpcQcServicesCobClientWrap> spColClient, QObject* parent)
	: m_spCobClient(spColClient)
	, QObject(parent)
{
	this->setAutoDelete(false);
}

NotfiyAuthorInfoTask::~NotfiyAuthorInfoTask()
{
	RSLOG_DEBUG << "desconstruct NotfiyAuthorInfoTask";
}

void NotfiyAuthorInfoTask::run()
{
	try
	{
		m_spCobClient->notifyAuthorizeStatusChanged([=](RIpcQcServicesCobClientWrap* client) {}, m_authorId);
	}
	catch (std::exception& e)
	{
		RSLOG_ERROR  << QString("recv_notifyAuthorizeStatusChanged exception: %1").arg(e.what()).toLocal8Bit().data();
	}
}

