#pragma once
#include <string>
#ifndef IQy_Ipc_Base_H_
#define IQy_Ipc_Base_H_
namespace Qy_IPC
{
	class IQy_Ipc_Base
	{
	     public:
			  //写数据
			   virtual void WriteData(unsigned char* buf,int len)=0;
			   //是否超时
			   virtual bool GetTimeOut(int CurTime)=0;
			   virtual bool ProcessConnection()=0;
			   virtual bool Disconnect()=0;
	};
	class IQy_HandelReceiveData
	{
	     public:
			    virtual void HandelReceiveData(char *buf,int Len,std::string strId)=0;
	};
}
#endif
