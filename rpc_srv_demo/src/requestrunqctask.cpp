#include "requestrunqctask.h"
#include "rslogger_declare.h"
#include "rslog.h"
#include "rslogging.h"
#include "threadpool/rthreadpool.h"
#include "threadpool/rbasejob.h"
#include "authorizedinfojob.h"
#include "userloginjob.h"



RRequestRunTestTask::RRequestRunTestTask(RThreadPool* threadpool)
	: RAbstractRequestRunQcTask()
    , m_pThreadPool(threadpool)
{
    RSLOG_DEBUG << "construct RRequestRunQcTask";
}

RRequestRunTestTask::~RRequestRunTestTask()
{
    RSLOG_DEBUG << "desconstruct ~RRequestRunQcTask";
}


void RRequestRunTestTask::requestAuthorizedInfo(const std::string& reqId)
{
    RSLOG_DEBUG << "requestAuthorizedInfo";
    RBaseJob* pJob = new RAuthorInfoJob(reqId);
    m_pThreadPool->submitJob(pJob);
    RSLOG_DEBUG << "requestAuthorizedInfo end";
}

void RRequestRunTestTask::requestUserLogin(const std::string& reqId, const std::string& userName, const std::string& userPwd)
{
    RSLOG_DEBUG << "requestUserLogin";
    RBaseJob* pJob = new RUserLoginJob(reqId, userName, userPwd);
    m_pThreadPool->submitJob(pJob);
    RSLOG_DEBUG << "requestUserLogin end";
}

