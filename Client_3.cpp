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
		printf("套接字无效");
		return false;
	}

	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8172);
	serAddr.sin_addr.s_addr = inet_addr("8.134.86.82");
	if (connect(sclient, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{  //连接失败 
		printf("connect error !/连接失败");
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
		printf("errno = %d\n", WSAGetLastError());//小于0的返回SOCKET_ERROR值，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。
		return;
	}

	std::string s_recdata = recvTCPBuff;
	if (s_recdata.size() == 0)
	{
		printf("error, 接收数据为空");//接收为空，会导致substr截取错误，崩溃。
		return;
	}
	s_recdata = s_recdata.substr(4, s_recdata.length());
	Json::Reader reader;
	Json::Value value;

	std::string C_md,C_mdId,S_uccess, M_essage,R_esolution;
	int F_ramerate;
	long long  L_ength;
	long F_rameCount;
	if (reader.parse(s_recdata, value))//解析出json放到json中区
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
		//initFrameintegrity();//统计已经到达的帧号
		waitwriteframe.resize(FrameCount+1);//782帧的长度.
		std::fill(waitwriteframe.begin(), waitwriteframe.end(), false);

		DataBuffMAP2.resize(FrameCount + 1);//buff记录
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
		printf("errno = %d\n", WSAGetLastError());//小于0的返回SOCKET_ERROR值，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。
		return;
	}

	std:: string s_recdata = recvTCPBuff;
	if (s_recdata.size() == 0)
	{
		printf("未收到数据。。");
		return;
	}
	s_recdata = s_recdata.substr(4, s_recdata.length());
	Json::Reader reader;//json解析
	Json::Value value;//表示一个json格式的对象

	std::string C_md,C_mdId,S_uccess,M_essage,R_esolution;
	//int F_ramerate;
	//long long  L_ength;
	//unsigned int F_rameCount;
	if (reader.parse(s_recdata, value))//解析出json放到json中区
	{
		C_md = value["cmd"].asString();
		C_mdId = value["cmdId"].asString();
		S_uccess = value["success"].asString();
		std::cout << "playcmd输出：" <<"     " << C_md << "     " << C_mdId << "     " << S_uccess << std::endl;
	}
	
}

void Client3::TCP_Makeup(int flag)
{
	std::string ssssend;
	if (0 == flag)//重发整帧
	{
		std::string makeupSendBuff_0 = "{\"cmd\":\"makeup\",\"cmdId\":\"94bd332b233e497a87905bd4dea86800\",\"clientId\":\"A1B1\",\"resourceId\":1,\"makeupIndex\":" + makeupIndex + ",\"receivePort\":" + UDPport + "}";
		int len = makeupSendBuff_0.size();
		std::string length = std::to_string(len);
		ssssend = "0" + length + makeupSendBuff_0;
	}
	else if (1 == flag)//重发帧分包
	{
		std::string makeupSendBuff_1 = "{\"cmd\":\"makeup\",\"cmdId\":\"94bd332b233e497a87905bd4dea86800\",\"clientId\":\"A1B1\",\"resourceId\":1,\"makeupIndex\":" + makeupIndex + ",\"makeupPackageIndex\":[" + makeupPackageIndex + "],\"receivePort\":" + UDPport + "}";

		int len = makeupSendBuff_1.size();
		std::string lenght = std::to_string(len);
		ssssend = "0" + lenght + makeupSendBuff_1;
	}

	//设置字符串的长度
	const char* pausebuff = ssssend.c_str();
	int n = send(sclient, pausebuff, strlen(pausebuff), 0);

	memset(recvTCPBuff, 0, sizeof(recvTCPBuff));
	int retq = recv(sclient, recvTCPBuff, 1400, 0);

	if (retq <= 0)
	{
		printf("connet close...... ");
		printf("errno = %d\n", WSAGetLastError());//小于0的返回SOCKET_ERROR值，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。
		exit(-1);
		return;
	}

	std::string s_recdata = recvTCPBuff;
	if (s_recdata.size() == 0)
	{
		printf("未收到数据。。");
		return;
	}
	s_recdata = s_recdata.substr(4, s_recdata.length());
	Json::Reader reader;//json解析
	Json::Value value;//表示一个json格式的对象

	std::string C_md, C_mdId, S_uccess, M_essage, R_esolution;
	if (reader.parse(s_recdata, value))//解析出json放到json中区
	{
		C_md = value["cmd"].asString();
		C_mdId = value["cmdId"].asString();
		S_uccess = value["success"].asString();
		//std::cout << "makeup_cmd补发情况："<< S_uccess << std::endl;
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
		printf("errno = %d\n", WSAGetLastError());//小于0的返回SOCKET_ERROR值，并且可以通过调用 WSAGetLastError 来检索特定的错误代码。
		return;
	}

	std::string s_recdata = recvTCPBuff;
	if (s_recdata.size() == 0)
	{
		printf("未收到数据。。");
		return;
	}
	s_recdata = s_recdata.substr(4, s_recdata.length());
	Json::Reader reader;//json解析
	Json::Value value;//表示一个json格式的对象

	std::string  S_uccess, R_esolution;
	
	if (reader.parse(s_recdata, value))//解析出json放到json中区
	{
		S_uccess = value["success"].asString();
		std::cout << "TCP_Pause()输出：" <<  S_uccess << std::endl;
		
	}
}
bool Client3::GetUdpPort()
{
	char recvServerBuf[100];
	UDPSocketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (UDPSocketfd == INVALID_SOCKET)
	{
		printf("Client: Error at socket(): %ld\n", WSAGetLastError());// 输出错误信息
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
	settimeout();						//设置超时时间0.25秒
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
				std::cout << "连接关闭" << std::endl;
				return;
			}
			else if (nRes < 0 && 10060 == GetLastError())
			{
				std::cout << "recvfrom等待超时，跳过" << std::endl;
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
		//轮询一下databuff查看缓冲区内是否有数据（是否都是可复写位）
		int num = 0;
		
		
		//轮询一下resendbuff查看缓冲区内是否有数据
		if (RESEND.size() != 0)//监视容器不为空，说明还有帧在被监视
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
			if (DataBuffFlag[i] == false)//复写位为false，说明还有帧在缓冲区
			{
				++num;
				ioflag = true;
				flag = false;
				//break;
				
			}
		}
		std::cout <<"databuff的大小" << num << std::endl;
		std::cout << "resendbuff的大小" << RESEND.size() << std::endl;
		std::cout << "IODATABUFF的大小" << IODATABUFF.size() << std::endl;
		std::cout << "接收到的最大帧号：" << maxrecvfram << std::endl;
		if (num > 0 && num < 3 && RESEND.size() == 0 && IODATABUFF.size() == 0)
		{
			if (udpdatabuffnumber == 0)
			{
				int i = 1;
				for (; i < DataBuffMAP2.size(); ++i)//最后再检查一下databuff2中哪个帧号不为-2
				{
					if (DataBuffMAP2[i] != -2)
					{
						std::cout << "最后此帧未完整:" << i << std::endl;
					}
				}
				std::cout << "继续发送的帧号：" << i << std::endl;
				std::string contin = LongToS(i);
				this->startIndex = contin;
				TCP_Play();

			}
			--udpdatabuffnumber;
		}
		

		//if三个容器都为空，帧号不对，那就从对应帧开始发

		if (maxrecvfram != FrameCount && flag )
		{
			if (maxrecvfram != FrameCount && maxrecvfram > FrameCount/2)
			{
				std::cout << "继续发送的帧号：" << maxrecvfram + 1 << std::endl;
	   			std::string contin = LongToS(maxrecvfram + 1);
	   			this->startIndex = contin;
	   			TCP_Play();
			}
		}
		if (maxrecvfram == FrameCount && flag)
		{
			break;
		}
		
	} while (true);//当前帧号 != 总帧
	
	return;
}
int  Client3::splitHandle(char* Framebuff, long frame, int nRes)//UDP包的处理
{
	int headLength = stoi(std::string{ Framebuff[1], Framebuff[2] });//首部长度
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
		if (4 == num)//第一个包
		{
			long firstframe = stol(strList[0]);
			int firstpackagemap = stoi(strList[1]);
			long FrameLength = stol(strList[3]);
			
			LostData qqq = { firstframe , firstpackagemap };		//判断接收包是否是丢失包重传
			auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
			if (iterat == RESEND.end() && firstframe <= CurrentFrameNumber)//丢包容器不存在，且帧号比当前小
			{
				std::cout <<"该帧再次重发：" << firstframe<<"丢弃" << std::endl;
				return 100;
			}
			if (iterat != RESEND.end())//收到重发的帧
			{
				readwritemtx.lock();	
				std::cout<<"收到重发的帧："<< firstframe<<"一共包：" << packageCount(FrameLength) << std::endl;
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
			//获取内容数据
			res[firstpackagemap - 1] = new char[1500];
			memset(res[firstpackagemap - 1], 1, 1500);

			for (int i = headLength + 3; i <= nRes; ++i)//逐字符拷贝
			{
				res[firstpackagemap - 1][i - headLength - 3] = Framebuff[i];
			}
			bool f = true;
			for (int arr = 0; arr < DATABUFFmax; ++arr)//寻找可复写的位
			{
				if (DataBuffFlag[arr])
				{
					////std::cout << "接收帧号:" << firstframe << "   接收包号:" << firstpackagemap << " 共：" << packageCount(FrameLength) << "包" << std::endl;
					//if (DataBuffMAP2[firstframe] == -2)
					//{
					//	printf("重新发现完整帧重发\n");
					//	return 100;
					//}

					//if (DataBuffMAP2[firstframe] != -1)//收到重发的整帧,与之前的重复了
					//{
					//	for (int j = 0; j < DataBuff[DataBuffMAP2[firstframe]].size(); ++j)
					//	{
					//		delete[] DataBuff[DataBuffMAP2[firstframe]][j];
					//		DataBuff[DataBuffMAP2[firstframe]][j] = nullptr;
					//	}
					//	DataBuffFlag[DataBuffMAP2[firstframe]] = true;
					//}
					
					DataBuff[arr] = res;
					DataBuffMAP2[firstframe] = arr;//vector存储帧号，以及对应的下标位置。
					vecbuflen[firstpackagemap - 1] = nRes - headLength - 3;
					filelengthmtx.lock();
					oneBuffLenght.insert(std::pair<long, std::vector<int>>(firstframe, vecbuflen));
					filelengthmtx.unlock();

					if (firstframe != shouldFrameNumber)//帧没按序到达
					{
						readwritemtx.lock();
						int iii = shouldFrameNumber - 1;//检查应该存入帧的前一个帧是否满了
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
						for (long rr = shouldFrameNumber; rr < firstframe; ++rr)//观察resend和databuff中是否存在当前帧，存在则不存入
						{
							std::cout << rr << "帧未正确到达！已存入（udp第一包）" << std::endl;
							std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
							auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
							std::time_t timestampqqqq = tmp.count();
							//成片的帧没收到。
							LostData aaa = { rr, 1, timestampqqqq };
							RESEND.push_back(aaa);
						}
						readwritemtx.unlock();

					}//把判定条件放到当前帧，毕竟，之前缺失的帧，都已记录，，缺失的帧收到之后也比较一下，小于了，就不赋值
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
				std::cout << "缓冲区已满" << std::endl;
				//std::cout << "最后一帧号：" << firstframe << std::endl;
				//std::cout << rr << "帧未正确到达！已存入（udp第一包）" << std::endl;
				std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
				auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
				std::time_t timestampqqqq = tmp.count();
				//成片的帧没收到。
				LostData aaa = { firstframe, 1, timestampqqqq };
				RESEND.push_back(aaa);
				readwritemtx.unlock();
				//exit(-1);
			}

			CurrentFrameNumber = firstframe;
			if (maxrecvfram <= CurrentFrameNumber)
				maxrecvfram = CurrentFrameNumber;
			
			if (checkComplete(DataBuff[DataBuffMAP2[firstframe]]))//判断是否帧是否已满，就通知io线程
			{
				waitwriteframe[firstframe] = true;//通知IO线程该帧可写
				ioflag = true;
			}
			if (firstframe == FrameCount)
			{
				std::cout << "切片中，接受到的最大的帧号：" << CurrentFrameNumber << std::endl;
				CurrentFrameNumber = firstframe;//记录接收到最后一帧
			}
		}
		else if (1 == num)//其他分包
		{
			long framemap = stol(strList[0]);
			int  packagemap = stoi(strList[1]);
			
			
			//唯二的两个问题。丢包，乱序，重发
			//同一帧
			//不同帧

			//按顺序接收同一帧的分包
			if (CurrentFrameNumber == framemap)
			{
				int check = DataBuffMAP2[framemap];
				//（同一帧乱序）就判断包号一致不，不一致，就存入监视线程
				if (check != -2)
				{
					if (DataBuff[check][0] != nullptr)
					{
						for (int i = 1; i < DataBuff[check].size(); ++i)
						{
							if (DataBuff[check][i] == nullptr)
							{	
								if (packagemap - 1 == i)//正确接收包
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
										waitwriteframe[framemap] = true;//通知IO线程该帧可写
										ioflag = true;
									}
									break;
								}
								else if (packagemap - 1 != i)//包乱序。记录应收的包
								{
									readwritemtx.lock();
									std::cout << framemap << "帧的" << i + 1 << " 包未正确到达！已存入" << std::endl;
									std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
									auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
									std::time_t timestampqqqq = tmp.count();
									LostData aaa = { framemap, i + 1, timestampqqqq };
									RESEND.push_back(aaa);
									readwritemtx.unlock();

									//保存当前接收数据
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
						std::cout << "首包不存在" << std::endl;
					}
				}
				else
				{
					std::cout << framemap << "帧已满,(分包中)" << std::endl;
					//查看监视缓冲区中有无，有则删除
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
			//和当前帧号不一致，可能是补发的，也可能是乱帧序的包。
			if (CurrentFrameNumber != framemap)
			{
				LostData qqq = { framemap , packagemap };
				int check = DataBuffMAP2[framemap];
				auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
				if (iterat == RESEND.end() && framemap < CurrentFrameNumber && check == -2)
				{
					std::cout << "该" << framemap << "包" << packagemap << "再次重发：丢弃" << std::endl;
					return 100;
				}
				if (check != -1)
				{
					if (check == -2)//帧满但是接收到了重发的
					{
						std::cout << "该包的帧" << framemap << "已满" << std::endl;
						if (iterat != RESEND.end())
						{
							readwritemtx.lock();
							RESEND.erase(iterat);
							readwritemtx.unlock();
						}					
						return 100;
					}

					if (DataBuff[check][0] != nullptr)//首包存在，正确
					{
						if (DataBuff[check][packagemap - 1] == nullptr)//分包对应的位置为空，正确
						{
							if (iterat != RESEND.end())//在监视线程内找到对应分包记录，正确
							{
								readwritemtx.lock();
								RESEND.erase(iterat);
								readwritemtx.unlock();
								//获取分包数据存入数据
								DataBuff[check][packagemap - 1] = new char[1500];
								memset(DataBuff[check][packagemap - 1], 0, 1500);

								for (int i = headLength + 3; i <= nRes; i++)//逐字符拷贝视频数据.
								{
									DataBuff[check][packagemap - 1][i - headLength - 3] = Framebuff[i];
								}
								filelengthmtx.lock();
								std::map<long, std::vector<int>>::iterator iter = oneBuffLenght.find(framemap);		
								iter->second[packagemap - 1] = nRes - headLength - 3;//添加数据长度
								filelengthmtx.unlock();
								if (checkComplete(DataBuff[DataBuffMAP2[framemap]]))//包完整了， 就通知io
								{
									std::cout << framemap <<"帧" << packagemap << "可写" << std::endl;
									waitwriteframe[framemap] = true;//通知IO线程该帧可写
									ioflag = true;
								}
							}
						}
					}
					else 
					{
						std::cout << "首包不存在" << std::endl; //帧包乱序
					}
				}
				else
				{
					std::cout << "DataMap中"<< framemap <<"不存在" << std::endl;
					LostData qqq = { framemap , packagemap };
					auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
					if (iterat == RESEND.end())					//接收线程内不存在，就插入
					{
						readwritemtx.lock();
						std::cout << framemap << "帧的" << packagemap << " 包先到，丢弃！准备分包重发" << std::endl;
						std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
						auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
						std::time_t timestampqqqq = tmp.count();
						LostData aaa = { framemap, packagemap, timestampqqqq };
						RESEND.push_back(aaa);
						readwritemtx.unlock();
					}
				}
			}
			////还是得判断12存入，34没存会怎么处理。
			//到达的不是补发的帧包的前提下，判断当前帧之前30帧的分包是否完整，不完整，就插入。完整
			if (framemap > 30)
			{
				for(int lostpackage = framemap - 1; lostpackage > framemap - 30; --lostpackage)
				{
					if (DataBuffMAP2[lostpackage] != -2 && DataBuffMAP2[lostpackage] != -1)//上一包不完整，继续
					{
						//std::cout << fenbaoNote << "帧的 包未正确到达！正在存入" << std::endl;
						int it = DataBuffMAP2[lostpackage];
						if (it == -2)
						{
							return 100;
						}
						int DataBuffitlen = DataBuff[it].size();
						if (DataBuffitlen > 1)
						{
							for (int i = 1; i < DataBuffitlen; ++i)//循环开始，检测有多少个包没收到
							{
								if (DataBuff[it][i] == nullptr)
								{
									LostData qqq = { lostpackage , i+1 };
									auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
									if (iterat == RESEND.end())					//接收线程内不存在，就插入
									{
										readwritemtx.lock();
										std::cout << lostpackage << "帧的" << i + 1 << " 包末尾未正确到达！已存入" << std::endl;
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
	std::cout << "UDP线程开启！" << std::endl;

	Recv_UDP_Data();
	std::cout << "UDP接收线程结束！" << std::endl;

	DeleteAllLastData();
}

void Client3::IOThreadWriteFile()
{
	std::cout<<"IO线程开启" << std::endl;
	
	IOWriteFile();

	std::cout<<"IO线程结束" << std::endl;
}
void Client3::IOWriteFile()
{
	fsfile.open("videotest3_2_9.h264", std::ios::app | std::ios::binary);

	bool flag = true;
	do{
		flag = true;
		//printf("io运行1\n");
		while (! ioflag )//阻塞,死循环////////io一直阻塞在帧按序到达||io缓冲区是否为空||标志位为false
		{
			//udp通知把flag变为true，但是此时io正在写文件，写完了，置为false。但是udp不会再次去通知true
			if (maxrecvfram == FrameCount && IODATABUFF.size() == 0)
			{
				break;
			}
		}
		//printf("io运行2\n");

		for (int i = blockedFrameNumber, num = DATABUFFmax; num >= 0 && i < waitwriteframe.size(); ++i, --num)//io读取缓冲区的数据
		{
			if (waitwriteframe[i])
			{
				int mapiter = DataBuffMAP2[i];//找到帧号对应的缓冲区下标
				
				if (DataBuffMAP2[i] != -2)
				{
					DataBuffMAP2[i] = -2;//帧已经完整，就置为-2
					std::vector<char*> temp;
					if (mapiter == -1)//还没有来的及取出，就被覆盖了？map扩容，导致，it指向错误。
					{
						exit(-1);
					}
					temp.resize((DataBuff[mapiter]).size());
					for (int j = 0; j < (DataBuff[mapiter]).size(); ++j)
					{
						temp[j] = DataBuff[mapiter][j];//指针换绑
						DataBuff[mapiter][j] = nullptr;
					}
					IODATABUFF.insert(std::pair<long, std::vector<char*>>(i, temp));
					DataBuffFlag[mapiter] = true;//可复写					
				}
			}
		}
		//写文件
		for (std::map<long, std::vector<char*>>::iterator it = IODATABUFF.begin(); it != IODATABUFF.end(); )
		{
			if (blockedFrameNumber == it->first)//按序到达
			{
				
				auto oneit = oneBuffLenght.find(it->first);
				//std::cout <<it->first<<"长度：" << it->second.size() << std::endl;
				for (int i = 0; i < it->second.size(); ++i)//写一帧
				{
					writeFile(it->second[i], oneit->second[i]);//要写入的长度
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
				//记录缺失的帧号，下次从这里写
				std::cout << "io阻塞在:" << blockedFrameNumber << std::endl;
				break;
			}
		}
		//轮询一下databuff查看缓冲区内是否有数据（是否都是可复写位）
		for (int i = 0; i < DATABUFFmax; ++i)
		{
			if (DataBuffFlag[i] == false)//复写位为false，说明还有帧在缓冲区
			{
				flag = false;
				break;
			}
		}
		if (RESEND.size() != 0)//监视容器不为空，说明还有帧在被监视
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
		//printf("IO释放锁\n");
	} while (true);
	fsfile.close();
	fsfile.clear();
}
void Client3::UDPMonitoringData()
{
	//如何判定丢包和缺帧? 
	//设置结构体,存储接收数据的帧号,和包号.检查是否按照预定接收到.如果接收到的帧号完整,就删除,都可以在udp线程中进行 
	std::wcout<<"监视线程开启！" << std::endl;
	bool flag = true;
	do{
		flag = true;
		Sleep(5);//睡眠5毫秒
		int num = RESEND.size();

		readwritemtx.lock();//之后io再次抢到了锁，导致再次重发
		
		for (auto iter = RESEND.begin(); iter != RESEND.end(); ++iter)
		{
			std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
			auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
			std::time_t timestampqqqq = tmp.count();
			if (timestampqqqq - iter->chuo >= 100)//,时间戳超过，重发整帧
			{
				if (iter->package == 1)//重发整帧
				{
					//更新时间戳
					std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
					auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
					std::time_t timestamp = tmp.count();
					iter->chuo = timestamp;
					std::cout << "整帧重发" << iter->frame << std::endl;
					makeupIndex = LongToS(iter->frame);
					TCP_Makeup(0);
				}
				else//重发分包//丢包重发
				{
					std::string resendPackageD = "";
					std::cout << "正在重发帧号:" << iter->frame << " 包号:" << iter->package;
					makeupIndex = LongToS(iter->frame);
					makeupPackageIndex = std::to_string(iter->package);
					TCP_Makeup(1);//分包重发
					//更新时间戳
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
		//轮询一下databuff查看缓冲区内是否有数据（是否都是可复写位）
		for (int i = 0; i < DATABUFFmax; ++i)
		{
			if (DataBuffFlag[i] == false)//复写位为false，说明还有帧在缓冲区
			{
				flag = false;//std::cout << "监视qqqqqs" << std::endl;
				break;
			}
		}
		if (RESEND.size() != 0)//监视容器不为空，说明还有帧在被监视
		{
			flag = false;
		}

		if (maxrecvfram == FrameCount && flag)
		{
			break;
		}

	} while (true);
	std::cout << "监视线程结束！" << std::endl;
	//重发上限.超过上线,就得判断
}

void  Client3::CreateThread()
{
	std::thread Thread_IOWriteProcess(std::bind(&Client3::IOThreadWriteFile, this));//开启IO写文件线程
	std::thread Thread_UDPProcess(std::bind(&Client3::UDPThreadprocess, this));		//开启udp接收处理线程
	std::thread Thread_UDPMonitoringData(std::bind(&Client3::UDPMonitoringData, this));//开启udp数据监视线程

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

	CreateThread();//开启线程

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
	//在字符串末尾也加入分隔符，方便截取最后一段
	std::string strs = str + split;
	size_t pos = strs.find(split);

	// 若找不到内容则字符串搜索函数返回 npos
	while (pos != strs.npos)
	{
		std::string temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
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
			//std::cout << "帧未满" << std::endl;
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
	int TimeOut = 250;//设置接收超时
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
		std::cout << "打开文件失败" << std::endl;
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
	//cout<<"帧数组长度为"<< FrameCount<< endl;
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
//if (pause > (DATABUFFmax * 0.6))//在这里可以判断一下缓冲区是否满了//一般逻辑正常，缓冲区就不会满
//{
//	std::cout << "接收缓冲区已满" << std::endl;
//	std::cout << "未插帧号:" << firstframe << std::endl;
//	TCP_Pause();
//	//记录当前的帧号，便于后续再发。
//}
//else if (pause < (DATABUFFmax * 0.2))
//{
//	std::cout << "继续发送" << firstframe << std::endl;

//	this->startIndex = LongToS(firstframe + 1);
//	
//	TCP_Play();
//}


//{
		//	long framemap = stol(strList[0]);
		//	int  packagemap = stoi(strList[1]);
		//	
		//	LostData qqq = { framemap , packagemap };		//判断接收包是否是丢失包重传

		//	int check = DataBuffMAP2[framemap];
		//	auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
		//	if (iterat == RESEND.end() && framemap < CurrentFrameNumber && check == -1)//丢包容器不存在，且帧号比当前小
		//	{
		//		std::cout << "该"<< framemap<<"包" << packagemap << "再次重发：丢弃" << std::endl;
		//		return 100;
		//	}
		//	if (check != -1)//if首包存在则继续，else丢弃
		//	{
		//		if (check == -2)
		//		{
		//			std::cout << "该包的帧已满" << std::endl;
		//			
		//			return 100;
		//		}
		//		if (DataBuff[check][0] != nullptr)
		//		{
		//			//获取分包数据
		//			if (DataBuff[check][packagemap - 1] == nullptr)
		//			{
		//				LostData qqq = { framemap, packagemap };//判断接收包是否是丢失包//检测到达的数据是否是重传的
		//				auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
		//				if (iterat != RESEND.end())
		//				{
		//					readwritemtx.lock();
		//					RESEND.erase(iterat);
		//					readwritemtx.unlock();
		//				}
		//				std::cout << "分包数据帧号" << framemap << "  分包数据包号:" << packagemap << std::endl;
		//				//到达新帧的时候，根据上一帧的帧号，搜索databuff中上一帧的包情况，然后再判断是否需要记录
		//				//上一帧没有找到就说明完整了
		//				if (fenbaoNote != framemap)//新包到了
		//				{std::cout << "新包到了"<<std::endl;
		//					if (DataBuffMAP2[fenbaoNote] != -2)//上一包不完整
		//					{
		//						std::cout << fenbaoNote << "帧的 包未正确到达！正在存入" << std::endl;
		//						for (int i = 1; i < DataBuff[DataBuffMAP2[fenbaoNote]].size(); ++i)//循环开始，检测有多少个包没收到
		//						{
		//							if (DataBuff[DataBuffMAP2[fenbaoNote]][i] == nullptr)//为nullptr，就是没到，插入
		//							{
		//								readwritemtx.lock();
		//								std::cout << fenbaoNote << "帧的"<< i + 1 <<" 包未正确到达！已存入" << std::endl;
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

		//				for (int i = headLength + 3; i <= nRes; i++)//逐字符拷贝视频数据.
		//				{
		//					DataBuff[check][packagemap - 1][i - headLength - 3] = Framebuff[i];
		//				}
		//				std::map<long, std::vector<int>>::iterator iter = oneBuffLenght.find(framemap);
		//				iter->second[packagemap - 1] = nRes - headLength - 3;//添加数据长度

		//				//记录当前帧的帧号
		//				fenbaoNote = framemap;

		//				if (checkComplete(DataBuff[DataBuffMAP2[framemap]]))//包完整了， 就通知io
		//				{
		//					//通知IO线程
		//					//DataBuffMAP2[framemap] = -2;//如果帧完整，就置为-2
		//					waitwriteframe[framemap] = true;//通知IO线程该帧可写
		//					ioflag = true;
		//				}
		//			}	
		//			else
		//			{
		//				std::cout << "该包已经存在,请注意！" << std::endl;
		//			}
		//		}
		//		else
		//		{
		//			std::cout << "首包不在，分包先到" << std::endl;
		//		}
		//	}
		//	else
		//	{
		//		std::cout << "DataBuffMAP2记录"<< framemap << "不存在（可能是首包不在）" <<  packagemap << std::endl;
		//		//return 400;
		//	}
		//}









	//if (CurrentFrameNumber != FrameCount)//记录makeupindex，如果当前的帧号等于makeindex，包号等于makeupPackageIndex
		//{
		//	historyFrameNumber = CurrentFrameNumber;
		//	//继续申请play
		//	std::cout << "继续发送的帧号：" << CurrentFrameNumber + 1 << std::endl;
		//	std::string contin = LongToS(CurrentFrameNumber + 1);
		//	this->startIndex = contin;
		//	TCP_Play();
		//}
		// 
		// 
		// 
//最后俩个帧没有收到


	//753的第二个包。第三个包以及后面的三个帧都丢了，但是，并没记录第二第三
	//此时，分包判断还在停留在751


/*std::cout << "UDPDataBuffF  no empty" << std::endl;
				for (int j = 1; j < DataBuffMAP2.size(); ++j)
				{
					if (DataBuffMAP2[j] != -2)
					{
						std::cout << "此帧不满：" << j << std::endl;
					}
				}*/







				//if (fenbaoNote != framemap)
				//{	
				//	if (DataBuffMAP2[fenbaoNote] != -2 && DataBuffMAP2[fenbaoNote] != -1)//上一包不完整，继续
				//	{
				//		//std::cout << fenbaoNote << "帧的 包未正确到达！正在存入" << std::endl;
				//		int it = DataBuffMAP2[fenbaoNote];
				//		int DataBuffitlen = DataBuff[it].size();
				//		if (DataBuffitlen > 1)
				//		{
				//			for (int i = 1; i < DataBuffitlen; ++i)//循环开始，检测有多少个包没收到
				//			{
				//				if (DataBuff[DataBuffMAP2[fenbaoNote]][i] == nullptr)//为nullptr，就是没到，插入
				//				{
				//					
				//					
				//					//有两个帧的34包没收到。。577、578
				//					
				//					
				//					
				//					LostData qqq = { framemap , packagemap };
				//					auto iterat = std::find(RESEND.begin(), RESEND.end(), qqq);
				//					if (iterat == RESEND.end())//接收线程内不存在，就插入
				//					{
				//						readwritemtx.lock();
				//						std::cout << fenbaoNote << "帧的" << i + 1 << " 包未正确到达！已存入" << std::endl;
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

				////记录当前帧的帧号
				//fenbaoNote = framemap;