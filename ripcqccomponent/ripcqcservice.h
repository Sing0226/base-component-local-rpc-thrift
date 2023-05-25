#ifndef __RIPC_QC_SERVICE_H_
#define __RIPC_QC_SERVICE_H_
#include <map>
#include <list>
#include <string>

namespace ripcqc{
	enum RIpcQcErrorCode
	{
		RIpcQcErrorNoError = 0,
		RIpcQcErrorInternal = 500,
		RIpcQcErrorNotAuthorized,
		RIpcQcErrorUndefined
	};
	typedef struct AuthorInfo {
		bool authorStatus;
		std::string authorInfo;

		AuthorInfo()
			: authorStatus(false)
		{
		}
	}AuthorInfo, *PAuthorInfo;
}
#endif