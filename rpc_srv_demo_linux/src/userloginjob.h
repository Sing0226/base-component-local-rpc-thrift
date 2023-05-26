#ifndef _R_USER_LOGIN_JOB_H__
#define _R_USER_LOGIN_JOB_H__

#include "threadpool/rbasejob.h"
#include <string.h>
#include "ripcqcservice.h"

//////////////////////////////////////////////////////////////////////////
class RUserLoginJob : public RBaseJob
{
public:
	RUserLoginJob(const std::string& reqId, 
		const std::string& userName, 
		const std::string& userPwd,
		const int job_priority = 0);
	virtual ~RUserLoginJob();

public:
    virtual void initialize();

protected:

    virtual int _run();

    virtual void _finalize();

    virtual void _onCancelJob();

private:
	std::string m_reqId;
	std::string m_userName;
	std::string m_userPwd;
	int m_loginStatus;
	std::string m_loginInfo;
	int m_retCode;
};

#endif

