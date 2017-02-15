#ifndef Qy_Ipc_Win_H_
#define Qy_Ipc_Win_H_
//windows ¹ÜµÀ
#include <string>
#include "Qy_IPC_Context.h"
#include "IQy_Ipc_Base.h"
namespace Qy_IPC
{
	class Qy_Ipc_Win:public IQy_Ipc_Base
	{
		public:
			Qy_Ipc_Win(void);
			~Qy_Ipc_Win(void);
			bool CreatePipe(std::string PipeName);
			bool ProcessConnection();
			void WriteData(unsigned char* buf,int len)
			{

			}
			bool GetTimeOut(int CurTime);
			bool Disconnect();
			SQy_IPC_Context *Get_IPC_Context()
			{
				return &m_IPC_Context;
			}
	private:
			SQy_IPC_Context m_IPC_Context;

	};
}
#endif
