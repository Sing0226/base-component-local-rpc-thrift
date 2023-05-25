#ifndef __RIPC_QC_GLOBAL_H__
#define __RIPC_QC_GLOBAL_H__

#include <QtCore/qglobal.h>

#include "ipc_declare.h"

#define QC_IPC_SERVER_NAME "Qc_Ipc_Server_{448550F3-B0C8-4FB3-949B-25D1029DFABE}"

namespace ripcqc {}

#define RIPC_QC_BEGIN_NAMESPACE namespace ripcqc {
#define RIPC_QC_END_NAMESPACE }
#define RIPC_QC_USE_NAMESPACE using namespace ripcqc;

#endif//__RIPC_QC_GLOBAL_H__
