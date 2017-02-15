#ifndef NoNeedWindowHeade
#include <Windows.h>
#endif
#include <vector>
#ifndef Qy_IPC_Context_H_
#define Qy_IPC_Context_H_
namespace Qy_IPC
{
	#define PipeBufferSize 1024*100
	#define CONNECTING_STATE 0   
	#define READING_STATE 1   
	#define WRITING_STATE 2  
	#define WRITOK_STATE 3  
	typedef struct SQy_IPC_Context
	{  
		//管道重叠
		OVERLAPPED oOverlap;  
		//写的
		OVERLAPPED oWriteOverlap;  
		//管道句柄
		HANDLE hPipeInst;  
		//有数据的事件
		HANDLE hDataEvent;
		//实际读入的字节数
		DWORD cbRead;   
		//要写入的字节数
		DWORD cbToWrite;
		//状态
		DWORD dwState;   //0,准备链接，1已连接
		int UpdataTime;//更新时间
		char ReceiveBuf[PipeBufferSize];
		char SendBuf[PipeBufferSize];
	}SQy_IPC_Context; 
#pragma  pack(1)
	typedef struct SQy_IPC_MSG_HEADER
	{
	   int	     MsgType;
	   GUID	     PktGuid;
	   //分包ID
       int       PktId;
	   //本包数据长度
       unsigned int DataLen;
	   unsigned int DataSum;
	   //数总长度
	   unsigned int TotalDataLen;

	} SQy_IPC_MSG_HEADER;
#pragma pack()
	typedef struct SQy_IPC_MSG
	{
		unsigned char *pBuf;
		unsigned int	 Len;
		//管道句柄
		HANDLE hPipeInst;
	}SQy_IPC_MSG;
    
}
#endif