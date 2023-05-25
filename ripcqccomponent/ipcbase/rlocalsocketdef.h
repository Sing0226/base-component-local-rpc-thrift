#ifndef _RS_LOCALSOCKETDEF_H_
#define _RS_LOCALSOCKETDEF_H_

#include "ipc_error.h"

static const int RsLocalSocketBufferLength = (1 << 18); // 256K

struct IRLocalSocket
{
	virtual ~IRLocalSocket() {};
	virtual void connectToServer(const wchar_t* name, OpenMode openMode) = 0;
	virtual void disconnectFromServer() = 0;
	virtual void init() = 0;
	virtual void term() = 0;
	virtual LocalSocketState state() const = 0;
	virtual bool flush() = 0;
	virtual int write(const char* buf, unsigned int len) = 0;
	virtual bool waitForBytesWritten(int msecs = 30000) = 0;
};

struct IRLocalSocketNotify 
{
	virtual void onConnected() = 0;
	virtual void onDisconnected() = 0;
	virtual void onError(LocalSocketError socketError) = 0;
	virtual void onStateChanged(LocalSocketState socketState) = 0;
	virtual void onRecvData(char* buf, unsigned int len) = 0;
};

class RLocalSocketBase 
	: public IRLocalSocket
	, protected IRLocalSocketNotify
{
public:
	RLocalSocketBase(IRLocalSocketNotify* pNotify) : m_pNotify(pNotify)
	{
	}
	virtual ~RLocalSocketBase()
	{
		m_pNotify = NULL;
	}

protected:
	virtual void onConnected() { if (m_pNotify) m_pNotify->onConnected(); }
	virtual void onDisconnected() { if (m_pNotify) m_pNotify->onDisconnected(); }
	virtual void onError(LocalSocketError error) { if (m_pNotify) m_pNotify->onError(error); }
	virtual void onStateChanged(LocalSocketState state) { if (m_pNotify) m_pNotify->onStateChanged(state); }
	virtual void onRecvData(char* data, unsigned int len) { if (m_pNotify) m_pNotify->onRecvData(data, len); }

protected:
	IRLocalSocketNotify* m_pNotify;
};

#endif //_KXLOCALSOCKETDEF_H_