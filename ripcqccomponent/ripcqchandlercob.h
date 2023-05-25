#ifndef __RIPC_QC_HANDLER_COB_H_
#define __RIPC_QC_HANDLER_COB_H_

#include <QObject>
#include <thrift/Thrift.h>

namespace rs{
	namespace qc{
		class ResAuthorStatus;
		class ResQcAnaFileInfo;
		class ResPatientNum;
		class ResPlaneNum;
		class ResListPlaneInfo;
		class ResListPatientInfo;
	}
}

class RIpcHandlerCob : public QObject
{
	Q_OBJECT
public:
	void invoke()
	{
		QMetaObject::invokeMethod(this, "toInvoke", Qt::QueuedConnection);
	}
	Q_INVOKABLE void toInvoke()
	{
		this->onInvoked();
		this->deleteLater();
	}
	virtual void onInvoked() = 0;
};


template<typename T>
class RIpcHandlerCobT : public RIpcHandlerCob
{
public:
	typedef ::std::function<void(T const& _return)> CobType;
	explicit RIpcHandlerCobT(CobType& cob)
		: m_cob(cob) {}
	T m_result;
	void onInvoked() override
	{
		m_cob(m_result);
	}
private:
	CobType m_cob;
};


typedef RIpcHandlerCobT<rs::qc::ResAuthorStatus> RIpcQcResAuthorStatusCob;

typedef RIpcHandlerCobT<rs::qc::ResQcAnaFileInfo> RIpcQcResAnaFileCob;

typedef RIpcHandlerCobT<int32_t> RIpcQcLoginCob;

typedef RIpcHandlerCobT<rs::qc::ResPatientNum> RIpcQcPatientNumCob;

typedef RIpcHandlerCobT<rs::qc::ResPlaneNum> RIpcQcPlaneNumCob;

typedef RIpcHandlerCobT<rs::qc::ResListPlaneInfo> RIpcQcListPlaneInfoCob;

typedef RIpcHandlerCobT<rs::qc::ResListPatientInfo> RIpcQcListPatientInfoCob;

#endif


