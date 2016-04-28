//#include "stdafx.h"
#include "CRCSocket.h"


CRCSocket::CRCSocket(HWND hWnd)
{
	this->hWnd = hWnd;
	OnceClosed = false;
	

	this->info_p = NULL;
	this->info_i = NULL;

	this->pts = NULL;
	this->s_rdrinit = NULL;

	hole = new char[TXRXBUFSIZE];

	ReadBufLength = 0;
	ReadBuf = NULL;

	s_rdrinit = NULL;

	client = NULL;
	Init();
}


CRCSocket::~CRCSocket()
{
	if (hole)
		delete[] hole;
	if (client) {
		if (client->buff) {
			delete[] client->buff;
			client->buff = NULL;
		}

		delete client;
		client = NULL;

	}
	if (s_rdrinit)
		delete s_rdrinit;
	for (vector<TRK*>::iterator it = Tracks.begin(); it != Tracks.end(); ++it) {
		delete *it;
	}
	Tracks.clear();
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
	client->buff = new char[TXRXBUFSIZE];

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

	char *szIncoming = NULL;
	

	_sh *sh;
	int recev;
	unsigned int offset, length;
	char * OutBuff = NULL;

	/*int inDataLength = recv(Socket,
		(char*)szIncoming,
		sizeof(szIncoming) / sizeof(szIncoming[0]),
		0);*/

	try {
		szIncoming = NULL;
		szIncoming = new char[TXRXBUFSIZE];
		ZeroMemory(szIncoming, sizeof(szIncoming));
		recev = recv(Socket, client->buff + client->offset, TXRXBUFSIZE, 0);
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

				PostMessage(hWnd, CM_POSTDATA, (WPARAM)OutBuff, length); //Отошлем в очередь на обработку
				memcpy(client->buff, &client->buff[length], offset - length);// Перепишем остаток в начало

				offset -= length;
				length = sh->dlina;
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
	}
	catch (std::bad_alloc)
	{  // ENTER THIS BLOCK ONLY IF bad_alloc IS THROWN.
	   // YOU COULD REQUEST OTHER ACTIONS BEFORE TERMINATING
		return 0;
	}
	//приняли меньше чем 1 порция
	client->offset = offset; //Запомним что что-то приняли
	if (szIncoming)
		delete[] szIncoming;
	return recev;
}

int CRCSocket::Close()
{
	
	closesocket(Socket);
	IsConnected = false;
	OnceClosed = true;
	if (client && client->buff) {
		delete[] client->buff;
		client->buff = NULL;
	}
	PostMessage(hWnd, CM_CONNECT, IsConnected, NULL);
	return 0;
}

void CRCSocket::PostData(WPARAM wParam, LPARAM lParam)
{
	try
	{
		ReadBuf = (char*)wParam;
		ReadBufLength = lParam;

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
		case MSG_OBJTRK: {
			int N = *((int*)((void*)PTR_D));
			RDRTRACK* pTK = (RDRTRACK*)(void*)(((char*)PTR_D) + 4);
			OnSrvMsg_RDRTRACK(pTK, N);
			//Memo1->Lines->Add("OBJTRK");
			break;
		}
		case MSG_DELTRK: {
			//Memo1->Lines->Add("DELTRK");
			int N = *((int*)((void*)PTR_D));
			int* DTK = (int*)(void*)((char*)PTR_D + 4);
			OnSrvMsg_DELTRACK(DTK, N);
			break;
		}
			// 
		case MSG_INIT:
		{
			//Memo1->Lines->Add("MSGINIT");
			if (!s_rdrinit)
				s_rdrinit = new RDR_INITCL;
			memcpy(s_rdrinit, (RDR_INITCL*)(void*)&sh[1], sizeof(RDR_INITCL));
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

void CRCSocket::OnSrvMsg_RDRTRACK(RDRTRACK * info, int N)
{
	for (int i = 0; i < N; i++)
	{
		// ищем трек
		int Idx = FindTrack(info[i].numTrack);
		RectToPolar2d(info[i].X, info[i].Y, &info[i].X, &info[i].Y);
		if (-1 == Idx)
		{
			//создаём трек
			TRK* t1 = new TRK(info[i].numTrack);
			Tracks.push_back(t1);
			// добавим точки
			t1->InsertPoints(info + i, 1);

			//UpdateTrackTable();
			//Memo1->Lines->Add("äîáàâèëè òðåê Id = " + IntToStr(info[i].numTrack));
		}
		// уже есть
		else if (Idx >= 0 && Idx < Tracks.size())
		{
			Tracks[Idx]->InsertPoints(info + i, 1);
			//UpdateTrackTable();
			//Memo1->Lines->Add("äîáàâèëè òî÷êó ê òðåêó Id = " + IntToStr(info[i].numTrack));
		}
	}
}
void CRCSocket::OnSrvMsg_DELTRACK(int* deltrackz, int N)
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j<Tracks.size(); j++)
		{
			if (Tracks[j]->id == deltrackz[i])
			{
				delete Tracks[j];
				Tracks.erase(Tracks.begin() + j);
				//break;
			}
		}
	}
}
int CRCSocket::FindTrack(int id) // если в массиве trak нашли то вернем индекс, иначе -1
{
	for (int i = 0; i < Tracks.size(); i++)
	{
		if (Tracks[i]->id == id) return i;
	}
	return -1;
}
void CRCSocket::RectToPolar2d(double x, double y, double* phi, double* ro)
{
	double x1 = x, y1 = y;

	*phi = -120 + 0.02 * 1000 * (atan(x1 / y1)) / M_PI_180;
	*ro = 0.75 * sqrt(x1*x1 + y1*y1) / 1;
}

void CRCSocket::FreeMemory(char *readBuf)
{
	delete [] readBuf;
	/*if (ReadBuf && ReadBufLength > 0) {
		delete [] ReadBuf;
		ReadBufLength = 0;
		ReadBuf = NULL;
	}*/
}

TRK::TRK(int _id)
{
	id = _id;
}
TRK::~TRK()
{
}
void TRK::InsertPoints(RDRTRACK* pt, int N)
{
	for (int i = 0; i < N; i++)
	{
		if (pt->numTrack != this->id) continue;
		P.push_back(pt[i]);
	}
}