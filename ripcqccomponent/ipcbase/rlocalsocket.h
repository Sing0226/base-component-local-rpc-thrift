#ifndef _RLOCALSOCKET_H_
#define _RLOCALSOCKET_H_

#include "rlocalsocketdef.h"

class RLocalSocketImpl;
class RLocalSocket : public RLocalSocketBase
{
public:
	explicit RLocalSocket(IRLocalSocketNotify* pNotify);
	~RLocalSocket();

	void connectToServer(const wchar_t* name, OpenMode openMode);
	void disconnectFromServer();

	void setSocketDescriptor(void* socketDescriptor);
	void init();
	void term();

	LocalSocketState state() const;
	bool flush();
	int write(const char* buf, unsigned int len);
	bool waitForBytesWritten(int msecs = 3000);
	void read();

private:
	RLocalSocketImpl* m_pImpl;
};

#endif //_KLOCALSOCKET_H_