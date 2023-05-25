#ifndef __RQC_IPC_SERVICES_NOTIFY_TASKS_H__
#define __RQC_IPC_SERVICES_NOTIFY_TASKS_H__

#include <memory>

class RIpcQcServicesCobClientWrap;

class NotfiyAuthorInfoTask : public QObject, public QRunnable
{
	Q_OBJECT
public:
	NotfiyAuthorInfoTask(std::shared_ptr<RIpcQcServicesCobClientWrap> spColClient,
		QObject* parent = nullptr);
	~NotfiyAuthorInfoTask();

	void run() override;

private:
	std::shared_ptr<RIpcQcServicesCobClientWrap> m_spCobClient;

	std::string m_authorId;
};

#endif // __RQC_IPC_SERVICES_NOTIFY_TASKS_H__
