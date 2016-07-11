#include "Qy_Ipc_HandelReceiveData.h"
#include "Qy_IPC_Context.h"
namespace Qy_IPC
{
	Qy_Ipc_HandelReceiveData::Qy_Ipc_HandelReceiveData()
	{

	}
	void Qy_Ipc_HandelReceiveData::HandelReceiveData(char *buf,int Len,std::string strId)
	{
		printf("%s\n",buf);
	}
}