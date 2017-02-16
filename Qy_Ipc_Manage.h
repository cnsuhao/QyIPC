#ifndef Qy_Ipc_Manage_H_
#define Qy_Ipc_Manage_H_

#include <vector>
#include <queue>
#include <map>
#include <string>
#include "IQy_Ipc_Base.h"
#include "Qy_IPC_Context.h"
namespace Qy_IPC
{
	
	enum EQyIpcType
	{
		QyIpcClient=0,
		QyIpcServer=1,
	};
	
	
	/***零界值***/
	class Qy_IPc_InterCriSec
	{
		public:
			Qy_IPc_InterCriSec(DWORD dwSpinCount = 4096);
			~Qy_IPc_InterCriSec();
			void Lock();
			void Unlock();
			BOOL TryLock();
			DWORD SetSpinCount(DWORD dwSpinCount);
			CRITICAL_SECTION* GetObject();
		private:
			Qy_IPc_InterCriSec(const Qy_IPc_InterCriSec& cs);
			Qy_IPc_InterCriSec operator = (const Qy_IPc_InterCriSec& cs);

		private:
			//临界值
			CRITICAL_SECTION m_crisec;
	};

	struct SReceiveCacheInfo;
	struct SReceiveData;
	class Qy_Ipc_Manage
	{
	   public:
		     Qy_Ipc_Manage();
			  ~Qy_Ipc_Manage(void);
			 
			  //初始化
			  int Init(IQy_HandelReceiveData* pReceiveData,EQyIpcType m_QyIpcType,IQy_IPC_DisConnect *pDisConnect=NULL);
			  
			  //创建管道,服务端调用
			  bool CreatePipe(std::string PipeName,unsigned char ClientMaxCount);
			  //打开服务器端管道
			  bool OpenServerPipe(std::string PipeName);
			  
			  //启动
			  void Start();
			  void Stop();
			  bool WritePipe(unsigned char *pBuf,unsigned int Len,HANDLE hPipeInst);
			  unsigned int check_sum(unsigned char * data,unsigned int  DataSize);
			  BOOL DisConnect(HANDLE hPipeInst);
	   private: 
		      static unsigned __stdcall QyIpcManage(LPVOID lpParameter);
	   protected:
		      IQy_IPC_DisConnect *m_pDisConnect;
		      void ParseReceiveData(char *buf,int Len,HANDLE hPipeInst);
			  void ReadWritePipe();
              //管道实例数据
			  std::vector<IQy_Ipc_Base*> m_IPC_Vect;
			  //线程
			  HANDLE m_ThreadHandles[2];
			  int m_nIsStart;
			 
			  std::map<std::string,SReceiveCacheInfo*> m_IPC_ReceiveDataMap;
			  //发送数据队列
			  std::map<HANDLE,std::queue<SQy_IPC_MSG*>*> m_IPC_SendDataQueueMap;
			  HANDLE m_ArrayHandle[1024];
			  int m_ArrayHandleCount;
			  //接收后处理数据的对象
			  IQy_HandelReceiveData* m_pQy_HandelReceiveData;
			  //IPC 
			  EQyIpcType m_QyIpcType;
			  //客户端
			  SQy_IPC_Context m_ClientQy_IPC_Context;
			  Qy_IPc_InterCriSec m_Lock;
			  bool m_bExit;

	};
}
#endif
