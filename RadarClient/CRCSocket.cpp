#define WIN32_LEAN_AND_MEAN

//#include "stdafx.h"
#include "Util.h"
#include "CRCSocket.h"
#include "CSettings.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

CRCSocket::CRCSocket(HWND hWnd/*, std::mutex* m*/)
{
	//this->m = m;

	this->hWnd = hWnd;
	OnceClosed = false;
	

	this->info_p = NULL;
	this->info_i = NULL;

	this->pts = NULL;
	this->s_rdrinit = NULL;

	hole = new char[TXRXBUFSIZE];


	s_rdrinit = NULL;

	client = NULL;

	PointOK = TrackOK = ImageOK = false;

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

	/*if (info_p)
		delete info_p;*/

	for (auto it = begin(Tracks); it != end(Tracks); ++it)
		delete (*it);
	Tracks.clear();

	for (auto it = begin(trak); it != end(trak); ++it)
		delete (*it);
	trak.clear();

	if (CurrentPosition)
		delete CurrentPosition;
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
	if ((host = gethostbyname(CSettings::GetString(StringHostName).c_str())) == NULL)
	{
		ErrorText = "Failed to resolve hostname!";
		WSACleanup();
	}

	// Setup our socket address structure

	SockAddr.sin_port = htons(CSettings::GetInt(IntPort));
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
		if (sh->word1 != 2863311530/* 0xAAAAAAAAu*/ || sh->word2 != 1431655765 /*0x55555555u*/)   // проверим шапку
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
				if (sh->word1 != 2863311530 /*0xAAAAAAAAu*/ || sh->word2 != 1431655765 /*0x55555555u*/)   // проверим шапку 
				{
					client->offset = 0;
					if (szIncoming)
						delete[] szIncoming;
					return recev;
				}

			}
			catch (std::bad_alloc)
			{  // ENTER THIS BLOCK ONLY IF bad_alloc IS THROWN.
			   // YOU COULD REQUEST OTHER ACTIONS BEFORE TERMINATING
				if (szIncoming)
					delete[] szIncoming;
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

unsigned int CRCSocket::PostData(WPARAM wParam, LPARAM lParam)
{
	try
	{
		PointOK = TrackOK = ImageOK = false;
		info_p = NULL;
		pts = NULL;
		//ReadBuf = (char*)wParam;
		int readBufLength = lParam;

		_sh *sh = (_sh*)wParam;

		sh->times = time(NULL);
		PTR_D = &sh[1];

		switch (sh->type)
		{
		case MSG_RPOINTS:
		{


			info_p = (RPOINTS*)(void*)PTR_D;

			//test for data integrity:
			int controlSum = readBufLength - sizeof(_sh) - sizeof(RPOINTS) - info_p->N*sizeof(RPOINT);
			if (controlSum == 0)			
				PointOK = true;			
			else
				return -1;			

			pts = (RPOINT*)(void*)(info_p + 1);
			b1 = info_p->d1;
			b2 = info_p->d2;
#ifdef _DEBUG
			if (info_p) {
				std::stringstream s;
				s << "MSG_RPOINTS. D=" << info_p->D << ", N=" << info_p->N << " d1=" << info_p->d1 << ", d2=" << info_p->d2 << " pts[0].B=" << pts[0].B;
				DebugMessage(dwi, s.str());
			}
#endif
		}
		break;
		case MSG_RIMAGE: {
			RIMAGE *imginfo = (RIMAGE*)PTR_D;
			//test for data integrity:
			int controlSum = readBufLength - sizeof(_sh) - sizeof(RIMAGE) - imginfo->N * imginfo->NR * 4;
			if (controlSum == 0)
				ImageOK = true;
			else
				return -1;
			//OnSrvMsg_RIMAGE((RIMAGE*)PTR_D, (void*)&((RIMAGE*)PTR_D)[1]);
			info_i = (RIMAGE*)PTR_D;
			pixels = (void*)&((RIMAGE*)PTR_D)[1];
			break;
		}
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
	
			// 
		case MSG_INIT: {
			//Memo1->Lines->Add("MSGINIT");
			if (!s_rdrinit)
				s_rdrinit = new RDR_INITCL;

			memcpy(s_rdrinit, (RDR_INITCL*)(void*)&sh[1], sizeof(RDR_INITCL));
			//OnSrvMsg_INIT((RDR_INITCL*)(void*)&sh[1]);
			//DoInit(s_rdrinit);	
#ifdef _DEBUG
			if (s_rdrinit) {
				std::stringstream s;
				s << "MSG_INIT. ViewStep=" << s_rdrinit->ViewStep << ", MaxNumSectPt=" << s_rdrinit->MaxNumSectPt << " Nazm=" << s_rdrinit->Nazm;
				DebugMessage(dwi, s.str());
			}
#endif
		}
		case MSG_LOCATION:
		{
			RDRCURRPOS* igpsp = (RDRCURRPOS*)(void*)((char*)PTR_D);
			OnSrvMsg_LOCATION(igpsp);
			break;
		}
		break;
		default: 
		{
			return 0;;
		}
		}
		return sh->type;

	}
	catch (...) {
		return -1;
	}
	return 0;
	//delete[](char*)wParam;
}

void CRCSocket::OnSrvMsg_RDRTRACK(RDRTRACK * info, int N)
{
	for (int i = 0; i < N; i++)
	{
		// ищем трек
		int Idx = FindTrack(info[i].numTrack);
		//RectToPolar2d(info[i].X, info[i].Y, &info[i].X, &info[i].Y);
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
/*
void CRCSocket::OnSrvMsg_DELTRACK(int* deltrackz, int N)
{
	m->lock();
	for (int i = 0; i < min(N, Tracks.size()); i++)
	{
		for (auto it = Tracks.begin();
		it != Tracks.end();) 
		{

			if ((*it)->id == deltrackz[i]) {
				delete *it;
				it = Tracks.erase(it);
			}
			else
				++it;
		}
	}
	m->unlock(); //against crash - but did not help
}*/

void CRCSocket::OnSrvMsg_LOCATION(RDRCURRPOS* d)
{
	if (!CurrentPosition)
		CurrentPosition = new RDRCURRPOS;
	memcpy(CurrentPosition, d, sizeof(RDRCURRPOS));
	Initialized = true;
}

void CRCSocket::OnSrvMsg_RIMAGE(RIMAGE* info, void* pixels)
{	
}

void CRCSocket::OnSrvMsg_INIT(RDR_INITCL* s_rdrinit)
{
	this->s_rdrinit = s_rdrinit;
	//DoInit(s_rdrinit);
}

unsigned CRCSocket::_IMG_MapAmp2ColorRGB255(float Amp, float Min)
{
	unsigned int colorImg;
	int i;
	float iScale = 3;
	//	if(maxValue-minValue>=0.01) iScale=1024.0/(maxValue-minValue);
	//    else	iScale=102400;


	colorImg = (Amp - Min)*iScale;
	if (colorImg >= 1024)
	{
		colorImg = 0xFF0000;
	}

	else if (colorImg >= 896)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((128 - colorImg) << 8) | 0xFF0000;

	}
	else if (colorImg >= 768)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((255 - colorImg) << 8) | 0xFF0000;
	}
	else if (colorImg >= 640)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((colorImg + 128) << 16) + ((colorImg + 128) << 8);
	}
	else if (colorImg >= 512)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((colorImg) << 16) + ((255 - colorImg) << 8);
	}
	else if (colorImg >= 384)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((colorImg + 128) << 8) + 128 - (colorImg);
	}
	else if (colorImg >= 256)
	{
		colorImg = colorImg & 0x007F;
		colorImg = (colorImg << 8) + 255 - (colorImg);
	}

	return colorImg;
}

unsigned CRCSocket::MapAmp2ColorRGB255(float Amp, float Min)
{
	float iScale = 3;
	DWORD colorImg;

	colorImg = (Amp - Min)*iScale;
	if (colorImg >= 1024)
	{
		colorImg = 0xFF0000;
	}

	else if (colorImg >= 896)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((128 - colorImg) << 8) | 0xFF0000;

	}
	else if (colorImg >= 768)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((255 - colorImg) << 8) | 0xFF0000;
	}
	else if (colorImg >= 640)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((colorImg + 128) << 16) + ((colorImg + 128) << 8);
	}
	else if (colorImg >= 512)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((colorImg) << 16) + ((255 - colorImg) << 8);
	}
	else if (colorImg >= 384)
	{
		colorImg = colorImg & 0x007F;
		colorImg = ((colorImg + 128) << 8) + 128 - (colorImg);
	}
	else if (colorImg >= 256)
	{
		colorImg = colorImg & 0x007F;
		colorImg = (colorImg << 8) + 255 - (colorImg);
	}

	return colorImg;
}

void CRCSocket::DoInit(RDR_INITCL* init)
{
}

int CRCSocket::FindTrack(int id) // если в массиве trak нашли то вернем индекс, иначе -1
{
	for (int i = 0; i < Tracks.size(); i++)
	{
		if (Tracks[i]->id == id) return i;
	}
	return -1;
}
/*void CRCSocket::RectToPolar2d(double x, double y, double* phi, double* ro)
{
	double x1 = x, y1 = y;

	*phi = -120 + 0.02 * 1000 * (atan(x1 / y1)) / M_PI_180;
	*ro = 0.75 * sqrt(x1*x1 + y1*y1) / 1;
}*/

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
	int stop = 0;
	if (_id<0 || _id>10000)
	{
		stop = 1;
	}
	id = _id;
	Found = false;
}
TRK::~TRK()
{
	for (int i = 0; i < P.size(); i++)
	{
		if (P.at(i))
		{
			delete P.at(i);
		}
	}
	P.clear();
}
void TRK::InsertPoints(RDRTRACK* pt, int N)
{
	for (int i = 0; i < N; i++)
	{
		if (pt->numTrack != this->id) continue;
		RDRTRACK *t = new RDRTRACK();
		memcpy(t, &pt[i], sizeof(RDRTRACK));
		P.push_back(t);
	}
}