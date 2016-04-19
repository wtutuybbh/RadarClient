//#include "stdafx.h"
#include "CRCSocket.h"


CRCSocket::CRCSocket(HWND hWnd)
{
	this->hWnd = hWnd;
	OnceClosed = false;
	Init();

	this->info_p = NULL;
	this->info_i = NULL;

	this->pts = NULL;
	this->s_rdrinit = NULL;
}


CRCSocket::~CRCSocket()
{
}

void CRCSocket::Init()
{
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		ErrorText = "Winsock error - Winsock initialization failed!";
		WSACleanup();
	}

	// Create our socket

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		ErrorText = "Winsock error - Socket creation Failed!";
		WSACleanup();
	}
	int nResult = WSAAsyncSelect(Socket, hWnd, WM_SOCKET, (FD_CLOSE | FD_READ));
	if (nResult)
	{
		MessageBox(hWnd, "WSAAsyncSelect failed", "Critical Error", MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
	}
	// Resolve IP address for hostname	
	if ((host = gethostbyname("localhost")) == NULL)
	{
		ErrorText = "Failed to resolve hostname!";
		WSACleanup();
	}

	// Setup our socket address structure

	SockAddr.sin_port = htons(10001);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	client = new _client;
	client->Socket = &Socket;
	client->offset = 0;
	client->buff = new char[65535];

	ErrorText = "";

	IsConnected = false;
}

int CRCSocket::Connect()
{
	// Attempt to connect to server
	if (OnceClosed)
		Init();

	int cResult = connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr));
	int errorCode = WSAGetLastError();
	/*if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0)
	{
		ErrorText = "Failed to establish connection with server!";
		WSACleanup();
		return -1;
	}*/

	// If iMode!=0, non-blocking mode is enabled.
	/*u_long iMode = 1;
	ioctlsocket(Socket, FIONBIO, &iMode);*/
	/*if (cResult == 0 || errorCode == WSAEWOULDBLOCK) {
		IsConnected = true;
	}*/
	return cResult;
}

int CRCSocket::Read()
{
	IsConnected = true;
	PostMessage(hWnd, CM_CONNECT, IsConnected, NULL);

	char szIncoming[1024];
	ZeroMemory(szIncoming, sizeof(szIncoming));

	_sh *sh;
	int recev;
	unsigned int offset, length;
	char * OutBuff;

	/*int inDataLength = recv(Socket,
		(char*)szIncoming,
		sizeof(szIncoming) / sizeof(szIncoming[0]),
		0);*/

	recev = recv(Socket, client->buff + client->offset, 65535, 0);
	/*strncat(szHistory, szIncoming, inDataLength);
	strcat(szHistory, "\r\n");*/
	offset = recev + client->offset;
	if (offset < sizeof(_sh)) //приняли меньше шапки
	{
		client->offset = offset; //Запомнили что что-то приняли
		return recev;
	}
	sh = (struct _sh*)client->buff;
	if (sh->word1 != 0xAAAAAAAA || sh->word2 != 0x55555555)   // проверим шапку
		return recev;
	length = sh->dlina;
	while (length <= offset)  // приняли больше или ровно 1 порцию
	{
		try // защита на выделение памяти
		{
			OutBuff = new char[length];              // Выделим память под принятую порцию
			memcpy(OutBuff, client->buff, length); // Скопируем принятые данные

			PostMessage(hWnd, CM_POSTDATA, (WPARAM)OutBuff, 0); //Отошлем в очередь на обработку
			memcpy(client->buff, &client->buff[length], offset - length);// Перепишем остаток в начало

			offset -= length;
			length = sh->dlina * 4;
			if (sh->word1 != 0xAAAAAAAA || sh->word2 != 0x55555555)   // проверим шапку
			{
				client->offset = 0;
				return recev;
			}

		}
		catch (std::bad_alloc)
		{  // ENTER THIS BLOCK ONLY IF bad_alloc IS THROWN.
		   // YOU COULD REQUEST OTHER ACTIONS BEFORE TERMINATING
			return recev;
		}
	}
	//приняли меньше чем 1 порция
	client->offset = offset; //Запомним что что-то приняли
	return recev;
}

int CRCSocket::Close()
{
	
	closesocket(Socket);
	IsConnected = false;
	OnceClosed = true;
	PostMessage(hWnd, CM_CONNECT, IsConnected, NULL);
	return 0;
}

void CRCSocket::PostData(WPARAM wParam, LPARAM lParam)
{
	try
	{
		_sh *sh = (_sh*)wParam;
		sh->times = time(NULL);
		PTR_D = &sh[1];

		switch (sh->type)
		{
		case MSG_RPOINTS:
		{
			info_p = (RPOINTS*)(void*)PTR_D;
			pts = (RPOINT*)(void*)(info_p + 1);
			//Memo1->Lines->Add("RPOINTS  n=" + IntToStr(info_p->N));
			b1 = info_p->d1;
			b2 = info_p->d2;
			/*if (b2 < b1)
			{
			CurrScan = int(b1 / rdrinit.ViewStep);
			}
			else CurrScan = int(b1 / rdrinit.ViewStep);
			Edit2->Text = IntToStr(CurrScan);*/
			//PTG[CurrScan]->Clear();
			/*for (int i = 0; i < info_p->N; i++)
			{
			//PTG[CurrScan]->AddXY(pts[i].B,pts[i].B,"",clTeeColor);
			}*/
		}
		break;
		case MSG_RIMAGE:
			info_i = (RIMAGE*)(void*)PTR_D;
			break;

		// 
		case MSG_PTSTRK:
			//Memo1->Lines->Add("PTSTRK");
			break;
		case MSG_OBJTRK:
			//Memo1->Lines->Add("OBJTRK");
			break;
		case MSG_DELTRK:
			//Memo1->Lines->Add("DELTRK");
			break;

			// 
		case MSG_INIT:
		{
			//Memo1->Lines->Add("MSGINIT");
			s_rdrinit = (RDR_INITCL*)(void*)&sh[1];
			//DoInit(s_rdrinit);
			break;
		}
		default:
			break;
		}

	}
	catch (...) {

	}
	//delete[](char*)wParam;
}
