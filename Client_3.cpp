#include "Client_3.h"

#define MAX_NETWORK_LENGTH 1400
#pragma comment(lib, "ws2_32.lib")
Client3::Client3()
{
	memset(DataBuffFlag, true, DATABUFFmax);
	DataBuff.resize(DATABUFFmax);
	RESEND.reserve(DATABUFFmax * 2);
}
bool Client3::Client_Create_TCP_Connet()
{
	sockVersion = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(sockVersion, &data) != 0)
	{
		return 0;
	}
	sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sclient == INVALID_SOCKET)
	{
		printf("�׽�����Ч");
		return false;
	}

	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8172);
	serAddr.sin_addr.s_addr = inet_addr("8.134.86.82");
	if (connect(sclient, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //����ʧ�� 
		printf("connect error !/����ʧ��");
		closesocket(sclient);
		return false;
	}
	return true;
}
void Client3::TCP_Describe()
{
	send(sclient, TcpSendBuff, strlen(TcpSendBuff), 0);
	memset(recvTCPBuff, 0, sizeof(recvTCPBuff));
	int ret = recv(sclient, recvTCPBuff, sizeof(recvTCPBuff), 0);

	if (ret == 0)
	{
		printf("connet close...... ");
		printf("errno = %d\n", WSAGetLastError());//С��0�ķ���SOCKET_ERRORֵ�����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣
		return;
	}

	std::string s_recdata = recvTCPBuff;
	if (s_recdata.size() == 0)
	{
		printf("error, ��������Ϊ��");//����Ϊ�գ��ᵼ��substr��ȡ���󣬱�����
		return;
	}
	s_recdata = s_recdata.substr(4, s_recdata.length());
	Json::Reader reader;
	Json::Value value;

	std::string C_md,C_mdId,S_uccess, M_essage,R_esolution;
	int F_ramerate;
	long long  L_ength;
	long F_rameCount;
	if (reader.parse(s_recdata, value))//������json�ŵ�json����
	{
		C_md = value["cmd"].asString();
		C_mdId = value["cmdId"].asString();
		S_uccess = value["success"].asString();
		std::cout << C_md <<"   " << C_mdId << "   " << S_uccess << std::endl;
		Json::Value arrayObj = value["message"];
		R_esolution = arrayObj["resolution"].asString();
		F_ramerate = arrayObj["framerate"].asInt();
		L_ength = arrayObj["length"].asUInt64();
		F_rameCount = arrayObj["frameCount"].asUInt64();

		//savefilePostion = new std::map<long, long>;
		std::cout << R_esolution << std::endl;
		std::cout << F_ramerate << std::endl;
		std::cout << L_ength << std::endl;
		std::cout << F_rameCount << std::endl;
		this->FrameCount = F_rameCount;
		//initFrameintegrity();//ͳ���Ѿ������֡��
		waitwriteframe.resize(FrameCount+1);//782֡�ĳ���.
		std::fill(waitwriteframe.begin(), waitwriteframe.end(), false);

		DataBuffMAP2.resize(FrameCount + 1);//buff��¼
		std::fill(DataBuffMAP2.begin(), DataBuffMAP2.end(), -1);
	}
}

void Client3::TCP_Play()
{
	std::string playSendBuff = "{\"cmd\":\"play\",\"cmdId\":\"d9409cb2e3bc43c581a945594c6059da\",\"clientId\":\"A1B1\",\"resourceId\":1,\"startFrameIndex\":" + startIndex + ",\"receivePort\":" + UDPport + "}";

	int len = playSendBuff.size();
	std::string lenght = std::to_string(len);

	std::string sssend = "0" + lenght + playSendBuff;

	const char* playbuff = sssend.c_str();
	int s = sizeof(playbuff);
	int n = send(sclient, playbuff, strlen(playbuff), 0);

	memset(recvTCPBuff, 0, sizeof(recvTCPBuff));
	int retq = recv(sclient, recvTCPBuff, sizeof(recvTCPBuff), 0);

	if (retq <= 0)
	{
		printf("connet close...... ");
		printf("errno = %d\n", WSAGetLastError());//С��0�ķ���SOCKET_ERRORֵ�����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣
		return;
	}

	std:: string s_recdata = recvTCPBuff;
	if (s_recdata.size() == 0)
	{
		printf("δ�յ����ݡ���");
		return;
	}
	s_recdata = s_recdata.substr(4, s_recdata.length());
	Json::Reader reader;//json����
	Json::Value value;//��ʾһ��json��ʽ�Ķ���

	std::string C_md,C_mdId,S_uccess,M_essage,R_esolution;
	//int F_ramerate;
	//long long  L_ength;
	//unsigned int F_rameCount;
	if (reader.parse(s_recdata, value))//������json�ŵ�json����
	{
		C_md = value["cmd"].asString();
		C_mdId = value["cmdId"].asString();
		S_uccess = value["success"].asString();
		std::cout << "playcmd�����" <<"     " << C_md << "     " << C_mdId << "     " << S_uccess << std::endl;
	}
	
}

void Client3::TCP_Makeup(int flag)
{
	std::string ssssend;
	if (0 == flag)//�ط���֡
	{
		std::string makeupSendBuff_0 = "{\"cmd\":\"makeup\",\"cmdId\":\"94bd332b233e497a87905bd4dea86800\",\"clientId\":\"A1B1\",\"resourceId\":1,\"makeupIndex\":" + makeupIndex + ",\"receivePort\":" + UDPport + "}";
		int len = makeupSendBuff_0.size();
		std::string length = std::to_string(len);
		ssssend = "0" + length + makeupSendBuff_0;
	}
	else if (1 == flag)//�ط�֡�ְ�
	{
		std::string makeupSendBuff_1 = "{\"cmd\":\"makeup\",\"cmdId\":\"94bd332b233e497a87905bd4dea86800\",\"clientId\":\"A1B1\",\"resourceId\":1,\"makeupIndex\":" + makeupIndex + ",\"makeupPackageIndex\":[" + makeupPackageIndex + "],\"receivePort\":" + UDPport + "}";

		int len = makeupSendBuff_1.size();
		std::string lenght = std::to_string(len);
		ssssend = "0" + lenght + makeupSendBuff_1;
	}

	//�����ַ����ĳ���
	const char* pausebuff = ssssend.c_str();
	int n = send(sclient, pausebuff, strlen(pausebuff), 0);

	memset(recvTCPBuff, 0, sizeof(recvTCPBuff));
	int retq = recv(sclient, recvTCPBuff, 1400, 0);

	if (retq <= 0)
	{
		printf("connet close...... ");
		printf("errno = %d\n", WSAGetLastError());//С��0�ķ���SOCKET_ERRORֵ�����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣
		exit(-1);
		return;
	}

	std::string s_recdata = recvTCPBuff;
	if (s_recdata.size() == 0)
	{
		printf("δ�յ����ݡ���");
		return;
	}
	s_recdata = s_recdata.substr(4, s_recdata.length());
	Json::Reader reader;//json����
	Json::Value value;//��ʾһ��json��ʽ�Ķ���

	std::string C_md, C_mdId, S_uccess, M_essage, R_esolution;
	if (reader.parse(s_recdata, value))//������json�ŵ�json����
	{
		C_md = value["cmd"].asString();
		C_mdId = value["cmdId"].asString();
		S_uccess = value["success"].asString();
		//std::cout << "makeup_cmd���������"<< S_uccess << std::endl;
	}
}

void Client3::TCP_Pause()
{
	const char* pausebuff = pauseSendBuff.c_str();
	int n = send(sclient, pausebuff, strlen(pausebuff), 0);
	memset(recvTCPBuff, 0, sizeof(recvTCPBuff));

	int retq = recv(sclient, recvTCPBuff, 1400, 0);

	if (retq <= 0)
	{
		printf("connet close...... ");
		printf("errno = %d\n", WSAGetLastError());//С��0�ķ���SOCKET_ERRORֵ�����ҿ���ͨ������ WSAGetLastError �������ض��Ĵ�����롣
		return;
	}

	std::string s_recdata = recvTCPBuff;
	if (s_recdata.size() == 0)
	{
		printf("δ�յ����ݡ���");
		return;
	}
	s_recdata = s_recdata.substr(4, s_recdata.length());
	Json::Reader reader;//json����
	Json::Value value;//��ʾһ��json��ʽ�Ķ���

	std::string  S_uccess, R_esolution;
	
	if (reader.parse(s_recdata, value))//������json�ŵ�json����
	{
		S_uccess = value["success"].asString();
		std::cout << "TCP_Pause()�����" <<  S_uccess << std::endl;
		
	}
}
bool Client3::GetUdpPort()
{
	char recvServerBuf[100];
	UDPSocketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocketfd == INVALID_SOCKET)
	{
		printf("Client: Error at socket(): %ld\n", WSAGetLastError());// ���������Ϣ
		WSACleanup();
		exit(-1);
	}
	
	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(8173);
	ReceiverAddr.sin_addr.s_addr = inet_addr("8.134.86.82");

	int len = sizeof(SOCKADDR);
	char cmd[18] = { '!' - 32, 'r', 'e','c','e', 'i', 'v', 'e', '_', 'p','o','r','t','|', 'a', 'b', 'c', '|' };
	//char cmd[18] = {'!'-32, 114,101,99, 101, 105, 118, 101, 95, 112, 111, 114, 116, 124, 97, 98, 99, 124};

	sendto(UDPSocketfd, cmd, 18, 0, (SOCKADDR*)&ReceiverAddr, len);
	memset(recvServerBuf, 0, sizeof(recvServerBuf));
	int nRes = recvfrom(UDPSocketfd, recvServerBuf, 100, 0, (SOCKADDR*)&ReceiverAddr, &len);
	if (nRes <= 0)
	{
		printf("udp client connect close, please check....\n");
		exit(-1);
	}
	printf("%s\n", recvServerBuf);

	std::string recvbuf = recvServerBuf;

	auto n = recvbuf.find_last_of('|');
	UDPport = recvbuf.substr(n + 1, recvbuf.size());
	return true;
}
void Client3::Recv_UDP_Data()
{
	int len = sizeof(SOCKADDR);
	memset(recvUDPBuf, 0, 1500);
	settimeout();						//���ó�ʱʱ��0.25��
	bool flag = true;
	do
	{
		flag = true;
		while (true)
		{
			memset(recvUDPBuf, 0, sizeof(recvUDPBuf));
			int nRes = recvfrom(UDPSocketfd, recvUDPBuf, 1500, 0, (SOCKADDR*)&ReceiverAddr, &len);
			
			if (nRes == 0)
			{
				std::cout << "���ӹر�" << std::endl;
				return;
			}
			else if (nRes < 0 && 10060 == GetLastError())
			{
				std::cout << "recvfrom�ȴ���ʱ������" << std::endl;
				break;
			}
			else if (nRes < 0)
			{
				printf("errno = %d\n", WSAGetLastError());
				exit(-1);
			}
			long FrameNumber = 0;
			int flag = splitHandle(recvUDPBuf, FrameNumber, nRes);
			if (flag == 200)
			{

			}	
			else if (flag == 400)
			{
				std::cout << "ERROR" << std::endl;
				exit(-1);
			}
		}
		//��ѯһ��databuff�鿴���������Ƿ������ݣ��Ƿ��ǿɸ�дλ��
		int num = 0;
		
		
		//��ѯһ��resendbuff�鿴���������Ƿ�������
		if (RESEND.size() != 0)//����������Ϊ�գ�˵������֡�ڱ�����
		{
			flag = false;
			/*std::cout << "RESEND no empty" << std::endl;*/
		}
		while (IODATABUFF.size() != 0 && CurrentFrameNumber == FrameCount)
		{
			//std::cout << "IO blocked" << std::endl;
			ioflag = true;
			flag = false;
			break;
		}
		for (int i = 0; i < DATABUFFmax; ++i)
		{
			if (DataBuffFlag[i] == false)//��дλΪfalse��˵������֡�ڻ�����
			{
				++num;
				ioflag = true;
				flag = false;
				//break;
				
			}
		}
		std::cout <<"databuff�Ĵ�С" << num << std::endl;
		std::cout << "resendbuff�Ĵ�С" << RESEND.size() << std::endl;
		std::cout << "IODATABUFF�Ĵ�С" << IODATABUFF.size() << std::endl;
		std::cout << "���յ������֡�ţ�" << maxrecvfram << std::endl;
		if (num > 0 && num < 3 && RESEND.size() == 0 && IODATABUFF.size() == 0)
		{
			if (udpdatabuffnumber == 0)
			{
				int i = 1;
				for (; i < DataBuffMAP2.size(); ++i)//����ټ��һ��databuff2���ĸ�֡�Ų�Ϊ-2
				{
					if (DataBuffMAP2[i] != -2)
					{
						std::cout << "����֡δ����:" << i << std::endl;
					}
				}
				std::cout << "�������͵�֡�ţ�" << i << std::endl;
				std::string contin = LongToS(i);
				this->startIndex = contin;
				TCP_Play();

			}
			--udpdatabuffnumber;
		}
		

		//if����������Ϊ�գ�֡�Ų��ԣ��ǾʹӶ�Ӧ֡��ʼ��

		if (maxrecvfram != FrameCount && flag )
		{
			if (maxrecvfram != FrameCount && maxrecvfram > FrameCount/2)
			{
				std::cout << "�������͵�֡�ţ�" << maxrecvfram + 1 << std::endl;
	   			std::string contin = LongToS(maxrecvfram + 1);
	   			this->startIndex = contin;
	   			TCP_Play();
			}
		}
		if (maxrecvfram == FrameCount && flag)
		{
			break;
		}
		
	} while (true);//��ǰ֡�� != ��֡
	
	return;
}
int  Client3::splitHandle(char* Framebuff, long frame, int nRes)//UDP���Ĵ���
{
	int headLength = stoi(std::string{ Framebuff[1], Framebuff[2] });//�ײ�����
	std::string head_Total_Data = "";
	for (int i = 3; i < headLength + 3; ++i)
	{
		head_Total_Data = head_Total_Data + Framebuff[i];
	}
	int num = count(head_Total_Data.begin(), head_Total_Data.end(), '|');
	std::vector<std::string> strList;
	strList.clear();
	StringtoSplit(head_Total_Data, '|', strList);

	if (num != 0)
	{
		if (4 == num)//��һ����
		{
			long firstframe = stol(strList[0]);
			int firstpackagemap = stoi(strList[1]);
			long FrameLength = stol(strList[3]);
			
			LostData qqq = { firstframe , firstpackagemap };		//�жϽ��հ��Ƿ��Ƕ�ʧ���ش�
			auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
			if (iterat == RESEND.end() && firstframe <= CurrentFrameNumber)//�������������ڣ���֡�űȵ�ǰС
			{
				std::cout <<"��֡�ٴ��ط���" << firstframe<<"����" << std::endl;
				return 100;
			}
			if (iterat != RESEND.end())//�յ��ط���֡
			{
				readwritemtx.lock();	
				std::cout<<"�յ��ط���֡��"<< firstframe<<"һ������" << packageCount(FrameLength) << std::endl;
				RESEND.erase(iterat);
				readwritemtx.unlock();				
			}
			
			if (DataBuffMAP2[firstframe] == -2)
			{
				return 100;
			}
			std::vector<char*> res;
			std::vector<int> vecbuflen;
			//vecbuflen.resize(packageCount(FrameLength));
			for (int i = 0; i < packageCount(FrameLength); ++i)
			{
				res.push_back(nullptr);
				vecbuflen.push_back(0);
			}
			//��ȡ��������
			res[firstpackagemap - 1] = new char[1500];
			memset(res[firstpackagemap - 1], 1, 1500);

			for (int i = headLength + 3; i <= nRes; ++i)//���ַ�����
			{
				res[firstpackagemap - 1][i - headLength - 3] = Framebuff[i];
			}
			bool f = true;
			for (int arr = 0; arr < DATABUFFmax; ++arr)//Ѱ�ҿɸ�д��λ
			{
				if (DataBuffFlag[arr])
				{
					////std::cout << "����֡��:" << firstframe << "   ���հ���:" << firstpackagemap << " ����" << packageCount(FrameLength) << "��" << std::endl;
					//if (DataBuffMAP2[firstframe] == -2)
					//{
					//	printf("���·�������֡�ط�\n");
					//	return 100;
					//}

					//if (DataBuffMAP2[firstframe] != -1)//�յ��ط�����֡,��֮ǰ���ظ���
					//{
					//	for (int j = 0; j < DataBuff[DataBuffMAP2[firstframe]].size(); ++j)
					//	{
					//		delete[] DataBuff[DataBuffMAP2[firstframe]][j];
					//		DataBuff[DataBuffMAP2[firstframe]][j] = nullptr;
					//	}
					//	DataBuffFlag[DataBuffMAP2[firstframe]] = true;
					//}
					
					DataBuff[arr] = res;
					DataBuffMAP2[firstframe] = arr;//vector�洢֡�ţ��Լ���Ӧ���±�λ�á�
					vecbuflen[firstpackagemap - 1] = nRes - headLength - 3;
					filelengthmtx.lock();
					oneBuffLenght.insert(std::pair<long, std::vector<int>>(firstframe, vecbuflen));
					filelengthmtx.unlock();

					if (firstframe != shouldFrameNumber)//֡û���򵽴�
					{
						readwritemtx.lock();
						int iii = shouldFrameNumber - 1;//���Ӧ�ô���֡��ǰһ��֡�Ƿ�����
						if (DataBuffMAP2[iii] != -2 && DataBuffMAP2[iii] != -1)
						{
							for (int j = 1; j < DataBuff[DataBuffMAP2[iii]].size(); ++j)
							{
								if (DataBuff[DataBuffMAP2[iii]][j] == nullptr)
								{
									std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
									auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
									std::time_t timestampqqqq = tmp.count();
									LostData aaa = { iii, j + 1, timestampqqqq };
									RESEND.push_back(aaa);
								}
							}
						}
						for (long rr = shouldFrameNumber; rr < firstframe; ++rr)//�۲�resend��databuff���Ƿ���ڵ�ǰ֡�������򲻴���
						{
							std::cout << rr << "֡δ��ȷ����Ѵ��루udp��һ����" << std::endl;
							std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
							auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
							std::time_t timestampqqqq = tmp.count();
							//��Ƭ��֡û�յ���
							LostData aaa = { rr, 1, timestampqqqq };
							RESEND.push_back(aaa);
						}
						readwritemtx.unlock();

					}//���ж������ŵ���ǰ֡���Ͼ���֮ǰȱʧ��֡�����Ѽ�¼����ȱʧ��֡�յ�֮��Ҳ�Ƚ�һ�£�С���ˣ��Ͳ���ֵ
					if (firstframe >= shouldFrameNumber)
					{
						shouldFrameNumber = firstframe + 1;
					}
					DataBuffFlag[arr] = false;
					f = false;
					break;
				}
			}
			if (f)
			{
				readwritemtx.lock();
				std::cout << "����������" << std::endl;
				//std::cout << "���һ֡�ţ�" << firstframe << std::endl;
				//std::cout << rr << "֡δ��ȷ����Ѵ��루udp��һ����" << std::endl;
				std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
				auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
				std::time_t timestampqqqq = tmp.count();
				//��Ƭ��֡û�յ���
				LostData aaa = { firstframe, 1, timestampqqqq };
				RESEND.push_back(aaa);
				readwritemtx.unlock();
				//exit(-1);
			}

			CurrentFrameNumber = firstframe;
			if (maxrecvfram <= CurrentFrameNumber)
				maxrecvfram = CurrentFrameNumber;
			
			if (checkComplete(DataBuff[DataBuffMAP2[firstframe]]))//�ж��Ƿ�֡�Ƿ���������֪ͨio�߳�
			{
				waitwriteframe[firstframe] = true;//֪ͨIO�̸߳�֡��д
				ioflag = true;
			}
			if (firstframe == FrameCount)
			{
				std::cout << "��Ƭ�У����ܵ�������֡�ţ�" << CurrentFrameNumber << std::endl;
				CurrentFrameNumber = firstframe;//��¼���յ����һ֡
			}
		}
		else if (1 == num)//�����ְ�
		{
			long framemap = stol(strList[0]);
			int  packagemap = stoi(strList[1]);
			
			
			//Ψ�����������⡣�����������ط�
			//ͬһ֡
			//��ͬ֡

			//��˳�����ͬһ֡�ķְ�
			if (CurrentFrameNumber == framemap)
			{
				int check = DataBuffMAP2[framemap];
				//��ͬһ֡���򣩾��жϰ���һ�²�����һ�£��ʹ�������߳�
				if (check != -2)
				{
					if (DataBuff[check][0] != nullptr)
					{
						for (int i = 1; i < DataBuff[check].size(); ++i)
						{
							if (DataBuff[check][i] == nullptr)
							{	
								if (packagemap - 1 == i)//��ȷ���հ�
								{
									DataBuff[check][packagemap - 1] = new char[1500];
									memset(DataBuff[check][packagemap - 1], 0, 1500);

									for (int i = headLength + 3; i <= nRes; i++)
									{
										DataBuff[check][packagemap - 1][i - headLength - 3] = Framebuff[i];
									}
									filelengthmtx.lock();
									std::map<long, std::vector<int>>::iterator iter = oneBuffLenght.find(framemap);
									iter->second[packagemap - 1] = nRes - headLength - 3;
									filelengthmtx.unlock();

									if (checkComplete(DataBuff[check]))
									{
										waitwriteframe[framemap] = true;//֪ͨIO�̸߳�֡��д
										ioflag = true;
									}
									break;
								}
								else if (packagemap - 1 != i)//�����򡣼�¼Ӧ�յİ�
								{
									readwritemtx.lock();
									std::cout << framemap << "֡��" << i + 1 << " ��δ��ȷ����Ѵ���" << std::endl;
									std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
									auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
									std::time_t timestampqqqq = tmp.count();
									LostData aaa = { framemap, i + 1, timestampqqqq };
									RESEND.push_back(aaa);
									readwritemtx.unlock();

									//���浱ǰ��������
									DataBuff[check][packagemap - 1] = new char[1500];
									memset(DataBuff[check][packagemap - 1], 0, 1500);
									for (int i = headLength + 3; i <= nRes; i++)
									{
										DataBuff[check][packagemap - 1][i - headLength - 3] = Framebuff[i];
									}
									filelengthmtx.lock();
									std::map<long, std::vector<int>>::iterator iter = oneBuffLenght.find(framemap);								
									iter->second[packagemap - 1] = nRes - headLength - 3;
									filelengthmtx.unlock();
									break;
								}
							}
						}
					}
					else
					{
						std::cout << "�װ�������" << std::endl;
					}
				}
				else
				{
					std::cout << framemap << "֡����,(�ְ���)" << std::endl;
					//�鿴���ӻ����������ޣ�����ɾ��
					LostData qqq = { framemap , packagemap };
					int check = DataBuffMAP2[framemap];
					auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
					if (iterat != RESEND.end())
					{
						readwritemtx.lock();
						RESEND.erase(iterat);
						readwritemtx.unlock();
					}
					return 100;
				}
				
			}
			//�͵�ǰ֡�Ų�һ�£������ǲ����ģ�Ҳ��������֡��İ���
			if (CurrentFrameNumber != framemap)
			{
				LostData qqq = { framemap , packagemap };
				int check = DataBuffMAP2[framemap];
				auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
				if (iterat == RESEND.end() && framemap < CurrentFrameNumber && check == -2)
				{
					std::cout << "��" << framemap << "��" << packagemap << "�ٴ��ط�������" << std::endl;
					return 100;
				}
				if (check != -1)
				{
					if (check == -2)//֡�����ǽ��յ����ط���
					{
						std::cout << "�ð���֡" << framemap << "����" << std::endl;
						if (iterat != RESEND.end())
						{
							readwritemtx.lock();
							RESEND.erase(iterat);
							readwritemtx.unlock();
						}					
						return 100;
					}

					if (DataBuff[check][0] != nullptr)//�װ����ڣ���ȷ
					{
						if (DataBuff[check][packagemap - 1] == nullptr)//�ְ���Ӧ��λ��Ϊ�գ���ȷ
						{
							if (iterat != RESEND.end())//�ڼ����߳����ҵ���Ӧ�ְ���¼����ȷ
							{
								readwritemtx.lock();
								RESEND.erase(iterat);
								readwritemtx.unlock();
								//��ȡ�ְ����ݴ�������
								DataBuff[check][packagemap - 1] = new char[1500];
								memset(DataBuff[check][packagemap - 1], 0, 1500);

								for (int i = headLength + 3; i <= nRes; i++)//���ַ�������Ƶ����.
								{
									DataBuff[check][packagemap - 1][i - headLength - 3] = Framebuff[i];
								}
								filelengthmtx.lock();
								std::map<long, std::vector<int>>::iterator iter = oneBuffLenght.find(framemap);		
								iter->second[packagemap - 1] = nRes - headLength - 3;//������ݳ���
								filelengthmtx.unlock();
								if (checkComplete(DataBuff[DataBuffMAP2[framemap]]))//�������ˣ� ��֪ͨio
								{
									std::cout << framemap <<"֡" << packagemap << "��д" << std::endl;
									waitwriteframe[framemap] = true;//֪ͨIO�̸߳�֡��д
									ioflag = true;
								}
							}
						}
					}
					else 
					{
						std::cout << "�װ�������" << std::endl; //֡������
					}
				}
				else
				{
					std::cout << "DataMap��"<< framemap <<"������" << std::endl;
					LostData qqq = { framemap , packagemap };
					auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
					if (iterat == RESEND.end())					//�����߳��ڲ����ڣ��Ͳ���
					{
						readwritemtx.lock();
						std::cout << framemap << "֡��" << packagemap << " ���ȵ���������׼���ְ��ط�" << std::endl;
						std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
						auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
						std::time_t timestampqqqq = tmp.count();
						LostData aaa = { framemap, packagemap, timestampqqqq };
						RESEND.push_back(aaa);
						readwritemtx.unlock();
					}
				}
			}
			////���ǵ��ж�12���룬34û�����ô����
			//����Ĳ��ǲ�����֡����ǰ���£��жϵ�ǰ֮֡ǰ30֡�ķְ��Ƿ����������������Ͳ��롣����
			if (framemap > 30)
			{
				for(int lostpackage = framemap - 1; lostpackage > framemap - 30; --lostpackage)
				{
					if (DataBuffMAP2[lostpackage] != -2 && DataBuffMAP2[lostpackage] != -1)//��һ��������������
					{
						//std::cout << fenbaoNote << "֡�� ��δ��ȷ������ڴ���" << std::endl;
						int it = DataBuffMAP2[lostpackage];
						if (it == -2)
						{
							return 100;
						}
						int DataBuffitlen = DataBuff[it].size();
						if (DataBuffitlen > 1)
						{
							for (int i = 1; i < DataBuffitlen; ++i)//ѭ����ʼ������ж��ٸ���û�յ�
							{
								if (DataBuff[it][i] == nullptr)
								{
									LostData qqq = { lostpackage , i+1 };
									auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
									if (iterat == RESEND.end())					//�����߳��ڲ����ڣ��Ͳ���
									{
										readwritemtx.lock();
										std::cout << lostpackage << "֡��" << i + 1 << " ��ĩβδ��ȷ����Ѵ���" << std::endl;
										std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
										auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
										std::time_t timestampqqqq = tmp.count();
										LostData aaa = { lostpackage, i + 1, timestampqqqq };
										RESEND.push_back(aaa);
										readwritemtx.unlock();
									}
								}
							}
						}

					}
				}
			}
			
		}
		
		else
		{
			std::cout << "Decode head info failure" << std::endl;
			return 400;
		}
		return 100;
	}
	else
	{
		std::cout << "Decode head info failure" << std::endl;
		return 400;	
	}
	return 500;
						
}

void Client3::UDPThreadprocess()
{
	std::cout << "UDP�߳̿�����" << std::endl;

	Recv_UDP_Data();
	std::cout << "UDP�����߳̽�����" << std::endl;

	DeleteAllLastData();
}

void Client3::IOThreadWriteFile()
{
	std::cout<<"IO�߳̿���" << std::endl;
	
	IOWriteFile();

	std::cout<<"IO�߳̽���" << std::endl;
}
void Client3::IOWriteFile()
{
	fsfile.open("videotest3_2_9.h264", std::ios::app | std::ios::binary);

	bool flag = true;
	do{
		flag = true;
		//printf("io����1\n");
		while (! ioflag )//����,��ѭ��////////ioһֱ������֡���򵽴�||io�������Ƿ�Ϊ��||��־λΪfalse
		{
			//udp֪ͨ��flag��Ϊtrue�����Ǵ�ʱio����д�ļ���д���ˣ���Ϊfalse������udp�����ٴ�ȥ֪ͨtrue
			if (maxrecvfram == FrameCount && IODATABUFF.size() == 0)
			{
				break;
			}
		}
		//printf("io����2\n");

		for (int i = blockedFrameNumber, num = DATABUFFmax; num >= 0 && i < waitwriteframe.size(); ++i, --num)//io��ȡ������������
		{
			if (waitwriteframe[i])
			{
				int mapiter = DataBuffMAP2[i];//�ҵ�֡�Ŷ�Ӧ�Ļ������±�
				
				if (DataBuffMAP2[i] != -2)
				{
					DataBuffMAP2[i] = -2;//֡�Ѿ�����������Ϊ-2
					std::vector<char*> temp;
					if (mapiter == -1)//��û�����ļ�ȡ�����ͱ������ˣ�map���ݣ����£�itָ�����
					{
						exit(-1);
					}
					temp.resize((DataBuff[mapiter]).size());
					for (int j = 0; j < (DataBuff[mapiter]).size(); ++j)
					{
						temp[j] = DataBuff[mapiter][j];//ָ�뻻��
						DataBuff[mapiter][j] = nullptr;
					}
					IODATABUFF.insert(std::pair<long, std::vector<char*>>(i, temp));
					DataBuffFlag[mapiter] = true;//�ɸ�д					
				}
			}
		}
		//д�ļ�
		for (std::map<long, std::vector<char*>>::iterator it = IODATABUFF.begin(); it != IODATABUFF.end(); )
		{
			if (blockedFrameNumber == it->first)//���򵽴�
			{
				
				auto oneit = oneBuffLenght.find(it->first);
				//std::cout <<it->first<<"���ȣ�" << it->second.size() << std::endl;
				for (int i = 0; i < it->second.size(); ++i)//дһ֡
				{
					writeFile(it->second[i], oneit->second[i]);//Ҫд��ĳ���
					delete[] it->second[i];
				}	
				filelengthmtx.lock();
				oneBuffLenght.erase(it->first);
				filelengthmtx.unlock();
				IODATABUFF.erase(it++);
				++blockedFrameNumber;
			}
			else
			{
				//��¼ȱʧ��֡�ţ��´δ�����д
				std::cout << "io������:" << blockedFrameNumber << std::endl;
				break;
			}
		}
		//��ѯһ��databuff�鿴���������Ƿ������ݣ��Ƿ��ǿɸ�дλ��
		for (int i = 0; i < DATABUFFmax; ++i)
		{
			if (DataBuffFlag[i] == false)//��дλΪfalse��˵������֡�ڻ�����
			{
				flag = false;
				break;
			}
		}
		if (RESEND.size() != 0)//����������Ϊ�գ�˵������֡�ڱ�����
		{
			flag = false;
			//break;
			//std::cout << "IOresendssssss" << std::endl;
		}
		if (IODATABUFF.size() != 0)
		{
			flag = false;
		}
		
		if (maxrecvfram == FrameCount && flag)
		{
			break;
		}

		ioflag = false;
		//printf("IO�ͷ���\n");
	} while (true);
	fsfile.close();
	fsfile.clear();
}
void Client3::UDPMonitoringData()
{
	//����ж�������ȱ֡? 
	//���ýṹ��,�洢�������ݵ�֡��,�Ͱ���.����Ƿ���Ԥ�����յ�.������յ���֡������,��ɾ��,��������udp�߳��н��� 
	std::wcout<<"�����߳̿�����" << std::endl;
	bool flag = true;
	do{
		flag = true;
		Sleep(5);//˯��5����
		int num = RESEND.size();

		readwritemtx.lock();//֮��io�ٴ����������������ٴ��ط�
		
		for (auto iter = RESEND.begin(); iter != RESEND.end(); ++iter)
		{
			std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
			auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
			std::time_t timestampqqqq = tmp.count();
			if (timestampqqqq - iter->chuo >= 100)//,ʱ����������ط���֡
			{
				if (iter->package == 1)//�ط���֡
				{
					//����ʱ���
					std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
					auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
					std::time_t timestamp = tmp.count();
					iter->chuo = timestamp;
					std::cout << "��֡�ط�" << iter->frame << std::endl;
					makeupIndex = LongToS(iter->frame);
					TCP_Makeup(0);
				}
				else//�ط��ְ�//�����ط�
				{
					std::string resendPackageD = "";
					std::cout << "�����ط�֡��:" << iter->frame << " ����:" << iter->package;
					makeupIndex = LongToS(iter->frame);
					makeupPackageIndex = std::to_string(iter->package);
					TCP_Makeup(1);//�ְ��ط�
					//����ʱ���
					std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
					auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
					std::time_t timestamp = tmp.count();
					iter->chuo = timestamp;
				}
			}
			else
			{
				break;
			}
			
		}
		readwritemtx.unlock();	
		//��ѯһ��databuff�鿴���������Ƿ������ݣ��Ƿ��ǿɸ�дλ��
		for (int i = 0; i < DATABUFFmax; ++i)
		{
			if (DataBuffFlag[i] == false)//��дλΪfalse��˵������֡�ڻ�����
			{
				flag = false;//std::cout << "����qqqqqs" << std::endl;
				break;
			}
		}
		if (RESEND.size() != 0)//����������Ϊ�գ�˵������֡�ڱ�����
		{
			flag = false;
		}

		if (maxrecvfram == FrameCount && flag)
		{
			break;
		}

	} while (true);
	std::cout << "�����߳̽�����" << std::endl;
	//�ط�����.��������,�͵��ж�
}

void  Client3::CreateThread()
{
	std::thread Thread_IOWriteProcess(std::bind(&Client3::IOThreadWriteFile, this));//����IOд�ļ��߳�
	std::thread Thread_UDPProcess(std::bind(&Client3::UDPThreadprocess, this));		//����udp���մ����߳�
	std::thread Thread_UDPMonitoringData(std::bind(&Client3::UDPMonitoringData, this));//����udp���ݼ����߳�

	Thread_UDPProcess.join();
	Thread_UDPMonitoringData.join();
	Thread_IOWriteProcess.join();
	
	return ;
}

void Client3::Process()
{
	Client_Create_TCP_Connet();
	TCP_Describe();
	GetUdpPort();

	this->startIndex = "1";
	TCP_Play();

	CreateThread();//�����߳�

	closesocket(sclient);
	WSACleanup();
}

int main()
{
	Client3 monster;
	monster.Process();

	return 0;
}


void Client3::StringtoSplit(const std::string& str, const char split, std::vector<std::string>& res)
{
	if (str == "")		return;
	//���ַ���ĩβҲ����ָ����������ȡ���һ��
	std::string strs = str + split;
	size_t pos = strs.find(split);

	// ���Ҳ����������ַ��������������� npos
	while (pos != strs.npos)
	{
		std::string temp = strs.substr(0, pos);
		res.push_back(temp);
		//ȥ���ѷָ���ַ���,��ʣ�µ��ַ����н��зָ�
		strs = strs.substr(pos + 1, strs.size());
		pos = strs.find(split);
	}
}
bool Client3::checkComplete(std::vector<char*>& temps)
{
	for (int i = 0; i < temps.size(); i++)
	{
		if (nullptr == temps[i])
		{	
			//std::cout << "֡δ��" << std::endl;
			return  false;
		}
	}
	return true;
}

std::string Client3::LongToS(long lon)
{
	std::ostringstream os;
	os << lon;
	std::string result;
	std::istringstream is(os.str());
	is >> result;

	return result;
}
int Client3::packageCount(long FrameLength)
{
	int count = 0;
	if (FrameLength > 0)
	{
		count = (int)(FrameLength / MAX_NETWORK_LENGTH);
		if (FrameLength % MAX_NETWORK_LENGTH != 0)
		{
			count = count + 1;
		}
	}
	return count;
}

int Client3::settimeout()
{
	int TimeOut = 250;//���ý��ճ�ʱ
	if (::setsockopt(UDPSocketfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&TimeOut, sizeof(TimeOut)) == SOCKET_ERROR)
	{
		return 0;
	}
	return 0;
}
void Client3::openWriteFile()
{
	fsfile.open(".\\videotest_3_2.h264", std::ios::app | std::ios::binary);
}
void Client3::writeFile(char* buff, int len)
{
	if (fsfile.fail())
	{
		std::cout << "���ļ�ʧ��" << std::endl;
	}
	else
	{
		fsfile.write(buff, len);
	}

}

void Client3::closeWriteFile()
{
	fsfile.close();
	fsfile.clear();
}

void Client3::DeleteAllLastData()
{

}

void Client3::initFrameintegrity()
{
	frameDataintegrity = new char[FrameCount + 1];
	memset(frameDataintegrity, 0, FrameCount + 1);
	//cout<<"֡���鳤��Ϊ"<< FrameCount<< endl;
}

void Client3::AddFrameintegrity(long framenumber)
{
	frameDataintegrity[framenumber] = '1';
}

bool Client3::JudgeFrameintegrity(long framenumber)
{
	if ('1' != frameDataintegrity[framenumber])
		return true;
	return false;
}

//int pause = 0;
//for (int bufflen = 0; bufflen < DATABUFFmax; ++bufflen)
//{
//	if (!DataBuffFlag[bufflen])
//	{
//		++pause;
//	}
//}
//if (pause > (DATABUFFmax * 0.6))//����������ж�һ�»������Ƿ�����//һ���߼��������������Ͳ�����
//{
//	std::cout << "���ջ���������" << std::endl;
//	std::cout << "δ��֡��:" << firstframe << std::endl;
//	TCP_Pause();
//	//��¼��ǰ��֡�ţ����ں����ٷ���
//}
//else if (pause < (DATABUFFmax * 0.2))
//{
//	std::cout << "��������" << firstframe << std::endl;

//	this->startIndex = LongToS(firstframe + 1);
//	
//	TCP_Play();
//}


//{
		//	long framemap = stol(strList[0]);
		//	int  packagemap = stoi(strList[1]);
		//	
		//	LostData qqq = { framemap , packagemap };		//�жϽ��հ��Ƿ��Ƕ�ʧ���ش�

		//	int check = DataBuffMAP2[framemap];
		//	auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
		//	if (iterat == RESEND.end() && framemap < CurrentFrameNumber && check == -1)//�������������ڣ���֡�űȵ�ǰС
		//	{
		//		std::cout << "��"<< framemap<<"��" << packagemap << "�ٴ��ط�������" << std::endl;
		//		return 100;
		//	}
		//	if (check != -1)//if�װ������������else����
		//	{
		//		if (check == -2)
		//		{
		//			std::cout << "�ð���֡����" << std::endl;
		//			
		//			return 100;
		//		}
		//		if (DataBuff[check][0] != nullptr)
		//		{
		//			//��ȡ�ְ�����
		//			if (DataBuff[check][packagemap - 1] == nullptr)
		//			{
		//				LostData qqq = { framemap, packagemap };//�жϽ��հ��Ƿ��Ƕ�ʧ��//��⵽��������Ƿ����ش���
		//				auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
		//				if (iterat != RESEND.end())
		//				{
		//					readwritemtx.lock();
		//					RESEND.erase(iterat);
		//					readwritemtx.unlock();
		//				}
		//				std::cout << "�ְ�����֡��" << framemap << "  �ְ����ݰ���:" << packagemap << std::endl;
		//				//������֡��ʱ�򣬸�����һ֡��֡�ţ�����databuff����һ֡�İ������Ȼ�����ж��Ƿ���Ҫ��¼
		//				//��һ֡û���ҵ���˵��������
		//				if (fenbaoNote != framemap)//�°�����
		//				{std::cout << "�°�����"<<std::endl;
		//					if (DataBuffMAP2[fenbaoNote] != -2)//��һ��������
		//					{
		//						std::cout << fenbaoNote << "֡�� ��δ��ȷ������ڴ���" << std::endl;
		//						for (int i = 1; i < DataBuff[DataBuffMAP2[fenbaoNote]].size(); ++i)//ѭ����ʼ������ж��ٸ���û�յ�
		//						{
		//							if (DataBuff[DataBuffMAP2[fenbaoNote]][i] == nullptr)//Ϊnullptr������û��������
		//							{
		//								readwritemtx.lock();
		//								std::cout << fenbaoNote << "֡��"<< i + 1 <<" ��δ��ȷ����Ѵ���" << std::endl;
		//								std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		//								auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
		//								std::time_t timestampqqqq = tmp.count();
		//								LostData aaa = { fenbaoNote, i + 1, timestampqqqq };
		//								RESEND.push_back(aaa);
		//								readwritemtx.unlock();
		//							}
		//						}
		//					}
		//				}
		//				DataBuff[check][packagemap - 1] = new char[1500];
		//				memset(DataBuff[check][packagemap - 1], 0, 1500);

		//				for (int i = headLength + 3; i <= nRes; i++)//���ַ�������Ƶ����.
		//				{
		//					DataBuff[check][packagemap - 1][i - headLength - 3] = Framebuff[i];
		//				}
		//				std::map<long, std::vector<int>>::iterator iter = oneBuffLenght.find(framemap);
		//				iter->second[packagemap - 1] = nRes - headLength - 3;//������ݳ���

		//				//��¼��ǰ֡��֡��
		//				fenbaoNote = framemap;

		//				if (checkComplete(DataBuff[DataBuffMAP2[framemap]]))//�������ˣ� ��֪ͨio
		//				{
		//					//֪ͨIO�߳�
		//					//DataBuffMAP2[framemap] = -2;//���֡����������Ϊ-2
		//					waitwriteframe[framemap] = true;//֪ͨIO�̸߳�֡��д
		//					ioflag = true;
		//				}
		//			}	
		//			else
		//			{
		//				std::cout << "�ð��Ѿ�����,��ע�⣡" << std::endl;
		//			}
		//		}
		//		else
		//		{
		//			std::cout << "�װ����ڣ��ְ��ȵ�" << std::endl;
		//		}
		//	}
		//	else
		//	{
		//		std::cout << "DataBuffMAP2��¼"<< framemap << "�����ڣ��������װ����ڣ�" <<  packagemap << std::endl;
		//		//return 400;
		//	}
		//}









	//if (CurrentFrameNumber != FrameCount)//��¼makeupindex�������ǰ��֡�ŵ���makeindex�����ŵ���makeupPackageIndex
		//{
		//	historyFrameNumber = CurrentFrameNumber;
		//	//��������play
		//	std::cout << "�������͵�֡�ţ�" << CurrentFrameNumber + 1 << std::endl;
		//	std::string contin = LongToS(CurrentFrameNumber + 1);
		//	this->startIndex = contin;
		//	TCP_Play();
		//}
		// 
		// 
		// 
//�������֡û���յ�


	//753�ĵڶ����������������Լ����������֡�����ˣ����ǣ���û��¼�ڶ�����
	//��ʱ���ְ��жϻ���ͣ����751


/*std::cout << "UDPDataBuffF  no empty" << std::endl;
				for (int j = 1; j < DataBuffMAP2.size(); ++j)
				{
					if (DataBuffMAP2[j] != -2)
					{
						std::cout << "��֡������" << j << std::endl;
					}
				}*/







				//if (fenbaoNote != framemap)
				//{	
				//	if (DataBuffMAP2[fenbaoNote] != -2 && DataBuffMAP2[fenbaoNote] != -1)//��һ��������������
				//	{
				//		//std::cout << fenbaoNote << "֡�� ��δ��ȷ������ڴ���" << std::endl;
				//		int it = DataBuffMAP2[fenbaoNote];
				//		int DataBuffitlen = DataBuff[it].size();
				//		if (DataBuffitlen > 1)
				//		{
				//			for (int i = 1; i < DataBuffitlen; ++i)//ѭ����ʼ������ж��ٸ���û�յ�
				//			{
				//				if (DataBuff[DataBuffMAP2[fenbaoNote]][i] == nullptr)//Ϊnullptr������û��������
				//				{
				//					
				//					
				//					//������֡��34��û�յ�����577��578
				//					
				//					
				//					
				//					LostData qqq = { framemap , packagemap };
				//					auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
				//					if (iterat == RESEND.end())//�����߳��ڲ����ڣ��Ͳ���
				//					{
				//						readwritemtx.lock();
				//						std::cout << fenbaoNote << "֡��" << i + 1 << " ��δ��ȷ����Ѵ���" << std::endl;
				//						std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
				//						auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
				//						std::time_t timestampqqqq = tmp.count();
				//						LostData aaa = { fenbaoNote, i + 1, timestampqqqq };
				//						RESEND.push_back(aaa);
				//						readwritemtx.unlock();
				//					}
				//				}
				//			}
				//		}

				//	}
				//}

				////��¼��ǰ֡��֡��
				//fenbaoNote = framemap;