#ifndef __RS_IPCCONSTANT_H__
#define __RS_IPCCONSTANT_H__

// Server Message
// -----------------------------------------------------------------
// |"ripc"| L | V | CID | SID | EXTRA* | ... | PAYLOAD ... PAYLOAD |
// -----------------------------------------------------------------
// |-----------                Message                  -----------|
// -----------------------------------------------------------------


// Client Message
// ---------------------------------------------------------------
// |"ripc"| L | V | CID | 0 | EXTRA* | ... | PAYLOAD ... PAYLOAD |
// ---------------------------------------------------------------
// |-----------                Message                -----------|
// ---------------------------------------------------------------


// V: uint32, version. server has its acceptable versions, and treat others a fatal.
// L: uint32, length of Message, less that 2**24, i.e. 16MB
// CID: uint32, id of request sent by client
// SID: uint32, id of reply sent by server, client must specify it 0.
// EXTRA: different versions may have extra bytes, for future use.



const char* const RIPC_BYE_MESSAGE = "DISCONNET_REQUEST110";
const char* const RIPC_HEARTBEAT_HEADER = "HEARTBEAT";
const char* const RIPC_FRAME_FLAGS = "ripc";
const uint64_t RIPC_HEARTBEAT_INTERVAL = 10000; // 5秒一个心跳包
const uint32_t RIPC_ID_ZERO = 0; // 0 作为传输内部使用的id
const uint32_t RIPC_CLIENT_MODE_VERSION = 0xF0000000;

#pragma pack(push, 1)
struct RIPCMessageHeader {
	char m_magicFlags[4]; // must be RIPC_FRAME_FLAGS
	uint32_t m_length;
	uint32_t m_version;
	uint32_t m_cid;
	uint32_t m_sid;
};
#pragma pack(pop)
#endif // __RS_IPCCONSTANT_H__
