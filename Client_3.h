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


//����ṹ�� 
struct LostData//���ţ�ʱ���
{
	long frame;
	int package;
	std::time_t chuo;
	//���ҽṹ��e�Ƿ����
	bool operator == (const LostData& e) 
	{
		return (this->frame == e.frame) && ((this->package == e.package));
	}
	////������ֵpos�Ƿ���posx���
	//bool operator == (const int& package) 
	// {
	//	return (this->package == package);
	//}
	
};

class Client3
{
public:
	Client3();
	bool Client_Create_TCP_Connet();								//����tcp����

	void Process();

	void TCP_Describe();											//�����ļ��ľ������	
	void TCP_Play();												//��Դ����
	void TCP_Makeup(int flag);										//����makeup������Դ�ļ�֡�����ط�����
	void TCP_Pause();												//����pause������Դ�ļ���ͣ��������
	bool GetUdpPort();												//�õ�udp�Ķ˿ں�	

	void CreateThread();											//�����߳�
	void UDPThreadprocess();										//UDP���ݴ���
	void Recv_UDP_Data();											//����udp����

	int splitHandle(char* buff, long frame, int nRes);				//����UDP����

	void IOThreadWriteFile();										//д�ļ��߳�
	void IOWriteFile();
	void openWriteFile();
	void writeFile(char* buff, int len);
	void closeWriteFile();

	void UDPMonitoringData();										//UDP���ݼ����߳�


	//���ߺ���
	std::string LongToS(long lon);									//longתstring
	int packageCount(long FrameLength);								//���������
	int settimeout();												//����recvfrom��ʱʱ��
	void StringtoSplit(const std::string& str, const char split, std::vector<std::string>& res);//��Ƭ����
	bool checkComplete(std::vector<char*>& temps);					//�жϸ�֡�Ƿ�����֡
	void DeleteAllLastData();										//����֮��free�ڴ�
	void initFrameintegrity();										//�ж�֡ͷ���Ƿ��Ѿ��յ�
	void AddFrameintegrity(long framenumber);						//ÿ���һ��֡ͷ������+1
	bool JudgeFrameintegrity(long framenumber);						//�ж�



private:
	WORD sockVersion;
	SOCKET sclient;
	sockaddr_in serAddr;

	SOCKET UDPSocketfd;
	sockaddr_in ReceiverAddr;

	const char* TcpSendBuff = "0094{\"cmd\":\"describe\",\"cmdId\":\"07e40042cf0948d884d3b9ce821513a5\",\"clientId\":\"A1B1\",\"resourceId\":1}";
	char recvTCPBuff[1400];

	char* recvUDPBuf = new char[1500];							//1500�ֽ�

	//std::string playSendBuff = "{\"cmd\":\"play\",\"cmdId\":\"d9409cb2e3bc43c581a945594c6059da\",\"clientId\":\"A1B1\",\"resourceId\":1,\"startFrameIndex\":"+ startIndex +",\"receivePort\":" + UDPport + "}";
	std::string pauseSendBuff = "0091{\"cmd\":\"pause\",\"cmdId\":\"600f4e58afdf4abfb143b6750e3183c7\",\"clientId\":\"A1B1\",\"resourceId\":1}";

	//std::string makeupSendBuff_0 = "{\"cmd\":\"makeup\",\"cmdId\":\"94bd332b233e497a87905bd4dea86800\",\"clientId\":\"A1B1\",\"resourceId\":1,\"makeupIndex\":" + makeupIndex + ",\"receivePort\":" + UDPport + "}";
	//std::string makeupSendBuff_1 = "{\"cmd\":\"makeup\",\"cmdId\":\"94bd332b233e497a87905bd4dea86800\",\"clientId\":\"A1B1\",\"resourceId\":1,\"makeupIndex\":" + makeupIndex + ",\"makeupPackageIndex\":[" + makeupPackageIndex + "],\"receivePort\":" + UDPport + "}";
	
	std::string makeupIndex = "";								//�����ط���֡��
	std::string makeupPackageIndex = "";						//�����ط��İ���

	long FrameCount = 0;										//�ܱ���ȫ��֡�ļ�����С��
	long CurrentFrameNumber = 0;								//��ǰ�Ѿ����͵�֡
	long shouldFrameNumber = 1;
	long maxrecvfram = 0;

	int udpdatabuffnumber = 5;
	std::string UDPport = "";									//udp�˿ں�
	std::string startIndex = "";								//�����ط��Ŀ�ʼ֡��
	char* frameDataintegrity;									//�ж�֡ͷ�Ƿ��յ�
	//����������
	static const int DATABUFFmax = 200;							//map�����50֡

	std::vector<int>DataBuffMAP2;								//��¼databuff������λ��
	std::vector< std::vector<char*> > DataBuff;
	bool DataBuffFlag[DATABUFFmax];								//��д��־λ

	
	std::mutex readwritemtx;									//������,����������
	std::mutex filelengthmtx;									//������,����������
	

	std::vector<bool>waitwriteframe;							//�ȴ�д��֡��
	std::atomic<bool> ioflag = false;							//֪ͨio�߳̿�д��flag
	long blockedFrameNumber = 1;								//��ǰ��������֡��
	


	std::map<long, std::vector<char*>> IODATABUFF;				//IO�߳��ڣ��洢����
	std::map<long, std::vector<int>>oneBuffLenght;				//�洢ÿ��buff�ĳ���

	std::fstream fsfile;										//д�ļ���


	std::vector<LostData> RESEND;								//��¼ȱʧ�İ���֡������
	std::map<LostData, int> max_resend_number_map;				//��¼֡������ط�������
	int  max_resend_number = 5;
	//֡long ��int ʱ���std::time_t
	//���ñ�������¼��ǰ������һ�����ĸ������Լ����ţ��Լ�֡�š�
	long fenbaoNote  = 1;


	//ĳЩ��־λ�Ƿ�Ҫ���ó�ԭ�ӱ���
	//1������һ�����⣬��ȱʧ�İ�û���յ�����һֱ��buff�У�������Ϊȱʧ�İ�̫�࣬û�м�ʱ�յ��ط��İ������»��������


	//bool canread = true;
	//bool canwrite = true;
	//std::condition_variable not_full;								//��������, ָʾ��������Ϊ��.
	//std::condition_variable not_empty;							//��������, ָʾ��������Ϊ��.
	//std::mutex mtx;												//������,����������
};