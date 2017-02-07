﻿#include "stdafx.h"

std::string make_string(boost::asio::streambuf& streambuf)
{
	return{ buffers_begin(streambuf.data()),
		buffers_end(streambuf.data()) };
}

#include "CRCSocket.h"
#include "Util.h"
#include "CSettings.h"
#include "CRCLogger.h"

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

const std::string CRCSocket::requestID = "CRCSocket";

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

CRCSocket::CRCSocket(HWND hWnd) : 
	socket_(io_service_),
	deadline_(io_service_),
	connectionTimeout(CSettings::GetInt(IntConnectionTimeout))
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

	if (!client)
	{
		client = new _client;
	}

	ErrorText = "";

	IsConnected = false;
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

void CRCSocket::connect(const std::string& host, const std::string& service, boost::posix_time::time_duration timeout)
{
	if (IsConnected) return;
	// Resolve the host name and service to a list of endpoints.
	tcp::resolver::query query(host, service);
	tcp::resolver::iterator iter = tcp::resolver(io_service_).resolve(query);

	// Set a deadline for the asynchronous operation. As a host name may
	// resolve to multiple endpoints, this function uses the composed operation
	// async_connect. The deadline applies to the entire operation, rather than
	// individual connection attempts.
	deadline_.expires_from_now(timeout);

	// Set up the variable that receives the result of the asynchronous
	// operation. The error code is set to would_block to signal that the
	// operation is incomplete. Asio guarantees that its asynchronous
	// operations will never fail with would_block, so any other value in
	// ec indicates completion.
	boost::system::error_code ec = boost::asio::error::would_block;

	// Start the asynchronous operation itself. The boost::lambda function
	// object is used as a callback and will update the ec variable when the
	// operation completes. The blocking_udp_client.cpp example shows how you
	// can use boost::bind rather than boost::lambda.
	boost::asio::async_connect(socket_, iter, boost::bind(&CRCSocket::handle_connect, this, _1, &ec));

	// Block until the asynchronous operation has completed.
	do io_service_.run_one(); while (ec == boost::asio::error::would_block);

	// Determine whether a connection was successfully established. The
	// deadline actor may have had a chance to run and close our socket, even
	// though the connect operation notionally succeeded. Therefore we must
	// check whether the socket is still open before deciding if we succeeded
	// or failed.
	if (ec || !socket_.is_open())
		throw boost::system::system_error(
			ec ? ec : boost::asio::error::operation_aborted);	
}
void CRCSocket::handle_connect(
	const boost::system::error_code& ec,
	boost::system::error_code* out_ec)
{	
	*out_ec = ec;	
	IsConnected = true;
}
void CRCSocket::read(boost::posix_time::time_duration timeout)
{
	if (!IsConnected) return;
	// Set a deadline for the asynchronous operation. Since this function uses
	// a composed operation (async_read_until), the deadline applies to the
	// entire operation, rather than individual reads from the socket.
	deadline_.expires_from_now(timeout);

	// Set up the variable that receives the result of the asynchronous
	// operation. The error code is set to would_block to signal that the
	// operation is incomplete. Asio guarantees that its asynchronous
	// operations will never fail with would_block, so any other value in
	// ec indicates completion.
	boost::system::error_code ec = boost::asio::error::would_block;
	std::size_t length = 0;

	// Start the asynchronous operation itself. The boost::lambda function
	// object is used as a callback and will update the ec variable when the
	// operation completes. The blocking_udp_client.cpp example shows how you
	// can use boost::bind rather than boost::lambda.
	boost::asio::async_read(socket_, input_buffer_, boost::asio::transfer_at_least(1), boost::bind(&CRCSocket::handle_read, this, _1, _2, &ec, &length, read_count));

	// Block until the asynchronous operation has completed.
	int do_count = 0;
	do 
	{
		do_count++;
		io_service_.run_one();
	}
	while (ec == boost::asio::error::would_block);

	LOG_INFO(requestID, "read", "read_count=%d, do_count=%d", read_count, do_count);

	if (ec)
		throw boost::system::system_error(ec);

	read_count++;
}
void CRCSocket::handle_read(
	const boost::system::error_code& ec, std::size_t length,
	boost::system::error_code* out_ec, std::size_t* out_length, long read_count)
{
	*out_ec = ec;
	*out_length = length;

	Read();
	
	if (stopReadLoop) {
		IsConnected = false;
		stopReadLoop = false;
		return;
	}

	read(boost::posix_time::seconds(connectionTimeout));
}
void CRCSocket::check_deadline()
	{
		// Check whether the deadline has passed. We compare the deadline against
		// the current time since a new asynchronous operation may have moved the
		// deadline before this actor had a chance to run.
		if (deadline_.expires_at() <= deadline_timer::traits_type::now())
		{
			// The deadline has passed. The socket is closed so that any outstanding
			// asynchronous operations are cancelled. This allows the blocked
			// connect(), read_line() or write_line() functions to return.
			boost::system::error_code ignored_ec;
			socket_.close(ignored_ec);

			// There is no longer an active deadline. The expiry is set to positive
			// infinity so that the actor takes no action until a new deadline is set.
			deadline_.expires_at(boost::posix_time::pos_infin);
		}
		//int count;
		// Put the actor back to sleep.
		deadline_.async_wait(boost::bind(&CRCSocket::check_deadline, this));
	}
void CRCSocket::Connect()
{
	string context = "CRCSocket::Connect()";
	try
	{
		LOG_INFO(requestID, context, "Start");

		connect(CSettings::GetString(StringHostName), num2str(CSettings::GetInt(IntPort), 0), boost::posix_time::seconds(connectionTimeout));

		IsConnected = true;

		stopReadLoop = false;

		LOG_INFO(requestID, context, "Connected successfully");

		boost::thread t(boost::bind(&CRCSocket::read, this, boost::posix_time::seconds(connectionTimeout)));

		t.detach();
	}
	catch (std::exception& e)
	{
		LOG_ERROR(requestID, context, "Error during connection. e.what()=%s", e.what());

	}
}

int CRCSocket::Read()
{	
	const char * input_buffer = boost::asio::buffer_cast<const char*>(input_buffer_.data());
	string context = "CRCSocket::Read()";
	//if (ReadLogEnabled) LOG_INFO(requestID, context, "Start");				

	char *szIncoming = nullptr;
	_sh *sh;
	int recev;
	unsigned int offset, length;
	char * OutBuff = nullptr;

	try {
		recev = input_buffer_.size();

		client->buff = new char[recev];

		memcpy(client->buff, input_buffer, recev);

		sh = (struct _sh*)client->buff;
		if (sh->word1 != 2863311530/* 0xAAAAAAAAu*/ || sh->word2 != 1431655765 /*0x55555555u*/)   // проверим шапку
		{
			if (client->buff)
			{
				delete[] client->buff;
				client->buff = nullptr;
			}
			if (ReadLogEnabled) LOG_WARN(requestID, context, "wrong sh! sh->word1 = %d, sh->word2 = %d", sh->word1, sh->word2);
			return recev;
		}
		else
		{
			client->offset = 0;
		}
		//return (int)s.length();

		offset = recev + client->offset;
		if (offset < sizeof(_sh)) //приняли меньше шапки
		{
			client->offset = offset; //Запомнили что что-то приняли
			if (client->buff)
			{
				delete[] client->buff;
				client->buff = nullptr;
			}
			return recev;
		}
		
		
		length = sh->dlina;
		if (ReadLogEnabled) {
			LOG_INFO(requestID, context, "0 | recev=%d, length = sh->dlina = %d, offset = %d, client->offset = %d", recev, length, offset, client->offset);
		}
		while (length < offset)  // приняли больше или ровно 1 порцию
		{
			try // защита на выделение памяти
			{
				if (length <= 0) {
					if (client->buff)
					{
						delete[] client->buff;
						client->buff = nullptr;
					}
					
					return recev;
				}
				OutBuff = new char[length];              // Выделим память под принятую порцию // operator delete[] in method FreeMemory()
				memcpy(OutBuff, client->buff, length); // Скопируем принятые данные

				PostMessage(hWnd, CM_POSTDATA, (WPARAM)OutBuff, length); //Отошлем в очередь на обработку
				memcpy(client->buff, &client->buff[length], offset - length);// Перепишем остаток в начало
				if (ReadLogEnabled) {
					LOG_INFO(requestID, context, "1 | recev=%d, length = sh->dlina = %d, offset = %d, client->offset = %d", recev, length, offset, client->offset);
				}
				offset -= length;
				length = sh->dlina;
				if (ReadLogEnabled) {
					LOG_INFO(requestID, context, "2 | recev=%d, length = sh->dlina = %d, offset = %d, client->offset = %d, sh->word1 = %d, sh->word2 = %d", recev, length, offset, client->offset, sh->word1, sh->word2);
				}
				if (sh->word1 != 2863311530 /*0xAAAAAAAAu*/ || sh->word2 != 1431655765 /*0x55555555u*/)   // проверим шапку 
				{
					client->offset = 0;
					if (ReadLogEnabled) {
						LOG_INFO(requestID, context, "3 | recev=%d, length = sh->dlina = %d, offset = %d, client->offset = %d", recev, length, offset, client->offset);
					}
					if (client->buff)
					{
						delete[] client->buff;
						client->buff = nullptr;
					}
					return recev;
				}

			}
			catch (std::bad_alloc)
			{  // ENTER THIS BLOCK ONLY IF bad_alloc IS THROWN.
			   // YOU COULD REQUEST OTHER ACTIONS BEFORE TERMINATING
				if (client->buff) 
				{
					delete[] client->buff;
					client->buff = nullptr;
				}					
				return recev;
			}
		}
	}
	catch (std::bad_alloc)
	{  // ENTER THIS BLOCK ONLY IF bad_alloc IS THROWN.
	   // YOU COULD REQUEST OTHER ACTIONS BEFORE TERMINATING
		if (client->buff)
		{
			delete[] client->buff;
			client->buff = nullptr;
		}
		return 0;
	}
	//приняли меньше чем 1 порция
	client->offset = offset; //Запомним что что-то приняли
	if (ReadLogEnabled) {
		LOG_INFO(requestID, context, "4 | recev=%d, length = sh->dlina = %d, offset = %d, client->offset = %d", recev, length, offset, client->offset);
	}
	if (client->buff)
	{
		delete[] client->buff;
		client->buff = nullptr;
	}
	return recev;
}

void CRCSocket::Close()
{	
	boost::system::error_code ignored_ec;
	socket_.close(ignored_ec);
	deadline_.expires_at(boost::posix_time::pos_infin);
	
	stopReadLoop = true;
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