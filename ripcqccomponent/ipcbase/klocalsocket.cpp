#include "stdafx.h"
#include "rs_localsocket.h"

#include <string>
#include <cstdint>
#include <vector>
#include <cassert>
#include <sstream>

#ifdef Q_OS_WIN
#include <windows.h>
#include <process.h>
#else
#include<kern/typedef.h>
#endif

#include <QObject>

namespace
{
	class RsIPCDebug
	{
	public:
		enum IpcDebugLevel
		{
			IpcDebugInfo = 0,
			IpcDebugWarning,
			IpcDebugError,
		};
		RsIPCDebug(IpcDebugLevel level = IpcDebugInfo)
			: m_debugLevel(level)
		{
		}
		~RsIPCDebug()
		{
			std::wstring string = stream.str();
			switch (m_debugLevel)
			{
			case IpcDebugError:
				{
					QString msg = QString("RsLocalSocket: %1").arg(QString::fromStdWString(string));
					KWpsDspsIpcUtils::writeLog(msg);
				}
				break;
			case IpcDebugWarning:
				{
					QString msg = QString("RsLocalSocket: %1").arg(QString::fromStdWString(string));
					KWpsDspsIpcUtils::writeLog(msg);
				}
				break;
			default:
				// 目前Info日志默认存盘导致日志文件过大，暂时屏蔽掉KLocalSocke的Info日志
				break;
			}
		}

		RsIPCDebug& operator<< (const wchar_t* t)
		{
			stream << std::wstring(t);
			stream << L" ";
			return *this;
		}

		RsIPCDebug& operator<< (uint64_t t)
		{
			stream << t;
			stream << L" ";
			return *this;
		}
		std::wstringstream stream;
		IpcDebugLevel m_debugLevel;
	};

	#define RsIPCDebugInfo RsIPCDebug(RsIPCDebug::IpcDebugInfo)
	#define RsIPCDebugWarning RsIPCDebug(RsIPCDebug::IpcDebugWarning)
	#define RsIPCDebugError RsIPCDebug(RsIPCDebug::IpcDebugError)
}

namespace {

const DWORD DEF_WAIT_TIMEOUT = 3000;
const LONG MAX_SEM_COUNT = (1<<10); // 1024

enum /* WindowsMessageType */
{
	PIPE_WRITE_MSG = (WM_USER + 100),
	PIPE_CONNECTED_MSG = (WM_USER + 101),
	PIPE_DISCONNECTED_MSG = (WM_USER + 102),
	PIPE_ERROR_MSG = (WM_USER + 103),
	PIPE_STATECHANGED_MSG = (WM_USER + 104),
	PIPE_RECV_MSG = (WM_USER + 105)
};

}


// NOTE 调用 popMessage(或者 pushMessage)前, 先检查 isEmpty (isFull)
class RsLocalSocketRingBuffer
{
	enum
	{
		RingBufferCapacity = MAX_SEM_COUNT,
	};
public:
	RsLocalSocketRingBuffer() : m_start(0), m_end(0), m_messages(RingBufferCapacity, "") {}

	void init()
	{
		 m_start = 0;
		 m_end = 0;
	}
	void term()
	{
		m_messages.swap(std::vector<std::string>(RingBufferCapacity, "")); // Release all strings
	}

	// PRE !isFull
	void pushMessage(const char* data, unsigned int len)
	{
		m_messages[m_end % RingBufferCapacity].swap(std::string(data, len));
		uint32_t r = InterlockedIncrement(&m_end);
		assert(r == m_end);
	}

	// PRE !isEmpty
	std::string popMessage()
	{
		assert(m_start < m_end);
		std::string message = m_messages.at(m_start % RingBufferCapacity);
		uint32_t r = InterlockedIncrement(&m_start);
		assert(r == m_start);
		return message;
	}

	bool isEmpty() const
	{
		return m_start == m_end;
	}

	bool isFull() const
	{
		return size() == RingBufferCapacity;
	}

	size_t size() const
	{
		return m_end - m_start;
	}

private:
	RsLocalSocketRingBuffer(const RsLocalSocketRingBuffer&);
	RsLocalSocketRingBuffer& operator=(const RsLocalSocketRingBuffer&);
private:
	std::vector<std::string> m_messages;
	volatile uint32_t m_start;
	volatile uint32_t m_end;
};

typedef RsLocalSocketRingBuffer RsLocalSocketMessageQueue;

class RsLocalSocketImpl
{
	enum _WorkingState
	{
		InitState = 0,
		WaitWrittenState = 0x1,
		WaitReadingState = 0x2,
		QuitState = 0x4,
		OnConnectingState = 0x8,
	};

	struct PipeInstanceForRead
	{
		OVERLAPPED oOverlap;
		void* lpPointer;
		HANDLE hPipeInst;
		BYTE  cbRead[RsLocalSocketBufferLength];
		DWORD dwRead;
		volatile bool bReading;
		volatile bool bSuccess;
	};
	struct PipeInstanceForWrite
	{
		OVERLAPPED oOverlap;
		void* lpPointer;
		HANDLE hPipeInst;
		BYTE  cbWrite[RsLocalSocketBufferLength];
		DWORD dwWrite;
		DWORD dwTotalWritten;
		volatile bool bWritting;
		volatile bool bSuccess;
	};
	enum _ENOTIFYHANDLE
	{
		NotifyQuit = 0,
		NotifyConnecting,
		NotifyWaitWritten,
		NotifyRead,
		NotifyMax,
	};

	static void WINAPI completedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap);
	static void WINAPI completedWriteRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap);
	static LRESULT WINAPI notifyWndProc( HWND hwnd, UINT uMsg, WPARAM w, LPARAM l);
	static unsigned WINAPI workingFunc(void* param);

	static int ioCompletHandle(RsLocalSocketImpl* pThis)
	{
		int retState = RsLocalSocketImpl::InitState;
		if (pThis->isReadSucceeded())
		{
			if (!pThis->isReading())
			{
				retState |= RsLocalSocketImpl::WaitReadingState;
				RsIPCDebugInfo << L"RsLocalSocketImpl::workingFunc WAIT_IO_COMPLETION WaitReadingState";
				
			}
			else
			{
				RsIPCDebugInfo << L"RsLocalSocketImpl::workingFunc WAIT_IO_COMPLETION isReading";
			}
		}
		else
		{
			RsIPCDebugInfo << L"RsLocalSocketImpl::workingFunc WAIT_IO_COMPLETION !isReadSucceeded";
		}

		if (pThis->isWriteSucceeded())
		{
			if (!pThis->isWritting())
			{
				retState |= RsLocalSocketImpl::WaitWrittenState;
				RsIPCDebugInfo << L"RsLocalSocketImpl::workingFunc WAIT_IO_COMPLETION WaitWrittenState";
			}
			else
			{
				RsIPCDebugInfo << L"RsLocalSocketImpl::workingFunc WAIT_IO_COMPLETION isWritting";
			}
		}
		else
		{
			RsIPCDebugInfo << L"RsLocalSocketImpl::workingFunc WAIT_IO_COMPLETION !isWriteSucceeded";
		}

		return retState;
	}

public:
	RsLocalSocketImpl()
		: m_hPipe(INVALID_HANDLE_VALUE)
		, m_hThread(INVALID_HANDLE_VALUE)
		, m_hNotifyWnd(NULL)
		, m_dwThreadID(-1)
		, m_pPipeInsRead(NULL)
		, m_pPipeInsWrite(NULL)
		, m_state(UnconnectedState)
		, m_pNotify(NULL)
		, m_nWrittingCount(0)
		, m_pWrittenQueue(new RsLocalSocketMessageQueue)
	{
		for (int i = 0; i < NotifyMax; i++)
			m_notifyHandlers[i] = INVALID_HANDLE_VALUE;
		m_notifyConnected = INVALID_HANDLE_VALUE;
		RsIPCDebugInfo << L"RsLocalSocketImpl";
	}
	~RsLocalSocketImpl()
	{
		delete m_pWrittenQueue;
	}

	void init(IRsLocalSocketNotify* pNotify)
	{
		if (m_pPipeInsRead != NULL)
		{
			return;
		}
		m_pWrittenQueue->init();
		m_pNotify = pNotify;
		m_state = UnconnectedState;
		m_openMode = 0;
		m_fullServerName = L"";

		m_hNotifyWnd = ::CreateWindowW(L"STATIC", L"", WS_POPUP, 0, 0, 0, 0, HWND_MESSAGE, NULL, ::GetModuleHandleW(NULL), NULL);
		if (!m_hNotifyWnd)
		{
			RsIPCDebugError << L"m_hNotifyWnd is NULL" << ::GetLastError();
		}

#if _WIN64
		::SetWindowLongPtr(m_hNotifyWnd, GWLP_WNDPROC, (LONG_PTR)&RsLocalSocketImpl::notifyWndProc);
#else
		::SetWindowLongW(m_hNotifyWnd, GWL_WNDPROC, (LONG)&KLocalSocketImpl::notifyWndProc);
#endif

		m_notifyHandlers[NotifyQuit] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		m_notifyHandlers[NotifyWaitWritten] = ::CreateSemaphore(NULL, 0, MAX_SEM_COUNT, NULL);
		m_notifyHandlers[NotifyRead] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		m_notifyHandlers[NotifyConnecting] = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		m_notifyConnected = ::CreateEvent(NULL, FALSE, FALSE, NULL);

		m_pPipeInsRead = (PipeInstanceForRead*)::GlobalAlloc(GPTR, sizeof(PipeInstanceForRead)); 
		if (m_pPipeInsRead) 
		{
			m_pPipeInsRead->lpPointer = this;
			::ZeroMemory( &m_pPipeInsRead->oOverlap, sizeof( OVERLAPPED ) );
			m_pPipeInsRead->hPipeInst = NULL;
			::ZeroMemory( m_pPipeInsRead->cbRead, RsLocalSocketBufferLength);
			m_pPipeInsRead->dwRead = 0;
			m_pPipeInsRead->bReading = FALSE;
			m_pPipeInsRead->bSuccess = TRUE;
		}

		m_pPipeInsWrite = (PipeInstanceForWrite*)::GlobalAlloc(GPTR, sizeof(PipeInstanceForWrite));
		if (m_pPipeInsWrite)
		{
			m_pPipeInsWrite->lpPointer = this;
			::ZeroMemory( &m_pPipeInsWrite->oOverlap, sizeof(OVERLAPPED));
			m_pPipeInsWrite->hPipeInst = NULL;
			::ZeroMemory( m_pPipeInsWrite->cbWrite, RsLocalSocketBufferLength);
			m_pPipeInsWrite->dwWrite = 0;
			m_pPipeInsWrite->dwTotalWritten = 0;
			m_pPipeInsWrite->bWritting = FALSE;
			m_pPipeInsWrite->bSuccess = TRUE;
		}

		m_hThread = (HANDLE)_beginthreadex(NULL, 0, 
			&RsLocalSocketImpl::workingFunc,
			this, 0, 
			(unsigned int*)&m_dwThreadID
			);
		::SetThreadPriority(m_hThread, THREAD_PRIORITY_ABOVE_NORMAL);
	}
	void term()
	{
		if (m_hThread == INVALID_HANDLE_VALUE)
		{
			return;
		}
		RsIPCDebugWarning << L"RsLocalSocketImpl::term()";
		::SetEvent(m_notifyHandlers[NotifyQuit]);
		if (WAIT_OBJECT_0 !=::WaitForSingleObject(m_hThread, DEF_WAIT_TIMEOUT))
		{
			::TerminateThread(m_hThread, -1);
		}
		m_hThread = INVALID_HANDLE_VALUE;
		m_pWrittenQueue->term();

		for (int i = 0; i < NotifyMax; i++)
		{
			::CloseHandle(m_notifyHandlers[i]);
			m_notifyHandlers[i] = INVALID_HANDLE_VALUE;
		}

		::CloseHandle(m_notifyConnected);
		m_notifyConnected = INVALID_HANDLE_VALUE;

		::DestroyWindow(m_hNotifyWnd);
		m_hNotifyWnd = NULL;

		::GlobalFree(m_pPipeInsRead);
		m_pPipeInsRead = NULL;
		::GlobalFree(m_pPipeInsWrite);
		m_pPipeInsWrite = NULL;
		m_state = UnconnectedState;
		m_pNotify = NULL;
		m_fullServerName.clear();
		m_openMode = 0;
	}

	void connectToServer(const wchar_t* name, OpenMode openMode)
	{
		if (state() == ConnectedState || state() == ConnectingState)
		{
			return;
		}

		if (!m_hNotifyWnd)
		{
			RsIPCDebugError << L"RsLocalSocketImpl::connectToServer(): m_hNotifyWnd is NULL" <<  GetLastError();
			return;
		}

		if (name == NULL || ::wcslen(name) == 0)
		{
			onStateChanged(UnconnectedState);
			onError(ServerNotFoundError);
			return;
		}
		LPCWSTR pipePath = L"\\\\.\\pipe\\";
		if (::wcsstr(name, pipePath) == name)
		{
			m_fullServerName = name;
		}
		else
		{
			m_fullServerName = pipePath;
			m_fullServerName += name;
		}
		m_openMode = openMode;

		onStateChanged(ConnectingState);
		::SetEvent(m_notifyHandlers[NotifyConnecting]);
	}

	void disconnectFromServer()
	{
		if (m_hPipe == INVALID_HANDLE_VALUE)
		{
			return;
		}

		waitForConnecting();
		
		RsIPCDebugWarning << L"RsLocalSocketImpl::disconnectFromServer()";
		onStateChanged(ClosingState);
		::DisconnectNamedPipe(m_hPipe);
		::CloseHandle(m_hPipe);
		m_hPipe = INVALID_HANDLE_VALUE;

		m_pPipeInsRead->hPipeInst = NULL;
		m_pPipeInsRead->bSuccess = FALSE;
		m_pPipeInsWrite->hPipeInst = NULL;
		m_pPipeInsWrite->bSuccess = FALSE;
		onStateChanged(UnconnectedState);
		onDisconnected();
	}

	void setSocketDescriptor(HANDLE socketDescriptor)
	{
		if (state() == ConnectedState || state() == ConnectingState)
		{
			return;
		}

		m_hPipe = socketDescriptor;
		m_pPipeInsRead->hPipeInst = m_hPipe;
		m_pPipeInsRead->bSuccess = TRUE;
		m_pPipeInsWrite->hPipeInst = m_hPipe;
		m_pPipeInsWrite->bSuccess = TRUE;

		onStateChanged(ConnectedState);
		onConnected();
	}

	LocalSocketState state() const
	{
		return m_state;
	}

	bool isValid() const
	{
		return state() == ConnectingState || state() == ConnectedState;
	}
	
	bool isConnecting() const
	{
		return state() == ConnectingState;
	}

	bool flush()
	{
		return true;
	}
	int write(const char* buf, unsigned int len)
	{
		if (isValid())
		{
			if (!m_pWrittenQueue->isFull())
			{
				m_pWrittenQueue->pushMessage(buf, len);
				if (!::ReleaseSemaphore(m_notifyHandlers[NotifyWaitWritten], 1, nullptr))
				{
					int error = ::GetLastError();
					RsIPCDebugError << L"write() ::ReleaseSemaphore" <<  error;
					// FIXME message is enqueued but fails to notify
				}
				return len;
			}
			else
			{
				RsIPCDebugInfo << L"write() queue is Full";
				return -1;

			}
		}
		else
		{
			RsIPCDebugWarning << L"write(): Not isValid";
			return -1;
		}
	}

	bool waitForBytesWritten(unsigned int msecs = 3000)
	{
		DWORD start = ::GetTickCount();
		DWORD end = ::GetTickCount();
		bool done = false;
		while (!done)
		{
			if (end - start > msecs)
			{
				break;
			}
			done = m_pWrittenQueue->isEmpty();
			end = ::GetTickCount();
			if (done)
			{
				break;
			}
			if (end - start > msecs)
			{
				break;
			}
			::Sleep(10); // GetTickCount's resolution is 10 to 16 msecs
			end = ::GetTickCount();
		}
		RsIPCDebugInfo << L"waitForBytesWritten ::Sleep" << (end - start);
		return done;
	}

	void read()
	{
		::SetEvent(m_notifyHandlers[NotifyRead]);
	}

	bool waitForConnecting()
	{
		if (!isConnecting())
			return false;

		if (WAIT_OBJECT_0 == ::WaitForSingleObject(m_notifyConnected, DEF_WAIT_TIMEOUT))
			return true;

		return false;
	}
	
private:
	void onConnected() const
	{
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, PIPE_CONNECTED_MSG, (WPARAM)m_pNotify, 0);
		}
		::SetEvent(m_notifyConnected);
	}
	void onDisconnected() const
	{
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, PIPE_DISCONNECTED_MSG, (WPARAM)m_pNotify, 0);
		}
		::ResetEvent(m_notifyConnected);
	}
	void onError(LocalSocketError socketError) const
	{
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, PIPE_ERROR_MSG, (WPARAM)m_pNotify, (LPARAM)socketError);
		}
	}
	void onStateChanged(LocalSocketState state) const
	{
		if (m_state == state)
		{
			return;
		}
		m_state = state;
		if (m_hNotifyWnd)
		{
			::PostMessage(m_hNotifyWnd, PIPE_STATECHANGED_MSG, (WPARAM)m_pNotify, (LPARAM)state);
		}
	}
	void onRecvData(char* buf, unsigned int len)
	{
		if (m_hNotifyWnd)
		{
#ifdef RS_IPC_CLIENT_CHANNEL_DEBUG_DUMP
			QByteArray bytes(buf, len);
			RsIPCDebugInfo << L"RsLocalSocketImpl::onRecvData::PostMessage" << L"-buf" << QString(bytes.toHex()).utf16() << L"-len" << len;
#endif

			BSTR data = ::SysAllocStringByteLen(buf, len);
			::PostMessage(m_hNotifyWnd, PIPE_RECV_MSG, (WPARAM)m_pNotify, (LPARAM)data);
		}
	}
public:
	void setError(LocalSocketError error = UnknownSocketError)
	{
		DWORD windowsError = GetLastError();
		LocalSocketError curError = UnknownSocketError;
		LocalSocketState curState = state();
		RsIPCDebugError << L"RsLocalSocketImpl::setError() " << windowsError  << L":" << error;
		// If the connectToServer fails due to WaitNamedPipe() time-out, assume ConnectionError  
		if (curState == ConnectingState && windowsError == ERROR_SEM_TIMEOUT)
			windowsError = ERROR_NO_DATA;

		switch (windowsError)
		{
			case ERROR_PIPE_NOT_CONNECTED:
			case ERROR_BROKEN_PIPE:
			case ERROR_NO_DATA:
				curError = ConnectionError;
				curState = UnconnectedState;
				break;
			case ERROR_FILE_NOT_FOUND:
				curError = ServerNotFoundError;
				curState = UnconnectedState;
				break;
			case ERROR_ACCESS_DENIED:
				curError = SocketAccessError;
				curState = UnconnectedState;
				break;
			case ERROR_INVALID_USER_BUFFER:
			case ERROR_NOT_ENOUGH_MEMORY:
			{
				RsIPCDebugError << L"RsLocalSocketImpl::setError() on ReadFileEx/WriteFileEx" << windowsError << error;
				curError = UnknownSocketError;
				curState = UnconnectedState;
			}
				break;
			default:
				curError = UnknownSocketError;
				curState = UnconnectedState;
				break;
		}

		if (error != curError)
			curError = error;

		if (curState != state())
		{
			onStateChanged(curState);
			if (state() == UnconnectedState)
				onDisconnected();
		}
		onError(curError);
	}

private: //run int thread
	BOOL doWriteMessage()
	{
		BOOL bDone = FALSE;
		assert(m_dwThreadID == ::GetCurrentThreadId());
		if (m_pPipeInsWrite->bSuccess == FALSE) // setError has been called or disconnectFromServer is called
		{
			RsIPCDebugInfo << L"doWriteMessage m_pPipeInsWrite->bSuccess is FALSE";
			return bDone;
		}

		if (m_pPipeInsWrite->bWritting == TRUE)
		{
			RsIPCDebugInfo << L"doWriteMessage is writting";
			return bDone;
		}
		else
		{
			RsIPCDebugInfo << L"doWriteMessage to write";
		}

		//@{
		if (m_pWrittenQueue->isEmpty())
		{
			RsIPCDebugInfo << L"doWriteMessage queue is empty";
			return bDone;
		}
		std::string data = m_pWrittenQueue->popMessage();
		// @}
		if (data.size() == 0)
		{
			RsIPCDebugInfo << L"doWriteMessage message is empty";
			return bDone;
		}
		if (data.size() > RsLocalSocketBufferLength)
		{
			RsIPCDebugWarning << L"doWriteMessage has a big message" << data.size();
			RsIPCDebugWarning << L"ignore this message";
			return bDone;
		}

		::ZeroMemory(&m_pPipeInsWrite->oOverlap, sizeof(OVERLAPPED));
		::memcpy_s(m_pPipeInsWrite->cbWrite, RsLocalSocketBufferLength, data.data(), data.size());
		m_pPipeInsWrite->dwWrite = (ULONG)data.size();
		m_pPipeInsWrite->dwTotalWritten = 0;
		m_pPipeInsWrite->bWritting = TRUE;
		if (!::WriteFileEx(
			m_pPipeInsWrite->hPipeInst,
			m_pPipeInsWrite->cbWrite,
			m_pPipeInsWrite->dwWrite,
			(LPOVERLAPPED)m_pPipeInsWrite,
			(LPOVERLAPPED_COMPLETION_ROUTINE) &(RsLocalSocketImpl::completedWriteRoutine)))
		{
			RsIPCDebugError << L"doWriteMessage WriteFileEx fail";
			setError();
			m_pPipeInsWrite->bSuccess = FALSE;
		}
		else
		{
			int error = ::GetLastError();
			if (error && error != 183)
			{
				RsIPCDebugError << L"doWriteMessage" <<  error;
			}
		}
		bDone = TRUE;
		return bDone;
	}

	BOOL doReadMessage()
	{
		BOOL bDone = FALSE;
		assert(m_dwThreadID == ::GetCurrentThreadId());
		if (m_pPipeInsRead->bReading)
		{
			RsIPCDebugInfo << L"doReadMessage is reading";
			return bDone;
		}

		m_pPipeInsRead->bReading = TRUE;
		::ZeroMemory(&m_pPipeInsRead->oOverlap, sizeof(OVERLAPPED));
		::ZeroMemory(m_pPipeInsRead->cbRead, RsLocalSocketBufferLength);
		if (!::ReadFileEx( 
			m_pPipeInsRead->hPipeInst,
			m_pPipeInsRead->cbRead, 
			RsLocalSocketBufferLength,
			(LPOVERLAPPED)m_pPipeInsRead,
			(LPOVERLAPPED_COMPLETION_ROUTINE)&(RsLocalSocketImpl::completedReadRoutine)))
		{
			RsIPCDebugError << L"doReadMessage ReadFileEx fail";
			setError();
			m_pPipeInsRead->bSuccess = FALSE;
		}
		else
		{
			int error = ::GetLastError();
			if (error)
			{
				RsIPCDebugError << L"doReadMessage" << error;
			}
		}
		bDone = TRUE;
		return bDone;
	}
	BOOL doRecvMessage(DWORD cbBytesRead)
	{
		assert(m_dwThreadID == ::GetCurrentThreadId());
		onRecvData((char*)(m_pPipeInsRead->cbRead), cbBytesRead);
		return TRUE;
	}
	BOOL doConnecting()
	{
		// Try to open a named pipe
		HANDLE localSocket = INVALID_HANDLE_VALUE;
		DWORD dwMode = PIPE_READMODE_BYTE; 

		for(;;)
		{
			DWORD permissions = (m_openMode & ReadOnly) ? GENERIC_READ : 0;
			permissions |= (m_openMode & WriteOnly) ? GENERIC_WRITE : 0;
			localSocket = ::CreateFileW(m_fullServerName.c_str(),   // pipe name
				permissions,
				0,              // no sharing
				NULL,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe
				FILE_FLAG_OVERLAPPED,
				NULL);          // no template file

			if (localSocket != INVALID_HANDLE_VALUE)
				break;
			DWORD error = ::GetLastError();
			// It is really an error only if it is not ERROR_PIPE_BUSY
			if (ERROR_PIPE_BUSY != error) {
				break;
			}

			// All pipe instances are busy, so wait until connected or up to 5 seconds.
			if (!::WaitNamedPipeW(m_fullServerName.c_str(), 5000))
				break;
		}

		if (localSocket == INVALID_HANDLE_VALUE)
		{
			RsIPCDebugError << L"RsLocalSocketImpl::connectToServer():  INVALID_HANDLE_VALUE";
			onStateChanged(UnconnectedState);
			onError(ServerNotFoundError);
			return FALSE;
		}
		if (!::SetNamedPipeHandleState(localSocket, &dwMode, NULL, NULL))
		{
			RsIPCDebugError << L"RsLocalSocketImpl::connectToServer(): SetNamedPipeHandleState";
			::DisconnectNamedPipe(localSocket);
			::CloseHandle(localSocket);
			onStateChanged(UnconnectedState);
			onError(ServerNotFoundError);
			return FALSE;
		}

		m_hPipe = localSocket;
		m_pPipeInsRead->hPipeInst = m_hPipe;
		m_pPipeInsRead->bSuccess = TRUE;
		m_pPipeInsWrite->hPipeInst = m_hPipe;
		m_pPipeInsWrite->bSuccess = TRUE;

		onStateChanged(ConnectedState);
		onConnected();
		RsIPCDebugWarning << L"RsLocalSocketImpl::connectToServer(): onConnected";
		return TRUE;
	}


	BOOL isReading() const
	{
		assert(m_dwThreadID == ::GetCurrentThreadId());
		return m_pPipeInsRead->bReading;
	}

	BOOL isReadSucceeded() const
	{
		assert(m_dwThreadID == ::GetCurrentThreadId());
		return m_pPipeInsRead->bSuccess;
	}

	BOOL isWritting() const
	{
		assert(m_dwThreadID == ::GetCurrentThreadId());
		return m_pPipeInsWrite->bWritting;
	}

	BOOL isWriteSucceeded() const
	{
			assert(m_dwThreadID == ::GetCurrentThreadId());
			return m_pPipeInsWrite->bSuccess;
	}

private:
	HWND m_hNotifyWnd;
	HANDLE m_hPipe;
	HANDLE m_notifyHandlers[NotifyMax];
	HANDLE m_notifyConnected;
	HANDLE m_hThread;
	DWORD m_dwThreadID;
	DWORD m_openMode;
	std::wstring m_fullServerName;
	mutable volatile LocalSocketState m_state;
	PipeInstanceForRead* m_pPipeInsRead;
	PipeInstanceForWrite* m_pPipeInsWrite;
	RsLocalSocketMessageQueue* m_pWrittenQueue;
	IRsLocalSocketNotify* m_pNotify;
	// m_nWrittingCount for Debug
	UINT m_nWrittingCount;
};

unsigned RsLocalSocketImpl::workingFunc(void* param)
{
	RsLocalSocketImpl* pThis = (RsLocalSocketImpl*)param;

	DWORD dwWait = 0;
	int curState = RsLocalSocketImpl::InitState;
	do 
	{
		curState = RsLocalSocketImpl::InitState;
		dwWait = ::WaitForMultipleObjectsEx(NotifyMax, pThis->m_notifyHandlers, FALSE, INFINITE, TRUE);

		switch (dwWait)
		{
		case WAIT_OBJECT_0 + NotifyQuit:
			curState |= RsLocalSocketImpl::QuitState;
			break;
		case WAIT_OBJECT_0 + NotifyConnecting:
			{
				curState |= RsLocalSocketImpl::OnConnectingState;
			}
			break;
		case WAIT_OBJECT_0 + NotifyWaitWritten:
		{
			curState |= RsLocalSocketImpl::WaitWrittenState;
		}
			break;
		case WAIT_OBJECT_0 + NotifyRead:
		{
			curState |= RsLocalSocketImpl::WaitReadingState;
		}
			break;
		
		case WAIT_IO_COMPLETION:
			curState |= ioCompletHandle(pThis);

			break;
		case WAIT_FAILED:
			RsIPCDebugWarning << L"RsLocalSocketImpl::workingFunc WAIT_FAILED" << GetLastError();
			// go through
		default:
			RsIPCDebugWarning << L"RsLocalSocketImpl::workingFunc default: " << dwWait;
			curState |= RsLocalSocketImpl::QuitState;
			break;
		}

		if (curState & RsLocalSocketImpl::QuitState)
		{
			RsIPCDebugWarning << L"RsLocalSocketImpl::workingFunc quit loop";
			break;//quit loop;
		}

		if (curState & RsLocalSocketImpl::OnConnectingState)
		{
			RsIPCDebugWarning << L"RsLocalSocketImpl::workingFunc doConnecting";
			pThis->doConnecting();
		}

		if (curState & RsLocalSocketImpl::WaitWrittenState)
		{
			RsIPCDebugInfo << L"RsLocalSocketImpl::workingFunc doWriteMessage";
			pThis->doWriteMessage();
		}

		if (curState & RsLocalSocketImpl::WaitReadingState)
		{
			RsIPCDebugInfo << L"RsLocalSocketImpl::workingFunc doReadMessage";
			pThis->doReadMessage();
		}



	} while ( !(curState & RsLocalSocketImpl::QuitState));

	return 0;
}

void RsLocalSocketImpl::completedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap)
{
	PipeInstanceForRead* pPipeIns = (PipeInstanceForRead*) lpOverLap; 
	RsLocalSocketImpl* pThis = (RsLocalSocketImpl*)pPipeIns->lpPointer;

	if (dwErr != 0 && NULL != pThis)
	{
		RsIPCDebugError << L"completedReadRoutine()" << dwErr;
		pThis->setError();
		return;
	}
	DWORD b = 0;
	int r = ::GetOverlappedResult(pPipeIns->hPipeInst, lpOverLap, &b, FALSE);
	if (r == FALSE)
	{
		int error = GetLastError();
		if (error)
		{
			RsIPCDebugError << L"Failed on GetOverlappedResult within completedReadRoutine()" << error;
		}
		else
		{
			RsIPCDebugError << L"Failed on GetOverlappedResult within completedReadRoutine()";
		}
	}

	if (0 != cbBytesRead && NULL != pThis) 
	{ 
		pThis->doRecvMessage(cbBytesRead);
	}
	else
	{
		RsIPCDebugError << L"Failed on GetOverlappedResult within completedReadRoutine() pThis is null";
	}

	pPipeIns->bReading = FALSE;
}

void RsLocalSocketImpl::completedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap)
{
	PipeInstanceForWrite* pPipeIns = (PipeInstanceForWrite*)lpOverLap; 
	RsLocalSocketImpl* pThis = (RsLocalSocketImpl*)pPipeIns->lpPointer;

	if (dwErr != 0)
	{
		RsIPCDebugError << L"completedWriteRoutine()" << dwErr;
		pThis->setError();
		return;
	}
	DWORD b = 0;
	int r = ::GetOverlappedResult(pPipeIns->hPipeInst, lpOverLap, &b, FALSE);
	if (r == FALSE)
	{
		int error = GetLastError();
		if (error)
		{
			RsIPCDebugError << L"Failed on GetOverlappedResult within completedWriteRoutine()" << error;
		}
		else
		{
			RsIPCDebugError << L"Failed on GetOverlappedResult within completedWriteRoutine()";
		}
	}

	pPipeIns->dwTotalWritten += cbWritten;
	// 已经异步写完，于是再异步读
	if (pPipeIns->dwTotalWritten == pPipeIns->dwWrite)
	{
		pPipeIns->bWritting = FALSE;
	}
	else
	{
		RsIPCDebugError << L"completedWriteRoutine partial written";
	}
}

LRESULT RsLocalSocketImpl::notifyWndProc( HWND hwnd, UINT uMsg, WPARAM w, LPARAM l )
{
	switch(uMsg)
	{
	case PIPE_CONNECTED_MSG:
		if (IRsLocalSocketNotify *pNotify = (IRsLocalSocketNotify *)w)
		{
			pNotify->onConnected();
		}
		return 0;
	case PIPE_DISCONNECTED_MSG:
		if (IRsLocalSocketNotify *pNotify = (IRsLocalSocketNotify *)w)
		{
			pNotify->onDisconnected();
		}
		return 0;
	case PIPE_ERROR_MSG:
		if (IRsLocalSocketNotify *pNotify = (IRsLocalSocketNotify *)w)
		{
			pNotify->onError(LocalSocketError(l));
		}
		return 0;
	case PIPE_STATECHANGED_MSG:
		if (IRsLocalSocketNotify *pNotify = (IRsLocalSocketNotify *)w)
		{
			pNotify->onStateChanged(LocalSocketState(l));
		}
		return 0;
	case PIPE_RECV_MSG:
		{
			BSTR data = (BSTR)l;
			if (IRsLocalSocketNotify *pNotify = (IRsLocalSocketNotify *)w)
			{
#ifdef RS_IPC_CLIENT_CHANNEL_DEBUG_DUMP
				QByteArray bytes((char *)data, ::SysStringByteLen(data));
				RsIPCDebugInfo << L"RsLocalSocketImpl::notifyWndProc" << L"-data" << QString(bytes.toHex()).utf16() << L"-len" << ::SysStringByteLen(data);
#endif

				pNotify->onRecvData((char *)data, ::SysStringByteLen(data));
			}
			::SysFreeString(data);
		}
		return 0;
	default:
		break;
	}

	return ::DefWindowProc(hwnd, uMsg, w, l);
}

RsLocalSocket::RsLocalSocket(IRsLocalSocketNotify* pNotify)
	: m_pImpl(new RsLocalSocketImpl)
	, RsLocalSocketBase(pNotify)
{
}

RsLocalSocket::~RsLocalSocket()
{
	delete m_pImpl;
}

void RsLocalSocket::init()
{
	m_pImpl->init(this);
}

void RsLocalSocket::term()
{
	m_pImpl->term();
}

void RsLocalSocket::connectToServer(const wchar_t *name, OpenMode openMode)
{
	m_pImpl->connectToServer(name, openMode);
}

void RsLocalSocket::disconnectFromServer()
{
	m_pImpl->disconnectFromServer();
}

void RsLocalSocket::setSocketDescriptor(void* socketDescriptor)
{
	m_pImpl->setSocketDescriptor(socketDescriptor);
}

LocalSocketState RsLocalSocket::state() const
{
	return m_pImpl->state();
}

bool RsLocalSocket::flush()
{
	return !!m_pImpl->flush();
}

int RsLocalSocket::write(const char *buf, unsigned int len)
{
	return m_pImpl->write(buf, len);
}

bool RsLocalSocket::waitForBytesWritten(int msecs)
{
	return !!m_pImpl->waitForBytesWritten(msecs);
}

void RsLocalSocket::read()
{
	m_pImpl->read();
}
