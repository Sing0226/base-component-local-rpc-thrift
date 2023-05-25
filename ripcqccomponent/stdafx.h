#ifndef __RIPC_QC_STDAFX_H__
#define __RIPC_QC_STDAFX_H__

#include <QtCore/qglobal.h>
#ifdef Q_OS_WIN
// Windows / CRT
#include <atlbase.h>
#include <shlobj.h>
#include <dinput.h>
#include <atltypes.h>
#include <atlcom.h>
#include <windows.h>
#endif

// C++
#include <assert.h>
#ifndef ASSERT
#define ASSERT assert
#endif

// Qt
#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include "global.h"
#include "rslogger_declare.h"
#include "rslogging.h"
#include "rslog.h"

RIPC_QC_USE_NAMESPACE

#endif //__RIPC_QC_STDAFX_H__
