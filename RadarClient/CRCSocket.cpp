//#include "stdafx.h"
#include "CRCSocket.h"
#include <mutex>


CRCSocket::CRCSocket(HWND hWnd, std::mutex* m)
{
	this->m = m;

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
				return;			

			pts = (RPOINT*)(void*)(info_p + 1);
			b1 = info_p->d1;
			b2 = info_p->d2;
		}
		break;
		case MSG_RIMAGE: {
			RIMAGE *imginfo = (RIMAGE*)PTR_D;
			//test for data integrity:
			int controlSum = readBufLength - sizeof(_sh) - sizeof(RIMAGE) - imginfo->N * imginfo->NR * 4;
			if (controlSum == 0)
				ImageOK = true;
			else
				return;
			OnSrvMsg_RIMAGE((RIMAGE*)PTR_D, (void*)&((RIMAGE*)PTR_D)[1]);
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
		case MSG_DELTRK: {
			//Memo1->Lines->Add("DELTRK");
			int N = *((int*)((void*)PTR_D));
			int* DTK = (int*)(void*)((char*)PTR_D + 4);
			OnSrvMsg_DELTRACK(DTK, N);
		}
			break;
		
			// 
		case MSG_INIT: {
			//Memo1->Lines->Add("MSGINIT");
			if (!s_rdrinit)
				s_rdrinit = new RDR_INITCL;

			memcpy(s_rdrinit, (RDR_INITCL*)(void*)&sh[1], sizeof(RDR_INITCL));
			OnSrvMsg_INIT((RDR_INITCL*)(void*)&sh[1]);
			//DoInit(s_rdrinit);			
		}
		case MSG_LOCATION:
		{
			RDRCURRPOS* igpsp = (RDRCURRPOS*)(void*)((char*)PTR_D);
			OnSrvMsg_LOCATION(igpsp);
			break;
		}
		break;
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
	m->lock();
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j<Tracks.size(); j++)
		{
			if (Tracks[j]->id == deltrackz[i])
			{
				delete Tracks[j];
				Tracks.erase(Tracks.begin() + j);//TODO: crashes here!!!
				//break;
			}
		}
	}
	m->unlock(); //against crash - but did not help
}

void CRCSocket::OnSrvMsg_LOCATION(RDRCURRPOS* d)
{
	if (!CurrentPosition)
		CurrentPosition = new RDRCURRPOS;
	memcpy(CurrentPosition, d, sizeof(RDRCURRPOS));
	Initialized = true;
	/*AnsiString s;
	s.sprintf("%lf", R2D(d->northdir));
	Edit1->Text = s;
	s.sprintf("%d", int(d->currstate));
	Edit2->Text = s;
	s.sprintf("%lf", (d->lon));
	Edit3->Text = s;
	s.sprintf("%lf", (d->lat));
	Edit4->Text = s;
	s.sprintf("%lf", (d->elv));
	Edit5->Text = s;
	s.sprintf("%lf", (d->direction));
	Edit6->Text = s;
	s.sprintf("%hd.%hd.%hd   %hd:%hd:%hd.%3hd",
		d->srvTime.st.wDay, d->srvTime.st.wMonth, d->srvTime.st.wYear,
		d->srvTime.st.wHour, d->srvTime.st.wMinute, d->srvTime.st.wSecond,
		d->srvTime.st.wMilliseconds);*/
}

void CRCSocket::OnSrvMsg_RIMAGE(RIMAGE* info, void* pixels)
{

	int offset_B_pixels;


	if (info->resv1[0] != 0) return; // если тип данных не флоат то хз
	{
		// новый битмап под кусок нового РЛИ
		char flip;
		int B1 = info->d1, B2 = info->d2;
		if (B2 < B1)
		{
			offset_B_pixels = B2;
			flip = 1;

		}
		else
		{
			flip = 0;
			offset_B_pixels = B1;
		}
		IDX = offset_B_pixels / rdrinit.ViewStep;
		//AnsiString s;
		//s.sprintf("IDX %d, offsB %d", IDX, offset_B_pixels);
		//lblB->Caption = s;
		//if(IDX < 0) IDX=0;
		//if(IDX >= NumViewSct) IDX = NumViewSct-1;

		//Series4->Clear();
		//float iX = R2D(rdrinit.begAzm+rdrinit.dAzm*offset_B_pixels);
		//float iY = rdrinit.maxR;
		//Series4->
		//Series4->AddXY(iX,iY,"",clTeeColor);


		// ищем макс. значение пикселя, затем нормируем и приводим к [0-255]
		//float* px = (float*)pixels;
		//float maxP = 0, x;

		// формируем пиксели из данных
		/*for (int i = 0; i < SCTB->Height; i++)
		{
			DWORD* P = (DWORD*)SCTB->ScanLine[i];
			for (int j = 0; j < SCTB->Width; j++)
			{
				int B = px[j*info->NR + 0];
				DWORD* P = (DWORD*)SCTB->ScanLine[SCTB->Height - 1 - i];
				DWORD L = _IMG_MapAmp2ColorRGB255(px[j*info->NR + i + 1], 0);

				if (flip) P[SCTB->Width - 1 - j] = L;
				else P[j] = L;

				//P[j] = RGB(rand()%255,rand()%255,rand()%255);
			}
		}*/

		//vsimg[IDX]->Assign(SCTB);

		/*
		//DBG
		for(int i = 0; i < SCTB->Height; i++)
		{
		DWORD* P = (DWORD*)SCTB->ScanLine[i];
		for(int j = 0; j < SCTB->Width; j++)
		{
		P[j] = RGB(rand()%255,rand()%255,rand()%255);
		}
		}
		Memo1->Lines->Add("OnSrvMsg_RIMAGE dbg шум");
		//
		*/
		//RLI_BTM->Canvas->CopyMode = cmSrcCopy;
		// теперь в общем битмапе заменим кусок
		/*for (int i = 0; i < NumViewSct; i++)
		{
			//if(i!=IDX)
			//RLI_BTM->Canvas->Draw(i*rdrinit.ViewStep,0, vsimg[i]);

		}*/

	}
}

void CRCSocket::OnSrvMsg_INIT(RDR_INITCL* s_rdrinit)
{
	DoInit(s_rdrinit);
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
	Found = false;
}
TRK::~TRK()
{
	P.clear();
}
void TRK::InsertPoints(RDRTRACK* pt, int N)
{
	for (int i = 0; i < N; i++)
	{
		if (pt->numTrack != this->id) continue;
		P.push_back(pt[i]);
	}
}