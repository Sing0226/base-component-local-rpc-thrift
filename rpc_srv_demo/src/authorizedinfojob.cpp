/////////////////////////////////////////////////////////////////////////////////
/// Copyright (C), 2023, 
/// Richinfo Corporation. All rights reserved.
/// @file    autotizedinfojob.cpp
/// @author  Simone
/// @date    2023/03/29
/// @brief   
///
/// @history v0.01 2023/03/29  单元创建
/////////////////////////////////////////////////////////////////////////////////

#include "authorizedinfojob.h"
#include "rslogger_declare.h"
#include "rslog.h"
#include "rslogging.h"
#include "ripcqcserver.h"
#include <QApplication>
#include <QFileInfo>
#include "ripcqcservice.h"
#include <ctime>
#include <random>

///////////////////////////////////////////// RAuthorInfoJob ///////////////////////////////////////////////////
//  初始化
RAuthorInfoJob::RAuthorInfoJob(const std::string& reqId,
	int job_priority)
	: RBaseJob(job_priority)
	, m_reqId(reqId)
	, m_retCode(ripcqc::RIpcQcErrorCode::RIpcQcErrorNoError)
	, m_authorStatus(false)
{
}

RAuthorInfoJob::~RAuthorInfoJob()
{
}

BOOL RAuthorInfoJob::initialize()
{
	BOOL bRet = FALSE;
	RSLOG_DEBUG << "initialize";
	return bRet;
}

BOOL RAuthorInfoJob::_run()
{
	BOOL bRet = FALSE;
	RSLOG_DEBUG << "run start ...";
	char szMsg[1024];
	sprintf_s(szMsg, "正在获取license信息: req id: %s", m_reqId.c_str());

	std::default_random_engine e;
	std::uniform_int_distribution<int> u(1, 1000); // 左闭右闭区间
	e.seed(time(0));
	int num = u(e);
	QString authorInfo;
	if (u(e) % 3)
	{
		QString authorInfo = QString::fromLocal8Bit("license 剩余天数:  %1").arg(num);
		m_authorStatus = true;
	}
	else
	{
		QString authorInfo = QString::fromLocal8Bit("系统未授权，请联系厂商提供授权信息!");
		m_authorStatus = false;
	}
	m_authorInfo = authorInfo.toLocal8Bit();
	RSLOG_DEBUG << szMsg;
	RSLOG_DEBUG << "run end";
	return bRet;
}

VOID RAuthorInfoJob::_finalize()
{
	RSLOG_DEBUG << "finalize start ...";
	ripcqc::AuthorInfo authorInfo;
	authorInfo.authorStatus = m_authorStatus;
	QString authorInfoMsg = QString::fromLocal8Bit(m_authorInfo.c_str());
	QString errMsg = QString::fromLocal8Bit("{\"errCode\":%1, \"errMsg\": \"%2\"}")
		.arg(m_retCode).arg(authorInfoMsg);
	authorInfo.authorInfo = errMsg.toLocal8Bit().data();

	RIpcQcServer::getInstance().onRunAuthorInfoTaskFinish(m_reqId, authorInfo);
	char szMsg[10240];
	sprintf_s(szMsg, "获取license信息完成: req id:: %s, info: %s\n",
		m_reqId.c_str(),
		authorInfo.authorInfo.c_str());
	RSLOG_DEBUG << szMsg;
	RSLOG_DEBUG << "finalize end";
}

VOID RAuthorInfoJob::_onCancelJob()
{
	RSLOG_DEBUG << "cancel job";
}
