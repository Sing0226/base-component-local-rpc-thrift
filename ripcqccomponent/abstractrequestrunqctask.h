#ifndef _RS_ABSTRACT_REQUEST_RUN_QC_TASK_H__
#define _RS_ABSTRACT_REQUEST_RUN_QC_TASK_H__
#include <string>
#include "ripcqcservice.h"

class RAbstractRequestRunQcTask
{
public:
	RAbstractRequestRunQcTask(){};
	virtual ~RAbstractRequestRunQcTask(){};

public:
	virtual void requestAuthorizedInfo(const std::string& reqId) = 0;
	virtual void requestUserLogin(const std::string& reqId, const std::string& userName, const std::string& userPwd) = 0;
};

#endif