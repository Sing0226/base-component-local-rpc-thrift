#ifndef __IPC_DECLARE_H__
#define __IPC_DECLARE_H__
#include <QtCore/qglobal.h>

#ifndef RIPC_QC_EXPORT
#  ifndef RIPC_QC_NO_EXPORTS
#    ifdef WRIPC_QC_LIB
#       define RIPC_QC_EXPORT Q_DECL_EXPORT
#    else
#       define RIPC_QC_EXPORT
#    endif
#  else
#    define RIPC_QC_EXPORT
#  endif
#endif


#endif /* __IPC_DECLARE_H__ */
