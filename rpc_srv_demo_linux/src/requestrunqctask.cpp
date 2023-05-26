#include "requestrunqctask.h"
#include "rslogger_declare.h"
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
    int jobId  = 0;
    m_pThreadPool->submitJob(pJob, &jobId);
    RSLOG_DEBUG << "submit job id = " << jobId;
    RSLOG_DEBUG << "requestAuthorizedInfo end";
}

void RRequestRunTestTask::requestUserLogin(const std::string& reqId, const std::string& userName, const std::string& userPwd)
{
    RSLOG_DEBUG << "requestUserLogin";
    RBaseJob* pJob = new RUserLoginJob(reqId, userName, userPwd);
    int jobId  = 0;
    m_pThreadPool->submitJob(pJob, &jobId);
    RSLOG_DEBUG << "submit job id = " << jobId;
    RSLOG_DEBUG << "requestUserLogin end";
}

