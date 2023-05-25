#include "userloginjob.h"
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
RUserLoginJob::RUserLoginJob(const std::string& reqId,
	const std::string& userName,
	const std::string& userPwd,
	const int job_priority)
	: RBaseJob(job_priority)
	, m_reqId(reqId)
	, m_userName(userName)
	, m_userPwd(userPwd)
	, m_loginStatus(0)
{
}

RUserLoginJob::~RUserLoginJob()
{
}

BOOL RUserLoginJob::initialize()
{
	BOOL bRet = FALSE;
	RSLOG_DEBUG << "initialize";
	return bRet;
}

BOOL RUserLoginJob::_run()
{
	BOOL bRet = FALSE;
	RSLOG_DEBUG << "run start ...";
	RSLOG_DEBUG << "name = " << m_userName.c_str() << ", pwd = " << m_userPwd.c_str();

	std::default_random_engine e;
	std::uniform_int_distribution<int> u(1, 1000); // 左闭右闭区间
	e.seed(time(0));
	if (u(e) % 2)
	{
		m_loginInfo = "登录成功";
		m_loginStatus = 0;
	}		
	else
	{
		m_loginInfo = "登录失败";
		m_loginStatus = 1;
	}
		
	RSLOG_DEBUG << "run end";
	return bRet;
}

VOID RUserLoginJob::_finalize()
{
	RSLOG_DEBUG << "finalize start ...";

	RIpcQcServer::getInstance().onLoginResult(m_reqId, m_loginStatus);
	char szMsg[10240];
	sprintf_s(szMsg, "获取license信息完成: req id:: %s, info: %d\n",
		m_reqId.c_str(),
		m_loginStatus);
	RSLOG_DEBUG << szMsg;
	RSLOG_DEBUG << "finalize end";
}

VOID RUserLoginJob::_onCancelJob()
{
	RSLOG_DEBUG << "cancel job";
}
