#include <string>
#include <map>
#include <Windows.h>
#ifndef Qy_IPC_PMan_H_
#define Qy_IPC_PMan_H_
namespace Qy_IPC
{
   //子进程管理器
  class CQy_IPC_PMan
   {
     public:
	         ~CQy_IPC_PMan();
		     static CQy_IPC_PMan* GetInstance();
		     static void FreeInstance();
			 bool StartProcess(std::string &ExeFile, std::string &CmdLine);
             
			 void WriteData(std::string &ExeFile,char* pBuf,int Len);
			 void SetExeInfo(std::string &ExeFile,HANDLE Pipe);
			 BOOL CloseApp(std::string ExeFile);
			 bool KillProcessFromName(std::wstring strProcessName);
     private:
			CQy_IPC_PMan();
			static CQy_IPC_PMan* pInstance;
			std::map<std::string,HANDLE> m_ExePipeMap;
   };
}
#endif