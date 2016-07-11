#include "Qy_Ipc_Win.h"
#include <Windows.h>
namespace Qy_IPC
{
	Qy_Ipc_Win::Qy_Ipc_Win(void)
	{
	}

	Qy_Ipc_Win::~Qy_Ipc_Win(void)
	{
	}

	bool Qy_Ipc_Win::CreatePipe(std::string PipeName)
	{
		const DWORD open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;// |FILE_FLAG_FIRST_PIPE_INSTANCE;

		memset(&m_IPC_Context,0,sizeof(m_IPC_Context));
		//PIPE_ACCESS_DUPLEX 管道是双向的
		//PIPE_ACCESS_INBOUND 数据从客户端流到服务器端
		//PIPE_ACCESS_OUTBOUND 数据从服务器端流到客户端
		//FILE_FLAG_OVERLAPPED 允许（但不要求）用这个管道进行异步（重叠式）操作
		m_IPC_Context.hPipeInst = CreateNamedPipeA(PipeName.c_str(),
			open_mode,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
			PIPE_UNLIMITED_INSTANCES,
			PipeBufferSize,
			PipeBufferSize,
			5000,
			NULL);
		

		if (m_IPC_Context.hPipeInst == INVALID_HANDLE_VALUE)
		{
			printf("Unable to create named pipe err %d\n", GetLastError());
			return false;
		}
		memset(&m_IPC_Context.oOverlap, 0, sizeof(m_IPC_Context.oOverlap));
		m_IPC_Context.oOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_IPC_Context.oWriteOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		m_IPC_Context.hDataEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		return true;
	}
	bool Qy_Ipc_Win::ProcessConnection()
	{
		
	  
		// Do we have a client connected to our pipe?
		if (INVALID_HANDLE_VALUE == m_IPC_Context.hPipeInst)
			return false;

		BOOL ok = ConnectNamedPipe(m_IPC_Context.hPipeInst,&m_IPC_Context.oOverlap);

		DWORD err = GetLastError();
		if (ok)
		{
			return false;
		}


		switch (err) 
		{
				case ERROR_IO_PENDING:
					  m_IPC_Context.dwState=CONNECTING_STATE;
					  break;
			   // Client is already connected, so signal an event.   
			   case ERROR_PIPE_CONNECTED:
				   
					  break;  
			   case ERROR_NO_DATA:
				   return false;
	   default:
		  return false;
		}

		return true;
	}

	bool Qy_Ipc_Win::GetTimeOut(int CurTime)
	{
		if(m_IPC_Context.dwState==1&& CurTime-m_IPC_Context.UpdataTime>10000)
		{
			return true;
		}
		return false;
	}
	bool Qy_Ipc_Win::Disconnect()
	{
		if (!DisconnectNamedPipe(m_IPC_Context.hPipeInst))  
		{  
			printf("DisconnectNamedPipe failed with %d.\n", GetLastError());  
			return false;
		}  
        return true;
	}
}