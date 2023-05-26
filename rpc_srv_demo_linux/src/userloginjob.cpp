#include "userloginjob.h"
#include "rslogger_declare.h"
#include "rslogging.h"
#include "ripcqcserver.h"
#include <QApplication>
#include <QFileInfo>
#include <ctime>
#include <random>



///////////////////////////////////////////// RAuthorInfoJob ///////////////////////////////////////////////////
//  ��ʼ��
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

void RUserLoginJob::initialize()
{
	RSLOG_DEBUG << "initialize";
}

int RUserLoginJob::_run()
{
    int ret = 0;
	RSLOG_DEBUG << "run start ...";
	RSLOG_DEBUG << "name = " << m_userName.c_str() << ", pwd = " << m_userPwd.c_str();

	std::default_random_engine e;
	std::uniform_int_distribution<int> u(1, 1000); // ����ұ�����
	e.seed(time(0));
	if (u(e) % 2)
	{
		m_loginInfo = "��¼�ɹ�";
		m_loginStatus = 0;
	}		
	else
	{
		m_loginInfo = "��¼ʧ��";
		m_loginStatus = 1;
	}
		
	RSLOG_DEBUG << "run end";
    return ret;
}

void RUserLoginJob::_finalize()
{
	RSLOG_DEBUG << "finalize start ...";

	RIpcQcServer::getInstance().onLoginResult(m_reqId, m_loginStatus);
	char szMsg[10240];
    sprintf(szMsg, "��ȡlicense��Ϣ���: req id:: %s, info: %d\n",
		m_reqId.c_str(),
		m_loginStatus);
	RSLOG_DEBUG << szMsg;
	RSLOG_DEBUG << "finalize end";
}

void RUserLoginJob::_onCancelJob()
{
	RSLOG_DEBUG << "cancel job";
}
