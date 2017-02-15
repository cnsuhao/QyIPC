
#ifndef Qy_IPC_PMan_H_
#define Qy_IPC_PMan_H_
#include <string>
#include <map>

namespace Qy_IPC
{
  //一个启动app的简单类
  class CQy_IPC_PMan
  {
     public:
	        
		     static CQy_IPC_PMan* GetInstance();
		     static void FreeInstance();
			 bool StartApp(std::string &ExeFile, std::string &CmdLine);
			 int CloseApp(std::string ExeFile);
			 bool KillProcessFromName(std::wstring strProcessName);
     private:
			CQy_IPC_PMan(); 
			~CQy_IPC_PMan();
			static CQy_IPC_PMan* pInstance;
   };
}
#endif