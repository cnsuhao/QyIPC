#include <Windows.h>
#include "Qy_Ipc_Manage.h"
#include "Qy_IPC_Context.h"
#include "Qy_Ipc_Win.h"
#include <process.h>
#include <assert.h>
namespace Qy_IPC
{

	struct SReceiveData
	{
		int DataLen;
		char *Buf;
		int PktId;
	};
	struct SReceiveCacheInfo
	{
		GUID Guid;
		int TotalLen;
		int CurLen;
		//管道句柄
		HANDLE hPipeInst;
		std::vector<SReceiveData *>* pDataList;
	};

	
	Qy_IPc_InterCriSec::Qy_IPc_InterCriSec(DWORD dwSpinCount)
	{
		::InitializeCriticalSectionAndSpinCount(&m_crisec, dwSpinCount);
	}
	Qy_IPc_InterCriSec::~Qy_IPc_InterCriSec()
	{
		::DeleteCriticalSection(&m_crisec);
	}
    void Qy_IPc_InterCriSec::Lock()
	{
		::EnterCriticalSection(&m_crisec);
	}
	void Qy_IPc_InterCriSec::Unlock()
	{
		::LeaveCriticalSection(&m_crisec);
	}
	BOOL Qy_IPc_InterCriSec::TryLock()
	{
		return ::TryEnterCriticalSection(&m_crisec);
	}
	DWORD Qy_IPc_InterCriSec::SetSpinCount(DWORD dwSpinCount) 
	{
		return ::SetCriticalSectionSpinCount(&m_crisec, dwSpinCount);
	}
    CRITICAL_SECTION* Qy_IPc_InterCriSec::GetObject()
	{
		return &m_crisec;
	}

	Qy_Ipc_Manage::Qy_Ipc_Manage():m_pDisConnect(NULL)
		,m_nIsStart(0)
		,m_bExit(true)
	{
		
	}

	Qy_Ipc_Manage::~Qy_Ipc_Manage(void)
	{
		Stop();
	}
	void Qy_Ipc_Manage::Stop()
	{
		
		if(m_nIsStart>0)
		{
			m_bExit=true;
			if(m_QyIpcType==QyIpcServer)
			{
			     SQy_IPC_Context *P=((Qy_Ipc_Win*)m_IPC_Vect[0])->Get_IPC_Context();
				 SetEvent(P->hDataEvent);
			}else{
			     SetEvent(m_ClientQy_IPC_Context.hDataEvent);
			}
			DWORD dwWait = WaitForMultipleObjects(  1,
				m_ThreadHandles,      // array of event objects   
				TRUE,        // does not wait for all   
				INFINITE); 
			printf("Exit");
			m_nIsStart=0;
		}
	   
		if(m_QyIpcType==QyIpcServer)
		{
			for(size_t i=0;i<m_IPC_Vect.size();i++)
			{
				SQy_IPC_Context *P=((Qy_Ipc_Win*)m_IPC_Vect[i])->Get_IPC_Context();
				DisconnectNamedPipe(P->hPipeInst);
				::CloseHandle(P->hPipeInst);
				::CloseHandle(P->oOverlap.hEvent);
				::CloseHandle(P->hDataEvent);
				::CloseHandle(P->oWriteOverlap.hEvent);
				IQy_Ipc_Base* p2=m_IPC_Vect.at(i);
				delete p2;
				m_IPC_Vect.clear();
			}
		}else
		{
			if(m_ClientQy_IPC_Context.hPipeInst!=INVALID_HANDLE_VALUE){
				::CloseHandle(m_ClientQy_IPC_Context.hPipeInst);
				 m_ClientQy_IPC_Context.hPipeInst=INVALID_HANDLE_VALUE;
				::CloseHandle(m_ClientQy_IPC_Context.oOverlap.hEvent);
				 m_ClientQy_IPC_Context.oOverlap.hEvent=INVALID_HANDLE_VALUE;
				::CloseHandle(m_ClientQy_IPC_Context.oWriteOverlap.hEvent);
				 m_ClientQy_IPC_Context.oWriteOverlap.hEvent=INVALID_HANDLE_VALUE;
				::CloseHandle(m_ClientQy_IPC_Context.hDataEvent);
				 m_ClientQy_IPC_Context.hDataEvent=INVALID_HANDLE_VALUE;
			}
		}
		
		
	}
	void Qy_Ipc_Manage::Init(IQy_HandelReceiveData* pReceiveData,EQyIpcType m_QyIpcType,IQy_IPC_DisConnect *pDisConnect)
	{
		m_pDisConnect=pDisConnect;
		this->m_QyIpcType=m_QyIpcType;
		m_pQy_HandelReceiveData=pReceiveData;
		m_ArrayHandleCount=0;
		memset(m_ArrayHandle,0,sizeof(m_ArrayHandle));
		
		memset(&m_ClientQy_IPC_Context,0,sizeof(m_ClientQy_IPC_Context));
		m_nIsStart=0;
		m_bExit=false;
		return;
	}
	
	bool Qy_Ipc_Manage::CreatePipe(std::string PipeName,unsigned char ClientMaxCount)
	{
		if(m_QyIpcType==QyIpcServer)
		{
			size_t PipeInstanceCount=ClientMaxCount;
			for(size_t i=0;i<PipeInstanceCount;i++)
			{
				Qy_Ipc_Win *Ipc = new Qy_Ipc_Win();
				if(!Ipc->CreatePipe(PipeName))
				{
					return false;
				}
				Ipc->ProcessConnection();
                Ipc->Get_IPC_Context()->dwState=CONNECTING_STATE;

				m_IPC_Vect.push_back(Ipc);
				m_ArrayHandle[m_ArrayHandleCount++]=Ipc->Get_IPC_Context()->oOverlap.hEvent;
				m_ArrayHandle[m_ArrayHandleCount++]=Ipc->Get_IPC_Context()->hDataEvent;
				m_ArrayHandle[m_ArrayHandleCount++]=Ipc->Get_IPC_Context()->oWriteOverlap.hEvent;
			}
		 
		}else
		{
			::MessageBox(NULL,L"客户端不能创建Pipe",L"提示",0);
			return false;
		}
		return true;
	}
	bool Qy_Ipc_Manage::OpenServerPipe(std::string PipeName)
	{
		
			m_ClientQy_IPC_Context.hPipeInst = CreateFileA( 
				PipeName.c_str(),			// Pipe name 
				GENERIC_READ |			// Read and write access 
				GENERIC_WRITE,
				0,						// No sharing 
				NULL,					// Default security attributes
				OPEN_EXISTING,			// Opens existing pipe|FILE_FLAG_OVERLAPPED 
				SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION |
				FILE_FLAG_OVERLAPPED,						// Default attributes 
				NULL);					// No template file 

			// Break if the pipe handle is valid. 
			if (m_ClientQy_IPC_Context.hPipeInst== INVALID_HANDLE_VALUE) 
			{
				printf("Unable to open named INVALID_HANDLE_VALUE");
				 return false;
			}
	 
			m_ClientQy_IPC_Context.oOverlap.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_ClientQy_IPC_Context.hDataEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			m_ClientQy_IPC_Context.oWriteOverlap.hEvent= CreateEvent(NULL, TRUE, FALSE, NULL);

			m_ArrayHandle[m_ArrayHandleCount++]=m_ClientQy_IPC_Context.oOverlap.hEvent;
			m_ArrayHandle[m_ArrayHandleCount++]=m_ClientQy_IPC_Context.hDataEvent;
			m_ArrayHandle[m_ArrayHandleCount++]=m_ClientQy_IPC_Context.oWriteOverlap.hEvent;
			DWORD cbRet;
			BOOL fSuccess=::ReadFile(m_ClientQy_IPC_Context.hPipeInst,m_ClientQy_IPC_Context.ReceiveBuf,PipeBufferSize,&cbRet,  &m_ClientQy_IPC_Context.oOverlap);
			m_ClientQy_IPC_Context.dwState = READING_STATE;
			
			return TRUE;;
	}


	unsigned int Qy_Ipc_Manage::check_sum(unsigned char * data,unsigned int  DataSize)
	{
		if ((data == NULL) || (DataSize==0))
		{
			return 0;
		}

		register unsigned int   sum  = 0;
		register unsigned int   iter = DataSize;
		register unsigned char  *bptr = data;

		while (iter-- > 0){
			sum += *bptr;
			bptr++;
		}
		return sum;
	}
	bool  Qy_Ipc_Manage::WritePipe(unsigned char *pBuf,unsigned int BufLen,HANDLE hPipeInst)
	{
		
		if(m_nIsStart<=0)
		{
			printf("请运行Start");
			return false;
		}
          
		SQy_IPC_Context *pIpc=NULL;
		if(m_QyIpcType==QyIpcServer)
		{
			for(size_t i=0;i<m_IPC_Vect.size();i++)
			{
				pIpc=((Qy_Ipc_Win *)m_IPC_Vect[i])->Get_IPC_Context();
				if(pIpc->hPipeInst==hPipeInst)
				{
					break;
				}
			}
			if(pIpc==NULL)
			{
				return false;
			}
		}
		else
		{
			if(m_ClientQy_IPC_Context.hPipeInst==INVALID_HANDLE_VALUE)
			{
                pIpc=NULL;
			}else
			{
                  pIpc=&m_ClientQy_IPC_Context;
			}
			
		}
        if(pIpc==NULL)
			return false;

		if(pIpc->dwState==WRITOK_STATE||pIpc->dwState==READING_STATE ||pIpc->dwState==WRITING_STATE)
		{
			
		}else
		{
			return false;
		}
 


		if(m_QyIpcType!=QyIpcServer)
		{
			hPipeInst=m_ClientQy_IPC_Context.hPipeInst;
			
		}else if(hPipeInst==0)
		{
				hPipeInst=((Qy_Ipc_Win*)m_IPC_Vect[0])->Get_IPC_Context()->hPipeInst;
		}
		static size_t HeaderLen=sizeof(SQy_IPC_MSG_HEADER);
		GUID PktGuid;
         CoCreateGuid(&PktGuid);
		
		int PktId=0;

		
		SQy_IPC_MSG_HEADER MsgHeader;//=(SQy_IPC_MSG_HEADER*)::malloc(sizeof(SQy_IPC_MSG_HEADER));
		MsgHeader.MsgType=1;
		
		MsgHeader.TotalDataLen=BufLen;
        MsgHeader.PktGuid=PktGuid;
		m_Lock.Lock();

		std::map<HANDLE,std::queue<SQy_IPC_MSG*>*>::iterator It=m_IPC_SendDataQueueMap.find(hPipeInst);
		if(It==m_IPC_SendDataQueueMap.end())
		{
               std::queue<SQy_IPC_MSG*>* pQ = new std::queue<SQy_IPC_MSG*>();
			   m_IPC_SendDataQueueMap.insert(std::pair<HANDLE,std::queue<SQy_IPC_MSG*>*>(hPipeInst,pQ));
			   It=m_IPC_SendDataQueueMap.find(hPipeInst);
		}
		unsigned int PktLen=PipeBufferSize-HeaderLen;
		while(BufLen>0)
		{
			unsigned char *pData =NULL;
			SQy_IPC_MSG *msg=(SQy_IPC_MSG*)::malloc(sizeof(SQy_IPC_MSG));
			msg->hPipeInst=hPipeInst;
			MsgHeader.PktId=PktId;
			MsgHeader.DataLen=BufLen > PktLen ? PktLen:BufLen;
            
			pData =(unsigned char*)::malloc(PipeBufferSize);
			memset(pData,0,PipeBufferSize);
			//拷贝数据
            memcpy(pData+HeaderLen,pBuf,MsgHeader.DataLen);
			
			//拷贝头
			MsgHeader.DataSum=check_sum(pData,PktLen);
			memcpy(pData,&MsgHeader,HeaderLen);
           

			msg->pBuf=pData;
			msg->Len=PipeBufferSize;

			It->second->push(msg);
			BufLen-=MsgHeader.DataLen;
		}
		
		if(pIpc->dwState==WRITOK_STATE||pIpc->dwState==READING_STATE)
		{
			BOOL xx=SetEvent(pIpc->hDataEvent);
			printf("写 %d \n",xx);
		}
		m_Lock.Unlock();
		return true;
	}
	
	bool Qy_Ipc_Manage::WritePipe(std::string StrData,HANDLE hPipeInst)
	{
	    bool Ok=false;
		int len=StrData.length()+100;
		unsigned char *pBuf=(unsigned char *)::malloc(len);
		memset(pBuf,0,len);
		memcpy(pBuf,StrData.c_str(),StrData.length());
		Ok=WritePipe(pBuf,StrData.length()+2,hPipeInst);
		free(pBuf);
		return Ok;
	}
	BOOL Qy_Ipc_Manage::DisConnect(HANDLE hPipeInst)
	{
		     
		
		    m_Lock.Lock();
			
			if(m_QyIpcType==QyIpcServer)
			{
				std::map<HANDLE,std::queue<SQy_IPC_MSG*>*>::iterator It=m_IPC_SendDataQueueMap.find(hPipeInst);
				if(It!=m_IPC_SendDataQueueMap.end())
				{
					while(It->second->size()>0)
					{
						SQy_IPC_MSG *msg=  It->second->front();
						free(msg->pBuf);
						free(msg);
						It->second->pop();
					}
					delete It->second;
					m_IPC_SendDataQueueMap.erase(It);
				}
					for(size_t i=0;i<m_IPC_Vect.size();i++)
					{
							SQy_IPC_Context *pIpc=((Qy_Ipc_Win *)m_IPC_Vect[i])->Get_IPC_Context();
							if(pIpc->hPipeInst==hPipeInst)
							{
								pIpc->dwState=CONNECTING_STATE;
								ResetEvent(pIpc->hDataEvent);
								ResetEvent(pIpc->oOverlap.hEvent);
								ResetEvent(pIpc->oWriteOverlap.hEvent);
								
								if (!DisconnectNamedPipe(hPipeInst))  
								{  
									m_Lock.Unlock();
									return 0-GetLastError();
								} 
								if(m_pDisConnect!=NULL)
									m_pDisConnect->DisConnct(hPipeInst);
								((Qy_Ipc_Win *)m_IPC_Vect[i])->ProcessConnection();
								break;
							}
					}
             }
		else
		{
			HANDLE h=m_ClientQy_IPC_Context.hPipeInst;
			CloseHandle(m_ClientQy_IPC_Context.hPipeInst);
			m_ClientQy_IPC_Context.hPipeInst=INVALID_HANDLE_VALUE;
			CloseHandle(m_ClientQy_IPC_Context.hDataEvent);
			m_ClientQy_IPC_Context.hDataEvent=INVALID_HANDLE_VALUE;
			CloseHandle(m_ClientQy_IPC_Context.oOverlap.hEvent);
			m_ClientQy_IPC_Context.oOverlap.hEvent=INVALID_HANDLE_VALUE;
            CloseHandle(m_ClientQy_IPC_Context.oWriteOverlap.hEvent);
			m_ClientQy_IPC_Context.oWriteOverlap.hEvent=INVALID_HANDLE_VALUE;

			if(m_pDisConnect!=NULL)
			   m_pDisConnect->DisConnct(h);
		}

        m_Lock.Unlock();
		return TRUE;
	}
	unsigned WINAPI Qy_Ipc_Manage::QyIpcManage(LPVOID lpParameter)
	{
		Qy_Ipc_Manage *p =(Qy_Ipc_Manage*)lpParameter;
		p->ReadWritePipe();
		return 1;
	}

	void Qy_Ipc_Manage::Start()
	{
		 m_nIsStart=1;
		 UINT addrr=0;
		 m_ThreadHandles[0]=(HANDLE)_beginthreadex(NULL, NULL, QyIpcManage, (LPVOID)this, 0,&addrr);
		
	}
    
	void Qy_Ipc_Manage:: ReadWritePipe()
	{
		DWORD  cbRet;  
		char *pBuf=( char*)::malloc(PipeBufferSize);
		memset(pBuf,0,PipeBufferSize);
		BOOL	fSuccess=FALSE;
		int Index=0;
		Sleep(20);
		while(!m_bExit)
		{
			if(m_QyIpcType==QyIpcServer)
			{
					DWORD dwWait = WaitForMultipleObjects(  
						m_ArrayHandleCount,    // number of event objects   
						m_ArrayHandle,      // array of event objects   
						FALSE,        // does not wait for all   
						INFINITE); 
					int i = dwWait - WAIT_OBJECT_0;  // determines which pipe   
					if ( i<0||i >(m_ArrayHandleCount - 1))  
					{  
						printf("Index out of range. %d\n",m_ArrayHandleCount);  
						break;
					} 
					if(m_bExit)
					{
						break;
					}

					Index=i/3;
					if(i%3==0)
					{
						
						SQy_IPC_Context *pIpc=((Qy_Ipc_Win *)m_IPC_Vect[Index])->Get_IPC_Context();
						fSuccess = GetOverlappedResult(  //判断一个重叠操作当前的状态        //非零表示成功，零表示失败
							pIpc->hPipeInst, // handle to pipe   
							&pIpc->oOverlap, // OVERLAPPED structure   
							&cbRet,            // bytes transferred   
							FALSE);            // do not wait  
						if(fSuccess)
						{
							switch (pIpc->dwState) 
							{ 
								case CONNECTING_STATE: 
													   printf("客服端链接\n"); 
													   pIpc->dwState = READING_STATE;
													   SetEvent(pIpc->hDataEvent);	
													   break;
								case READING_STATE:
														printf("读取数据\n");
														break;
							}
							if(fSuccess&&cbRet>0)
							{
								ParseReceiveData(pIpc->ReceiveBuf,cbRet,pIpc->hPipeInst);
								pIpc->UpdataTime=::GetTickCount();
							}
							fSuccess = ReadFile( 
							pIpc->hPipeInst,
							pIpc->ReceiveBuf,
							PipeBufferSize,
							NULL,  
							&pIpc->oOverlap); 
							
						}else //客户端已断开
						{
							ResetEvent(pIpc->oOverlap.hEvent);
							DisConnect(pIpc->hPipeInst);
						}
					}else if(i%3==1)
					{
						ResetEvent(m_ArrayHandle[i]);
						SQy_IPC_Context *pIpc=((Qy_Ipc_Win *)m_IPC_Vect[Index])->Get_IPC_Context();
						if(pIpc->dwState==READING_STATE||pIpc->dwState==WRITOK_STATE)
						{
							m_Lock.Lock();
							std::map<HANDLE,std::queue<SQy_IPC_MSG*>*>::iterator It2=m_IPC_SendDataQueueMap.find(pIpc->hPipeInst);
							if(It2!=m_IPC_SendDataQueueMap.end()&&It2->second->size()>0)
							{
								SQy_IPC_MSG* It=It2->second->front();
								pIpc->dwState=WRITING_STATE;
									memset(pIpc->SendBuf,0,PipeBufferSize);
									unsigned char *pBuf=It->pBuf;
									memcpy(pIpc->SendBuf,pBuf,It->Len);
									pIpc->cbToWrite=It->Len;
									WriteFile(  
										pIpc->hPipeInst,  
										pIpc->SendBuf,  
										pIpc->cbToWrite,  
										NULL,  
										&pIpc->oWriteOverlap);
									printf("写数据\n");
								free(It->pBuf);
								It->pBuf=NULL;
								free(It);
								It=NULL;
								It2->second->pop();
							}
							m_Lock.Unlock();
						}
						
					}else if(i%3==2)
					{
							SQy_IPC_Context *pIpc=((Qy_Ipc_Win *)m_IPC_Vect[Index])->Get_IPC_Context();
							fSuccess = GetOverlappedResult(  //判断一个重叠操作当前的状态        //非零表示成功，零表示失败
							pIpc->hPipeInst, // handle to pipe   
							&pIpc->oWriteOverlap, // OVERLAPPED structure   
							&cbRet,            // bytes transferred   
							FALSE); 
							if(fSuccess&&cbRet>0)
							{
								pIpc->dwState=WRITOK_STATE;
								SetEvent(pIpc->hDataEvent);	
							}
							ResetEvent(pIpc->oWriteOverlap.hEvent);
					}
					
			}else
			{
				
					printf("%d,%d,%d,%d \n",m_ArrayHandle[0],m_ArrayHandle[1],m_ArrayHandle[2],m_ArrayHandleCount);  
					DWORD dwWait = WaitForMultipleObjects(  
					m_ArrayHandleCount,    // number of event objects   
					m_ArrayHandle,      // array of event objects   
					FALSE,        // does not wait for all   
					INFINITE);
					int i = dwWait - WAIT_OBJECT_0;  
					if (i < 0 || i >(m_ArrayHandleCount - 1))  
					{  
						printf("Index out of range. %d,%d,%d,%d\n",m_ArrayHandleCount,i,WAIT_FAILED,GetLastError());  
						printf("%d,%d,%d \n",m_ArrayHandle[0],m_ArrayHandle[1],m_ArrayHandle[2]);  
						break;
					}  
					if(m_bExit)
					{
						break;
					}
					if(i%3==0)//读数据
					{
							fSuccess = GetOverlappedResult(  //判断一个重叠操作当前的状态        //非零表示成功，零表示失败
								m_ClientQy_IPC_Context.hPipeInst, // handle to pipe   
								&m_ClientQy_IPC_Context.oOverlap, // OVERLAPPED structure   
								&cbRet,            // bytes transferred   
								FALSE);            // do not wait  
							if(fSuccess)
							{
								switch (m_ClientQy_IPC_Context.dwState) 
								{ 
									case CONNECTING_STATE: 
										printf("客服端链接\n"); 
										m_ClientQy_IPC_Context.dwState = READING_STATE;
										break;
								}
								if(fSuccess&&cbRet>0)
								{
									ParseReceiveData(m_ClientQy_IPC_Context.ReceiveBuf,cbRet,m_ClientQy_IPC_Context.hPipeInst);
									m_ClientQy_IPC_Context.UpdataTime=::GetTickCount();
									int i=0;
									i++;
								}
								ReadFile(m_ClientQy_IPC_Context.hPipeInst,m_ClientQy_IPC_Context.ReceiveBuf,PipeBufferSize,NULL,  &m_ClientQy_IPC_Context.oOverlap);
							}else /********服务端已经断开********/
							{
								   ResetEvent(m_ClientQy_IPC_Context.oOverlap.hEvent);
                                   DisConnect(m_ClientQy_IPC_Context.hPipeInst);
							}
					}else if(i%3==1)//写数据
					{
						ResetEvent(m_ClientQy_IPC_Context.hDataEvent);

						if(m_ClientQy_IPC_Context.dwState==READING_STATE||m_ClientQy_IPC_Context.dwState==WRITOK_STATE)
						{
							m_Lock.Lock();
							std::map<HANDLE,std::queue<SQy_IPC_MSG*>*>::iterator It2=m_IPC_SendDataQueueMap.find(m_ClientQy_IPC_Context.hPipeInst);
							if(It2!=m_IPC_SendDataQueueMap.end()&&It2->second->size()>0)
							{
								SQy_IPC_MSG* It=It2->second->front();

								memset(m_ClientQy_IPC_Context.SendBuf,0,PipeBufferSize);
								unsigned char *pBuf=It->pBuf;
								memcpy(m_ClientQy_IPC_Context.SendBuf,pBuf,It->Len);
								m_ClientQy_IPC_Context.cbToWrite=It->Len;
								BOOL fSuccess =WriteFile(  
									m_ClientQy_IPC_Context.hPipeInst,  
									m_ClientQy_IPC_Context.SendBuf,  
									m_ClientQy_IPC_Context.cbToWrite,  
									NULL,  
									&m_ClientQy_IPC_Context.oWriteOverlap);
								if(fSuccess)
								{
									printf("写数据成功\n");
								}
								//if(It-)
								free(It->pBuf);
								free(It);
								It2->second->pop();
							}
							m_Lock.Unlock();
						}
						
						
					}else if(i%3==2)//写数据状态
					{
						fSuccess = GetOverlappedResult(  //判断一个重叠操作当前的状态        //非零表示成功，零表示失败
							m_ClientQy_IPC_Context.hPipeInst, // handle to pipe   
							&m_ClientQy_IPC_Context.oWriteOverlap, // OVERLAPPED structure   
							&cbRet,            // bytes transferred   
							FALSE);            // do not wait  
						if(fSuccess&&cbRet>0)
						{
							m_ClientQy_IPC_Context.dwState=WRITOK_STATE;
							SetEvent(m_ClientQy_IPC_Context.hDataEvent);	
						}
						ResetEvent(m_ClientQy_IPC_Context.oWriteOverlap.hEvent);					
					}
			}
		}
	}
	bool SortByM1( const SReceiveData *v1, const SReceiveData *v2)//注意：本函数的参数的类型一定要与vector中元素的类型一致  
	{  
		return v1->PktId < v2->PktId;//升序排列  
	} 
	void Qy_Ipc_Manage::ParseReceiveData(char *buf,int Len,HANDLE hPipeInst)
	{
		if(Len!=PipeBufferSize){
			assert(0);
			return;
		}
		if(Len>=4)
		{
			SQy_IPC_MSG_HEADER header;
			static int headerLen=sizeof(SQy_IPC_MSG_HEADER);
			memcpy(&header,buf,4);
			if(header.MsgType==1)
			{
				if(Len>=headerLen)
				{
					memcpy(&header,buf,headerLen);					
					char form[256]="";
					sprintf_s(form,"%d;{%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x}",hPipeInst,
						header.PktGuid.Data1,header.PktGuid.Data2,header.PktGuid.Data3,
						header.PktGuid.Data4[0],header.PktGuid.Data4[1],header.PktGuid.Data4[2],header.PktGuid.Data4[3],
						header.PktGuid.Data4[4],header.PktGuid.Data4[5],header.PktGuid.Data4[6],header.PktGuid.Data4[7]);
					
					printf("数据包：%s\n",form);
					printf("数据包：DataLen=%d\n",header.DataLen);
					printf("数据包：TotalDataLen=%d\n",header.TotalDataLen);

					
					sprintf_s(form,"%d",hPipeInst);
					if(m_pQy_HandelReceiveData!=NULL)
					{
						char *pBuf=(char *)::malloc(header.DataLen);
						memcpy(pBuf,buf+headerLen,header.DataLen);
						if(header.DataLen==header.TotalDataLen)
						{
							m_pQy_HandelReceiveData->HandelReceiveData(pBuf,header.DataLen, hPipeInst);
							free(pBuf);
						}else if(header.DataLen<header.TotalDataLen)
						{
							std::map<std::string,SReceiveCacheInfo*>::iterator It=m_IPC_ReceiveDataMap.find(form);
							if(It==m_IPC_ReceiveDataMap.end())
							{
									SReceiveCacheInfo *info =(SReceiveCacheInfo *)::malloc(sizeof(SReceiveCacheInfo));
									info->pDataList = new std::vector<SReceiveData *>();
									info->hPipeInst=hPipeInst;
									info->CurLen=header.DataLen;
									info->TotalLen=header.TotalDataLen;

									SReceiveData* pData =(SReceiveData*)::malloc(sizeof(SReceiveData));
									pData->PktId=header.PktId;
									pData->DataLen=header.DataLen;
									pData->Buf=pBuf;
									info->pDataList->push_back(pData);
									m_IPC_ReceiveDataMap.insert(std::pair<std::string, SReceiveCacheInfo*>(form,info));
									It=m_IPC_ReceiveDataMap.find(form);
							}else{
									SReceiveData* pData =(SReceiveData*)::malloc(sizeof(SReceiveData));
									pData->PktId=header.PktId;
									pData->DataLen=header.DataLen;
									pData->Buf=pBuf;
									It->second->CurLen+=header.DataLen;
									It->second->pDataList->push_back(pData);
									if(It->second->CurLen>=It->second->TotalLen)
									{
	                                     std::sort(It->second->pDataList->begin(),It->second->pDataList->end(),SortByM1); 
										 char *PtChar = (char*)::malloc(It->second->TotalLen);
										 int AcLen=0;
										 for(size_t i=0;i<It->second->pDataList->size();i++)
										 {
											  pData =It->second->pDataList->at(i);
                                              memcpy(PtChar+AcLen,pData->Buf,pData->DataLen);
											  AcLen+=pData->DataLen;
											  free(pData->Buf);
											  free(pData);
										 }
										 if(AcLen!=It->second->TotalLen)
										 {
										    assert(0);
										 }
                                        m_pQy_HandelReceiveData->HandelReceiveData(PtChar,AcLen, hPipeInst);
                                        It->second->pDataList->clear();
										delete It->second;
										delete It->second->pDataList;
										m_IPC_ReceiveDataMap.erase(It);
									}
							}

						}
					}
				}
			}
		}
	}
}
