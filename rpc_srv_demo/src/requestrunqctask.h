
#include "abstractrequestrunqctask.h"
#include <QString>

class RThreadPool;
class RRequestRunTestTask : public RAbstractRequestRunQcTask
{
public:
    explicit RRequestRunTestTask(RThreadPool *threadpool);
	virtual ~RRequestRunTestTask();

public:
	virtual void requestAuthorizedInfo(const std::string& reqId) override;
	virtual void requestUserLogin(const std::string& reqId, const std::string& userName, const std::string& userPwd) override;
private:
	RThreadPool* m_pThreadPool;
};
