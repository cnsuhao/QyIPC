#include "Qy_IPC_PMan.h"
#include <process.h>
#include <Windows.h>
#include <map>
#include "Qy_Ipc_Manage.h"
#include <tlhelp32.h>
typedef struct SProcessInfo
{
	PROCESS_INFORMATION pi;
	STARTUPINFOW si;
};

bool KillProcessFromName(std::wstring strProcessName)  
{  
	 transform(strProcessName.begin(), strProcessName.end(), strProcessName.begin(),  toupper);   
	//创建进程快照(TH32CS_SNAPPROCESS表示创建所有进程的快照)  
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);  

	//PROCESSENTRY32进程快照的结构体  
	PROCESSENTRY32 pe;  

	//实例化后使用Process32First获取第一个快照的进程前必做的初始化操作  
	pe.dwSize = sizeof(PROCESSENTRY32);  


	//下面的IF效果同:  
	//if(hProcessSnap == INVALID_HANDLE_VALUE)   无效的句柄  
	if(!Process32First(hSnapShot,&pe))  
	{  
		return false;  
	}  

	//将字符串转换为小写  
	//strProcessName.MakeLower();  

	//如果句柄有效  则一直获取下一个句柄循环下去  
	while (Process32Next(hSnapShot,&pe))  
	{  

		//pe.szExeFile获取当前进程的可执行文件名称  
		std::wstring scTmp = pe.szExeFile;

		//将可执行文件名称所有英文字母修改为小写  
		 transform(scTmp.begin(), scTmp.end(), scTmp.begin(),  toupper);   

		//比较当前进程的可执行文件名称和传递进来的文件名称是否相同  
		//相同的话Compare返回0  
		if(scTmp==strProcessName)  
		{  

			//从快照进程中获取该进程的PID(即任务管理器中的PID)  
			DWORD dwProcessID = pe.th32ProcessID;  
			HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE,FALSE,dwProcessID);  
			::TerminateProcess(hProcess,0);  
			CloseHandle(hProcess);  
			return true;  
		}  
		
	}  
	return false;  
}  

namespace Qy_IPC
{
	std::map<std::string,SProcessInfo*>  G_ProcessInfoMap;
	CQy_IPC_PMan::CQy_IPC_PMan()
	{

	}
	CQy_IPC_PMan::~CQy_IPC_PMan()
	{

	}
    bool CQy_IPC_PMan::KillProcessFromName(std::wstring strProcessName)
	{
		return ::KillProcessFromName(strProcessName);
	}
	CQy_IPC_PMan* CQy_IPC_PMan::GetInstance()
	{
		if(pInstance==NULL)
		{
			pInstance = new CQy_IPC_PMan();
		}
		return pInstance;
	}
	void CQy_IPC_PMan::FreeInstance()
	{
		delete pInstance;
		pInstance=NULL;
	}
	bool CQy_IPC_PMan::StartApp(std::wstring ExeFile, std::wstring CmdLine)
	{
		SProcessInfo Info,*pInfo=&Info;
        memset(pInfo,0,sizeof(SProcessInfo));
		
		BOOL ret = CreateProcessW(ExeFile.c_str(),(LPWSTR)CmdLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &pInfo->si, &pInfo->pi);
		if (!ret) 
		{
			
			return false;
		}
	    return true;
	}
	char exchange(char c)
	{

		if(c <= 'Z' && c >= 'A')

			c = tolower(c);

		else if(c >= 'a' && c <= 'z')

			c = toupper(c);

		return c;

	}
	
	CQy_IPC_PMan* CQy_IPC_PMan::pInstance=NULL;
}