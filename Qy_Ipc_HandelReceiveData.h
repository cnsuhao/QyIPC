#include "IQy_Ipc_Base.h"
#ifndef Qy_Ipc_HandelReceiveData_H_
#define Qy_Ipc_HandelReceiveData_H_
namespace Qy_IPC
{
	class Qy_Ipc_HandelReceiveData:public IQy_HandelReceiveData
	{ 
		   public:
				  Qy_Ipc_HandelReceiveData();
				   virtual void HandelReceiveData(char *buf,int Len,std::string strId);
	};
}
#endif