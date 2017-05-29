#include "stdafx.h"

#include "CRCSocket.h"
#include "Util.h"
#include "CSettings.h"
#include "CRCLogger.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

const std::string CRCSocket::requestID = "CRCSocket";

CRCSocket::CRCSocket()
{
	if (CRCSocket_Constructor_Log) LOG_INFO("CRCSocket", "CRCSocket", (boost::format("Start... hWnd=%1%") % hWnd).str().c_str());

	hole = new char[TXRXBUFSIZE];
}


CRCSocket::~CRCSocket()
{
	if (hole)
		delete[] hole;
	if (client) {
		if (client->buff) {
			delete[] client->buff;
			client->buff = nullptr;
		}
		delete client;
		client = nullptr;
	}
	if (s_rdrinit)
		delete s_rdrinit;

	for (auto it = begin(Tracks); it != end(Tracks); ++it)
		delete (*it);

	Tracks.clear();

	for (auto it = begin(trak); it != end(trak); ++it)
		delete (*it);
	trak.clear();

	if (CurrentPosition)
		delete CurrentPosition;
}

void CRCSocket::Init(HWND hwnd)
{
	hWnd = hwnd;
	string context = "CRCSocket::Init()";
	LOG_INFO(requestID, context, "Start");

	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		ErrorText = "Winsock error - Winsock initialization failed!";
		LOG_ERROR__(ErrorText.c_str());
		WSACleanup();
	}

	// Create our socket

	Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Socket == INVALID_SOCKET)
	{
		ErrorText = "Winsock error - Socket creation Failed!";
		LOG_ERROR__(ErrorText.c_str());
		WSACleanup();
	}
	int nResult = WSAAsyncSelect(Socket, hWnd, WM_SOCKET, (FD_CLOSE | FD_READ | FD_CONNECT));
	if (nResult)
	{
		LOG_ERROR__("WSAAsyncSelect failed");
		MessageBox(hWnd, L"WSAAsyncSelect failed", L"Critical Error", MB_ICONERROR);
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
	}
	SockAddr.sin_port = htons(CSettings::GetInt(IntPort));
	SockAddr.sin_family = AF_INET;
	// Resolve IP address for hostname	
	if ((host = gethostbyname(CSettings::GetString(StringHostName).c_str())) == nullptr)
	{
		ErrorText = "Failed to resolve hostname!";
		LOG_ERROR__(ErrorText.c_str());
		WSACleanup();
		return;
	}

	// Setup our socket address structure

	
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	if (!client)
	{
		client = new _client;
	}
	client->Socket = &Socket;

	if (!client->buff)
	{
		client->buff = new char[TXRXBUFSIZE];
	}

	

	ErrorText = "";

	IsConnected = false;

	LOG_INFO(requestID, context, "End");
}

int CRCSocket::Connect()
{
	string context = "CRCSocket::Connect()";
	LOG_INFO(requestID, context, "Start");
	// Attempt to connect to server
	if (OnceClosed) {
		LOG_INFO(requestID, context, "Socket was once closed, need to run Init()...");
		Init(hWnd);
		OnceClosed = false;
	}

	int cResult = connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr));
	if (cResult == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		wchar_t *s = nullptr;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, error,
			MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
			(LPWSTR)&s, 0, nullptr);
		wstring ws(s);

		string str(ws.begin(), ws.end());
		str = str.substr(0, str.length() - 1);
		LOG_ERROR__("cResult == SOCKET_ERROR: WSAGetLastError returned %d, %s", error, str.c_str());
	}
	//int errorCode = WSAGetLastError();
	LOG_INFO(requestID, context, (boost::format("End: return %1%") % cResult).str().c_str());
	return cResult;
}

int CRCSocket::Read()
{
	string context = "CRCSocket::Read()";
	if (CRCSocketReadLogEnabled) LOG_INFO(requestID, context, "Start");
	if (!IsConnected && !OnceClosed)
	{
		LOG_INFO(requestID, context, "First read, setting state to 'Connected'");
		IsConnected = true;
		PostMessage(hWnd, CM_CONNECT, IsConnected, NULL);
	}
	if (!IsConnected)
	{
		LOG_ERROR__("Not connected.");
		return 0;
	}
	if (!client->buff)
	{
		LOG_ERROR__("client->buff is nullptr.");
		return 0;
	}

	char *szIncoming = nullptr;
	_sh *sh;
	int recev = 0, this_recv_offset = 0, sh_offset = 0;
	char * OutBuff = nullptr;

	try {		
		sh_offset = 0;
		recev = recv(Socket, client->buff + client->recv_offset, TXRXBUFSIZE - client->recv_offset, 0);
		if (CRCSocketReadLogEnabled) LOG_INFO__("client->read_number=%d, recev=%d, client->recv_offset=%d", client->read_number, recev, client->recv_offset);
		client->read_number++;
		if (recev<=0)
		{
			LOG_WARN__("recev<=0");
			return recev;
		}
		this_recv_offset = client->recv_offset + recev;
		if (this_recv_offset < sizeof(_sh)) //приняли меньше шапки
		{		
			if (CRCSocketReadLogEnabled) LOG_INFO__("this_recv_offset = %d < sizeof(_sh)", this_recv_offset);
			client->recv_offset += recev;
			return recev;
		}
		sh = (struct _sh*)(client->buff);
		if (!test_sh(sh))
		{
			if (CRCSocketReadLogEnabled) LOG_ERROR__("wrong sh at position 0: sh = (struct _sh*)(client->buff);");
			return recev;
		}
		else
		{
			if (CRCSocketReadLogEnabled) LOG_INFO__("first block has length=%d", sh->dlina);
		}
		while(sh_offset < TXRXBUFSIZE && sh_offset + sh->dlina <= this_recv_offset)
		{
			if (CRCSocketReadLogEnabled) LOG_INFO__("while loop, sh_offset=%d, sh->dlina=%d", sh_offset, sh->dlina);
			sh = (struct _sh*)(client->buff + sh_offset);
			if (!test_sh(sh))
			{
				if (CRCSocketReadLogEnabled) LOG_ERROR__("wrong sh at position [%d]: sh = (struct _sh*)(client->buff);", sh_offset);
				return recev;
			}
			OutBuff = new char[sh->dlina];
			memcpy(OutBuff, client->buff + sh_offset, sh->dlina);
			PostMessage(hWnd, CM_POSTDATA, WPARAM(OutBuff), sh->dlina);
			if (CRCSocketReadLogEnabled) LOG_INFO__("sent %d bytes to processing, sh->type=%d", sh->dlina, sh->type);
			if (this_recv_offset - sh_offset - sh->dlina > sizeof(_sh))
			{
				// next sh_ can be retrieved
				if (CRCSocketReadLogEnabled) LOG_INFO__("next sh_ can be retrieved. we move pointer sh_offset: %d -> %d", sh_offset, sh_offset + sh->dlina);
				sh_offset += sh->dlina;
				sh = (struct _sh*)(client->buff + sh_offset);
				if (CRCSocketReadLogEnabled) LOG_INFO__("sh->dlina after retrieve = %d", sh->dlina);
			}
			else
			{
				if (CRCSocketReadLogEnabled) LOG_INFO__("next sh_ can NOT be retrieved. remaining tail length = %d", this_recv_offset - sh_offset - sh->dlina);
				if (this_recv_offset - sh_offset - sh->dlina > 0) 
				{
					memcpy(client->buff, client->buff + sh_offset + sh->dlina, this_recv_offset - sh_offset - sh->dlina);					
				}
				client->recv_offset = this_recv_offset - sh_offset - sh->dlina;
				return recev;
			}
		}

		

		
			if (CRCSocketReadLogEnabled) LOG_INFO__("some data for the next read. remaining tail length = %d", this_recv_offset - sh_offset);

			if (sh_offset > 0)
			{
				memcpy(client->buff, client->buff + sh_offset, this_recv_offset - sh_offset);
			}
			client->recv_offset = this_recv_offset - sh_offset;

			return recev;
		

		if (CRCSocketReadLogEnabled) LOG_INFO__("wtf? sh_offset=0?");

		return recev;
		
		
	}
	catch (std::bad_alloc)
	{  // ENTER THIS BLOCK ONLY IF bad_alloc IS THROWN.
	   // YOU COULD REQUEST OTHER ACTIONS BEFORE TERMINATING
		return 0;
	}

}

int CRCSocket::Close()
{
	closesocket(Socket);
	IsConnected = false;
	OnceClosed = true;
	if (client && client->buff) {
		delete[] client->buff;
		client->buff = nullptr;
	}
	if (s_rdrinit)
	{
		delete s_rdrinit;
		s_rdrinit = nullptr;
	}
	PostMessage(hWnd, CM_CONNECT, IsConnected, NULL);
	return 0;
}

unsigned int CRCSocket::PostData(WPARAM wParam, LPARAM lParam)
{
	string context = "CRCSocket::PostData()";
	try
	{
		PointOK = TrackOK = ImageOK = false;
		info_p = nullptr;
		pts = nullptr;
		int readBufLength = lParam;

		_sh *sh = (_sh*)wParam;

		sh->times = time(nullptr);
		PTR_D = &sh[1];

		switch (sh->type)
		{
		case MSG_RPOINTS:
		{
			info_p = (RPOINTS*)(void*)PTR_D;

			//test for data integrity:
			int controlSum = readBufLength - sizeof(_sh) - sizeof(RPOINTS) - info_p->N * sizeof(RPOINT);
			if (controlSum == 0)
			{
				PointOK = true;
			}
			else
			{
				LOG_ERROR__("MSG_RPOINTS: readBufLength - sizeof(_sh) - sizeof(RPOINTS) - info_p->N*sizeof(RPOINT) != 0");
				return -1;
			}

			pts = (RPOINT*)(void*)(info_p + 1);
			b1 = info_p->d1;
			b2 = info_p->d2;

			if (CRCSocketPostDataLogEnabled)
			{
				if (info_p && pts)
				{
					LOG_INFO(requestID, context, (boost::format("MSG_RPOINTS. D=%1%, N=%2%, d1=%3%, d2=%4%, pts[0].B=%5%") % info_p->D % info_p->N % info_p->d1 % info_p->d2 % pts[0].B).str().c_str());
					//TODO: pts may be empty...
				}
				if (!info_p)
				{
					LOG_WARN(requestID, context, "info_p is nullptr");
				}
				if (!pts)
				{
					LOG_WARN(requestID, context, "pts is nullptr");
				}
				
			}
		}
		break;
		case MSG_RIMAGE:
		{
			RIMAGE *imginfo = (RIMAGE*)PTR_D;
			//test for data integrity:
			int controlSum = readBufLength - sizeof(_sh) - sizeof(RIMAGE) - imginfo->N * imginfo->NR * 4;
			if (controlSum == 0)
			{
				ImageOK = true;
			}
			else
			{
				LOG_ERROR__("MSG_RIMAGE: readBufLength - sizeof(_sh) - sizeof(RIMAGE) - imginfo->N * imginfo->NR * 4 != 0");
				return -1;
			}
			info_i = (RIMAGE*)PTR_D;
			pixels = (void*)&((RIMAGE*)PTR_D)[1];
			if (CRCSocketPostDataLogEnabled)
			{
				if (info_i)
				{
					LOG_INFO(requestID, context, (boost::format("MSG_RIMAGE. D=%1%, N=%2%, d1=%3%, d2=%4%, NR=%5%") % info_i->D % info_i->N % info_i->d1 % info_i->d2 % info_i->NR).str().c_str());
				}
				if (!info_i)
				{
					LOG_WARN(requestID, context, "info_i is nullptr");
				}
			}
		}
		break;
		// 
		case MSG_PTSTRK:
		{
			if (CRCSocketPostDataLogEnabled)
			{
				LOG_INFO(requestID, context, "MSG_PTSTRK");
			}
		}
		break;
		case MSG_OBJTRK:
		{
			int N = *((int*)((void*)PTR_D));
			RDRTRACK* pTK = (RDRTRACK*)(void*)(((char*)PTR_D) + 4);
			if (CRCSocketPostDataLogEnabled)
			{
				if (pTK)
				{
					LOG_INFO(requestID, context, (boost::format("MSG_OBJTRK. N=%1%, numTrack=%2%") % N % pTK->numTrack).str().c_str());
				}
				if (!pTK)
				{
					LOG_WARN(requestID, context, "pTK is nullptr");
				}
			}
			OnSrvMsg_RDRTRACK(pTK, N);
		}
		break;
		case MSG_INIT:
		{
			if (!s_rdrinit)
			{
				s_rdrinit = new RDR_INITCL;
			}

			memcpy(s_rdrinit, (RDR_INITCL*)(void*)&sh[1], sizeof(RDR_INITCL));

			if (s_rdrinit->MaxNAzm <= 0 && s_rdrinit->dAzm >= 0)
			{
				s_rdrinit->MaxNAzm = int(M_PIx2 / s_rdrinit->dAzm);
			}

			if (CRCSocketPostDataLogEnabled)
			{
				LOG_INFO__("MSG_INIT. Nazm=%d, Nelv=%d, dAzm=%f, dElv=%f, begAzm=%f, begElv=%f,	dR=%f, NR=%d, minR=%f, maxR=%f, ViewStep=%d, Proto[0]=%d, Proto[1]=%d, ScanMode=%d, srvTime=%d, MaxNumSectPt=%d, MaxNumSectImg=%d, blankR1=%d, blankR2=%d, MaxNAzm=%d, MaxNElv=%d",
					s_rdrinit->Nazm, s_rdrinit->Nelv,
					s_rdrinit->dAzm, s_rdrinit->dElv,
					s_rdrinit->begAzm, s_rdrinit->begElv,
					s_rdrinit->dR,
					s_rdrinit->NR,
					s_rdrinit->minR, s_rdrinit->maxR,
					s_rdrinit->ViewStep,
					s_rdrinit->Proto[0], s_rdrinit->Proto[1],
					s_rdrinit->ScanMode,
					s_rdrinit->srvTime,
					s_rdrinit->MaxNumSectPt,
					s_rdrinit->MaxNumSectImg,
					s_rdrinit->blankR1,
					s_rdrinit->blankR2,
					s_rdrinit->MaxNAzm,
					s_rdrinit->MaxNElv);
				try
				{
					s_rdrinit->MaxNAzm = CSettings::GetInt(IntNazm);
					LOG_INFO__("Using settings value MaxNAzm=%d", s_rdrinit->MaxNAzm);
				}
				catch (const std::out_of_range& oor)
				{
					LOG_INFO__("Using server value MaxNAzm=%d", s_rdrinit->MaxNAzm);
				}
				/*
				struct RDR_INITCL
				{
				int           Nazm;         // offs 0    число дискретов по азимуту
				int           Nelv;         // offs 4
				double        dAzm;         // offs 8
				double        dElv;         // offs 16
				double        begAzm;       // offs 24
				double        begElv;       // offs 32
				double        dR;           // offs 40
				int           NR;           // offs 44
				double        minR;         // offs 52
				double        maxR;         // offs 60
				char          resv1[32];    // offs 68
				int           ViewStep;     // offs 100
				short         Proto[2];     // offs 104
				unsigned int  ScanMode;     // offs 108
				UTCtime       srvTime;
				int           MaxNumSectPt;
				int           MaxNumSectImg;
				int           blankR1;
				int           blankR2;
				char          resv2[1024 - 104 - 16 - 4 - 4 - 4 - 4];
				};
				*/
			}
		}
		break;
		case MSG_LOCATION:
		{
			RDRCURRPOS* igpsp = (RDRCURRPOS*)(void*)((char*)PTR_D);
			if (CRCSocketPostDataLogEnabled)
			{
				LOG_INFO(requestID, context, (boost::format("MSG_LOCATION. lon=%1%, lat=%2%") % igpsp->lon % igpsp->lat).str().c_str());
			}
			OnSrvMsg_LOCATION(igpsp);
		}
		break;
		default:
		{
			return 0;;
		}
		break;
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
	string context = "CRCSocket::OnSrvMsg_RDRTRACK";
	for (int i = 0; i < N; i++)
	{
		// ищем трек
		int Idx = FindTrack(info[i].numTrack);
		if (-1 == Idx)
		{
			if (CRCSocketPostDataLogEnabled) LOG_INFO(requestID, context, (boost::format("MSG_OBJTRK. N=%1%, track not found, creating new") % N).str().c_str());
			//создаём трек
			TRK* t1 = new TRK(info[i].numTrack);
			Tracks.push_back(t1);
			// добавим точки
			t1->InsertPoints(info + i, 1);
		}
		// уже есть
		else if (Idx >= 0 && Idx < Tracks.size())
		{
			if (CRCSocketPostDataLogEnabled) LOG_INFO(requestID, context, (boost::format("MSG_OBJTRK. N=%1%, track found") % N).str().c_str());
			Tracks[Idx]->InsertPoints(info + i, 1);
		}
	}
}

void CRCSocket::OnSrvMsg_LOCATION(RDRCURRPOS* d)
{
	if (!CurrentPosition)
		CurrentPosition = new RDRCURRPOS;
	memcpy(CurrentPosition, d, sizeof(RDRCURRPOS));
	Initialized = true;
}

void CRCSocket::OnSrvMsg_INIT(RDR_INITCL* s_rdrinit)
{
	this->s_rdrinit = s_rdrinit;
}

unsigned CRCSocket::_IMG_MapAmp2ColorRGB255(float Amp, float Min)
{
	unsigned int colorImg;
	int i;
	float iScale = 3;

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

void CRCSocket::FreeMemory(char *readBuf) const
{
	if (readBuf)
	{
		delete[] readBuf; // operator new in method Read() (variable OutBuf)
	}
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
	LOG_INFO(CRCSocket::requestID, "TRK DESTRUCTOR", (boost::format("id=%1%") % this->id).str().c_str());
	for (int i = 0; i < P.size(); i++)
	{
		if (P.at(i))
		{
			delete P.at(i);
		}
	}
	P.clear();
}

bool CRCSocket::test_sh(_sh* ptr)
{
	if (ptr->word1 != 2863311530/* 0xAAAAAAAAu*/ || ptr->word2 != 1431655765 /*0x55555555u*/)   // проверим шапку
	{
		return false;
	}
	return true;
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
