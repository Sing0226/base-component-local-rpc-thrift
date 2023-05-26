/////////////////////////////////////////////////////////////////////////////////
/// Copyright (C), 2015, 
/// Rayshape Corporation. All rights reserved.
/// @file    authorizedinfojob.h
/// @author  Simone
/// @date    2023/04/07
/// @brief   
///
/// @history v0.01 2023/04/07  单元创建
/////////////////////////////////////////////////////////////////////////////////

#ifndef _R_AUTHRORIZED_INFO_JOB_H__
#define _R_AUTHRORIZED_INFO_JOB_H__

#include "threadpool/rbasejob.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////////
class RAuthorInfoJob : public RBaseJob
{
public:
	RAuthorInfoJob(const std::string& reqId,
		const int job_priority = 0);
	virtual ~RAuthorInfoJob();

public:
    virtual void initialize();

protected:

    virtual int _run();

    virtual void _finalize();

    virtual void _onCancelJob();

private:
	std::string m_reqId;
	bool m_authorStatus;
	int m_retCode;
	std::string m_authorInfo;
};

#endif

