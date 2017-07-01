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
			  void                      Init(IQy_HandelReceiveData* pReceiveData,EQyIpcType m_QyIpcType,IQy_IPC_DisConnect *pDisConnect=NULL);
			  
			  //创建管道,服务端调用
			  bool                      CreatePipe(const char* StrPipeName,unsigned char ClientMaxCount);
			  //打开服务器端管道
			  bool                      OpenServerPipe(const char* StrPipeName);
			  
			  //启动
			  void                      Start();
			  void                      Stop();
			  bool                      WritePipe(unsigned char *pBuf,unsigned int Len,HANDLE hPipeInst);
			  bool                      WritePipe(std::string &StrData,HANDLE hPipeInst);
			  BOOL                      DisConnect(HANDLE hPipeInst);
	   private: 
		      static unsigned __stdcall QyIpcManage(LPVOID lpParameter);
			  BOOL                      FreeServer(HANDLE &hPipeInst);
			  void                      FreeClient();
			  //服务端读写
			  void                      RwServer();
			  //客户读写
			  void                      RwClient();
			  //分包处理
			  void                      SplicPacket(const HANDLE &hPipeInst,SQy_IPC_MSG_HEADER &header,const char* from,char *pBuf);
			  unsigned int              check_sum(unsigned char * data,unsigned int  DataSize);
			  SQy_IPC_Context           *GetIpcCtx(HANDLE& hPipeInst);
	   protected:
		     
		      void ParseReceiveData(char *buf,int Len,HANDLE hPipeInst);
			  void ReadWritePipe(); 
			  
			  IQy_IPC_DisConnect *m_pDisConnect;
              //管道实例数据
			  std::vector<IQy_Ipc_Base*> m_IPC_Vect;
			  //线程
			  HANDLE m_ThreadHandles[2];
			  HANDLE m_PipeThreadRWExit;
			  int    m_nIsStart;
			 
			  ///接收数据队列
			  std::map<std::string,SReceiveCacheInfo*>   m_IPC_ReceiveDataMap;
			  //发送数据队列
			  std::map<HANDLE,std::queue<SQy_IPC_MSG*>*> m_IPC_SendDataQueueMap;
			  HANDLE         m_ArrayHandle[1024];
			  volatile  int m_ArrayHandleCount;
			  //接收后处理数据的对象
			  IQy_HandelReceiveData*                     m_pQy_HandelReceiveData;
			  //IPC 
			  EQyIpcType                                 m_QyIpcType;
			  //客户端
			  SQy_IPC_Context                            m_ClientQy_IPC_Context;
			  Qy_IPc_InterCriSec                         m_Lock;
			  bool m_bExit;

	};
}
#endif
