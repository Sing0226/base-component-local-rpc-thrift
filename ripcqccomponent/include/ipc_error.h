#ifndef __RS_IPC_ERROR_H__
#define __RS_IPC_ERROR_H__

enum LocalSocketError
{
	ConnectionRefusedError,
	PeerClosedError,
	ServerNotFoundError,
	SocketAccessError,
	SocketResourceError,
	SocketTimeoutError,
	DatagramTooLargeError,
	ConnectionError,
	UnsupportedSocketOperationError,
	UnknownSocketError,
};

enum LocalSocketState
{
	UnconnectedState,
	ConnectingState = 2,// copy from Qt
	ConnectedState = 3,
	ClosingState = 6,
};

enum OpenMode {
	NotOpen = 0x0000,
	ReadOnly = 0x0001,
	WriteOnly = 0x0002,
	ReadWrite = ReadOnly | WriteOnly,
	Append = 0x0004,
	Truncate = 0x0008,
	Text = 0x0010,
	Unbuffered = 0x0020
};

enum LoginErrCode	 //登录错误码
{
	ERR_LOGIN_BASE = 0x100,
	ERR_LOGIN_SUCCESS = ERR_LOGIN_BASE,
	ERR_PASSWORD_EMPTY ,
	ERR_PASSWORD_UNMATCH ,
	ERR_USER_UNDEFINED
};

#endif /* __RS_IPC_ERROR_H__ */
