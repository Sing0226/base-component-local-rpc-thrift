#ifndef _R_AUTHRORIZED_INFO_JOB_H__
#define _R_AUTHRORIZED_INFO_JOB_H__

#include "threadpool/rbasejob.h"
#include <string.h>
#include "ripcqcservice.h"

//////////////////////////////////////////////////////////////////////////
class RAuthorInfoJob : public RBaseJob
{
public:
	RAuthorInfoJob(const std::string& reqId,
		const int job_priority = 0);
	virtual ~RAuthorInfoJob();

public:
	virtual BOOL initialize();

protected:

	virtual BOOL _run();

	virtual VOID _finalize();

	virtual VOID _onCancelJob();

private:
	std::string m_reqId;
	bool m_authorStatus;
	int m_retCode;
	std::string m_authorInfo;
};

#endif

