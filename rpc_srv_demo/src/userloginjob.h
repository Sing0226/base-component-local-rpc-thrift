/////////////////////////////////////////////////////////////////////////////////
/// Copyright (C), 2015, 
/// Rayshape Corporation. All rights reserved.
/// @file    ranalysedicomjob.h
/// @author  Simone
/// @date    2023/04/11
/// @brief   
///
/// @history v0.01 2023/04/11  ��Ԫ����
/////////////////////////////////////////////////////////////////////////////////

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
	virtual BOOL initialize();

protected:

	virtual BOOL _run();

	virtual VOID _finalize();

	virtual VOID _onCancelJob();

private:
	std::string m_reqId;
	std::string m_userName;
	std::string m_userPwd;
	int m_loginStatus;
	std::string m_loginInfo;
	int m_retCode;
};

#endif
