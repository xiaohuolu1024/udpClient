#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WINSOCK2.H>
#include<windows.h>
#include<STDIO.H>

#include<iostream>
#include<functional>
#include<fstream>
#include<string>
#include<cstring>
#include<map>
#include<thread>
#include<vector>

#include <cstdlib>
#include <condition_variable>
#include <mutex>
#include<list>
#include "json/json.h"
#pragma comment(lib, "ws2_32.lib")


//定义结构体 
struct LostData//包号，时间戳
{
	long frame;
	int package;
	std::time_t chuo;
	//查找结构体e是否存在
	bool operator == (const LostData& e) 
	{
		return (this->frame == e.frame) && ((this->package == e.package));
	}
	////查找数值pos是否与posx相等
	//bool operator == (const int& package) 
	// {
	//	return (this->package == package);
	//}
	
};

class Client3
{
public:
	Client3();
	bool Client_Create_TCP_Connet();								//建立tcp连接

	void Process();

	void TCP_Describe();											//请求文件的具体参数	
	void TCP_Play();												//资源请求
	void TCP_Makeup(int flag);										//发送makeup请求。资源文件帧数据重发请求
	void TCP_Pause();												//发送pause请求。资源文件暂停推送请求
	bool GetUdpPort();												//得到udp的端口号	

	void CreateThread();											//开启线程
	void UDPThreadprocess();										//UDP数据处理
	void Recv_UDP_Data();											//接收udp数据

	int splitHandle(char* buff, long frame, int nRes);				//处理UDP数据

	void IOThreadWriteFile();										//写文件线程
	void IOWriteFile();
	void openWriteFile();
	void writeFile(char* buff, int len);
	void closeWriteFile();

	void UDPMonitoringData();										//UDP数据监视线程


	//工具函数
	std::string LongToS(long lon);									//long转string
	int packageCount(long FrameLength);								//计算包数量
	int settimeout();												//设置recvfrom超时时间
	void StringtoSplit(const std::string& str, const char split, std::vector<std::string>& res);//切片函数
	bool checkComplete(std::vector<char*>& temps);					//判断该帧是否是整帧
	void DeleteAllLastData();										//结束之后free内存
	void initFrameintegrity();										//判断帧头包是否已经收到
	void AddFrameintegrity(long framenumber);						//每获得一个帧头包，就+1
	bool JudgeFrameintegrity(long framenumber);						//判断



private:
	WORD sockVersion;
	SOCKET sclient;
	sockaddr_in serAddr;

	SOCKET UDPSocketfd;
	sockaddr_in ReceiverAddr;

	const char* TcpSendBuff = "0094{\"cmd\":\"describe\",\"cmdId\":\"07e40042cf0948d884d3b9ce821513a5\",\"clientId\":\"A1B1\",\"resourceId\":1}";
	char recvTCPBuff[1400];

	char* recvUDPBuf = new char[1500];							//1500字节

	//std::string playSendBuff = "{\"cmd\":\"play\",\"cmdId\":\"d9409cb2e3bc43c581a945594c6059da\",\"clientId\":\"A1B1\",\"resourceId\":1,\"startFrameIndex\":"+ startIndex +",\"receivePort\":" + UDPport + "}";
	std::string pauseSendBuff = "0091{\"cmd\":\"pause\",\"cmdId\":\"600f4e58afdf4abfb143b6750e3183c7\",\"clientId\":\"A1B1\",\"resourceId\":1}";

	//std::string makeupSendBuff_0 = "{\"cmd\":\"makeup\",\"cmdId\":\"94bd332b233e497a87905bd4dea86800\",\"clientId\":\"A1B1\",\"resourceId\":1,\"makeupIndex\":" + makeupIndex + ",\"receivePort\":" + UDPport + "}";
	//std::string makeupSendBuff_1 = "{\"cmd\":\"makeup\",\"cmdId\":\"94bd332b233e497a87905bd4dea86800\",\"clientId\":\"A1B1\",\"resourceId\":1,\"makeupIndex\":" + makeupIndex + ",\"makeupPackageIndex\":[" + makeupPackageIndex + "],\"receivePort\":" + UDPport + "}";
	
	std::string makeupIndex = "";								//请求重发的帧号
	std::string makeupPackageIndex = "";						//请求重发的包号

	long FrameCount = 0;										//能保存全部帧的计数大小，
	long CurrentFrameNumber = 0;								//当前已经发送的帧
	long shouldFrameNumber = 1;
	long maxrecvfram = 0;

	int udpdatabuffnumber = 5;
	std::string UDPport = "";									//udp端口号
	std::string startIndex = "";								//请求重发的开始帧号
	char* frameDataintegrity;									//判断帧头是否收到
	//创建缓冲区
	static const int DATABUFFmax = 200;							//map最多存放50帧

	std::vector<int>DataBuffMAP2;								//记录databuff的数据位置
	std::vector< std::vector<char*> > DataBuff;
	bool DataBuffFlag[DATABUFFmax];								//复写标志位

	
	std::mutex readwritemtx;									//互斥量,保护缓冲区
	std::mutex filelengthmtx;									//互斥量,保护缓冲区
	

	std::vector<bool>waitwriteframe;							//等待写的帧号
	std::atomic<bool> ioflag = false;							//通知io线程可写的flag
	long blockedFrameNumber = 1;								//当前被阻塞的帧号
	


	std::map<long, std::vector<char*>> IODATABUFF;				//IO线程内，存储数据
	std::map<long, std::vector<int>>oneBuffLenght;				//存储每个buff的长度

	std::fstream fsfile;										//写文件流


	std::vector<LostData> RESEND;								//记录缺失的包和帧的数据
	std::map<LostData, int> max_resend_number_map;				//记录帧的最大重发次数，
	int  max_resend_number = 5;
	//帧long 包int 时间戳std::time_t
	//设置变量，记录当前包的上一个包的个数，以及包号，以及帧号。
	long fenbaoNote  = 1;


	//某些标志位是否要设置成原子变量
	//1存在这一种问题，当缺失的包没有收到，会一直在buff中，后续因为缺失的包太多，没有及时收到重发的包，导致缓冲区溢出


	//bool canread = true;
	//bool canwrite = true;
	//std::condition_variable not_full;								//条件变量, 指示缓冲区不为满.
	//std::condition_variable not_empty;							//条件变量, 指示缓冲区不为空.
	//std::mutex mtx;												//互斥量,保护缓冲区
};