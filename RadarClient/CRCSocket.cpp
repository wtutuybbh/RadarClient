#include "stdafx.h"

#include "CRCSocket.h"
#include "Util.h"
#include "CSettings.h"
#include "CRCLogger.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

const std::string CRCSocket::requestID = "CRCSocket";

CRCSocket::CRCSocket(HWND hWnd) : 
	resolver(io_service), 
	query(CSettings::GetString(StringHostName), num2str(CSettings::GetInt(IntPort), 0)),
	//iterator(resolver.resolve(query)), // calls iterator constructor
	socket_(io_service)                      // calls socket constructor
{
	string context = "CRCSocket::CRCSocket";
	LOG_INFO(requestID, context, (boost::format("Start... hWnd=%1%") % hWnd).str().c_str());

	this->hWnd = hWnd;
	OnceClosed = false;
	

	this->info_p = nullptr;
	this->info_i = nullptr;

	this->pts = nullptr;
	this->s_rdrinit = nullptr;

	hole = new char[TXRXBUFSIZE];


	s_rdrinit = nullptr;

	client = nullptr;

	PointOK = TrackOK = ImageOK = false;

	
	
	//Init();
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

void CRCSocket::Init()
{
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
		MessageBox(hWnd, "WSAAsyncSelect failed", "Critical Error", MB_ICONERROR);		
		SendMessage(hWnd, WM_DESTROY, NULL, NULL);
	}
	// Resolve IP address for hostname	
	if ((host = gethostbyname(CSettings::GetString(StringHostName).c_str())) == nullptr)
	{
		ErrorText = "Failed to resolve hostname!";
		LOG_ERROR__(ErrorText.c_str());
		WSACleanup();
	}

	// Setup our socket address structure

	SockAddr.sin_port = htons(CSettings::GetInt(IntPort));
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	
	if (!client)
	{
		client = new _client;
	}
	client->Socket = &Socket;
	client->offset = 0;
	client->buff = new char[TXRXBUFSIZE];

	ErrorText = "";

	IsConnected = false;

	LOG_INFO(requestID, context, "End");
}

void CRCSocket::handle_resolve(const boost::system::error_code& err,
	tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		std::cout << "handle_resolve...\n";
		// Attempt a connection to each endpoint in the list until we
		// successfully establish a connection.
		boost::asio::async_connect(socket_, endpoint_iterator,
			boost::bind(&CRCSocket::handle_connect, this,
				boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void CRCSocket::handle_connect(const boost::system::error_code& err)
{
	if (!err)
	{
		std::cout << "handle_connect...\n";
		// The connection was successful. Send the request.
		boost::asio::async_write(socket_, request_,
			boost::bind(&CRCSocket::handle_write_request, this,
				boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void CRCSocket::handle_write_request(const boost::system::error_code& err)
{
	if (!err)
	{
		std::cout << "handle_write_request:\n";
		// Read the response status line. The response_ streambuf will
		// automatically grow to accommodate the entire line. The growth may be
		// limited by passing a maximum size to the streambuf constructor.
		/*boost::asio::async_read_until(socket_, response_, "\r\n",
		boost::bind(&client::handle_read_status_line, this,
		boost::asio::placeholders::error));*/

		boost::asio::async_read(socket_, response_,
			boost::asio::transfer_at_least(1),
			boost::bind(&CRCSocket::handle_read_response, this,
				boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void CRCSocket::handle_read_response(const boost::system::error_code& err)
{
	if (!err)
	{
		std::cout << "handle_read_response:\n";
		// Write all of the data that has been read so far.
		std::cout << &response_;

		// Continue reading remaining data until EOF.
		boost::asio::async_read(socket_, response_,
			boost::asio::transfer_at_least(1),
			boost::bind(&CRCSocket::handle_read_response, this,
				boost::asio::placeholders::error));
	}
	else if (err != boost::asio::error::eof)
	{
		std::cout << "Error: " << err << "\n";
	}
}

int CRCSocket::Connect()
{
	string context = "CRCSocket::Connect()";
	LOG_INFO(requestID, context, "Start");

	resolver.async_resolve(query,
		boost::bind(&CRCSocket::handle_resolve, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));

	boost::shared_ptr<boost::asio::io_service::work> work(new boost::asio::io_service::work(io_service));
	boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
	t.detach();

	return;
	// Attempt to connect to server
	if (OnceClosed) {
		LOG_INFO(requestID, context, "Socket was once closed, need to run Init()...");
		Init();
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
		LOG_ERROR__("cResult == SOCKET_ERROR: WSAGetLastError returned %d, %s", error, str);
	}
	//int errorCode = WSAGetLastError();
	LOG_INFO(requestID, context, (boost::format("End: return %1%") % cResult).str().c_str());
	return cResult;
}

int CRCSocket::Read()
{	
	string context = "CRCSocket::Read()";
	if (ReadLogEnabled) LOG_INFO(requestID, context, "Start");
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
	int recev;
	unsigned int offset, length;
	char * OutBuff = nullptr;

	try {
		szIncoming = new char[TXRXBUFSIZE];
		ZeroMemory(szIncoming, sizeof(szIncoming));
		recev = recv(Socket, client->buff + client->offset, TXRXBUFSIZE, 0);

		if (ReadLogEnabled) LOG_INFO(requestID, context, (boost::format("recv returned %1% bytes of data") % recev).str().c_str());

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
				OutBuff = new char[length];              // Выделим память под принятую порцию // operator delete[] in method FreeMemory()
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
				int controlSum = readBufLength - sizeof(_sh) - sizeof(RPOINTS) - info_p->N*sizeof(RPOINT);
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

				if (PostDataLogEnabled)
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
				if (PostDataLogEnabled)
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
				if (PostDataLogEnabled)
				{
					LOG_INFO(requestID, context, "MSG_PTSTRK");
				}
			}
			break;
			case MSG_OBJTRK: 
			{
				int N = *((int*)((void*)PTR_D));
				RDRTRACK* pTK = (RDRTRACK*)(void*)(((char*)PTR_D) + 4);
				if (PostDataLogEnabled)
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

				if (PostDataLogEnabled)
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
				if (PostDataLogEnabled)
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
			if (PostDataLogEnabled) LOG_INFO(requestID, context, (boost::format("MSG_OBJTRK. N=%1%, track not found, creating new") % N).str().c_str());
			//создаём трек
			TRK* t1 = new TRK(info[i].numTrack);
			Tracks.push_back(t1);
			// добавим точки
			t1->InsertPoints(info + i, 1);
		}
		// уже есть
		else if (Idx >= 0 && Idx < Tracks.size())
		{
			if (PostDataLogEnabled) LOG_INFO(requestID, context, (boost::format("MSG_OBJTRK. N=%1%, track found") % N).str().c_str());
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