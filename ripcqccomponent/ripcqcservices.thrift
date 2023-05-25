// The first thing to know about are types. The available types in Thrift are:
//
//  bool        Boolean, one byte
//  byte        Signed byte
//  i16         Signed 16-bit integer
//  i32         Signed 32-bit integer
//  i64         Signed 64-bit integer
//  double      64-bit floating point value
//  string      String
//  map<t1,t2>  Map from one type to another
//  list<t1>    Ordered list of one type
//  set<t1>     Set of unique elements of one type
//
// Did you also notice that Thrift supports C style comments?


// Just in case you were wondering... yes. We support simple C comments too.

// Thrift files can reference other Thrift files to include common struct
// and service definitions. These are found using the current path, or by
// searching relative to any paths specified with the -I compiler flag.
//
// Included objects are accessed using the name of the .thrift file as a
// prefix. i.e. shared.SharedObject

// include "shared.thrift"

// Thrift lets you do typedefs to get pretty names for your types. Standard
// C style here.
 
// typedef i32 MyInteger

// Thrift files can namespace, package, or prefix their output in various
// target languages.

namespace cpp rs.qc

struct ResAuthorStatus {
	1: i32 authorStatus;
	2: string strAutorInfo;
}

enum ELoginErrCode {
	ERR_LOGIN_SUCCESS = 0x100,
	ERR_PWD_EMPTY,
	ERR_PWD_UNMATCH,
	ERR_USER_UNDEFINED
}


service ripcqcservices {
	void notifyAuthorizeStatusChanged(1: string authId);
	i32 userLogin(1: string userName, 2: string userPwd);
	ResAuthorStatus queryAuthorStatus(1: string reqId);
}