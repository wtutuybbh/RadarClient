// MockUpRadar.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BEG_AZM 0.0
#define BEG_ELV 0.0

#define R_DEVIATION 500

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <iostream>
#include <random>
#include <FreeImage.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "10001"

#define MSGBASE (0x0400 + 512)
#define MSGEND QEvent::User + 100
#define MSCAPTION QEvent::User + 101
#define MSGOTOBR QEvent::User + 102
#define MSGPOINTS QEvent::User + 103
#define MSGSTRING QEvent::User + 104

#define MSGPPOINTS QEvent::User + 105  // список подтвержденных точек
#define MSGTRACK QEvent::User + 106    // список траекторий
#define MSGDELTRACK QEvent::User + 107 // список удаленных траекторий


#define MSG_RPOINTS (MSGBASE + 150)
#define MSG_RIMAGE (MSGBASE + 151)


#define MSGPPOINTS QEvent::User + 105  // список подтвержденных точек
#define MSGTRACK QEvent::User + 106    // список траекторий
#define MSGDELTRACK QEvent::User + 107 // список удаленных траекторий

#define MSG_PTSTRK (MSGBASE + 105)  // список подтвержденных точек
#define MSG_OBJTRK (MSGBASE + 106)    // список траекторий
#define MSG_DELTRK (MSGBASE + 107) // список удаленных траекторий
#define MSG_LOCATION (MSGBASE + 161)

#define MSG_ECHO (MSGBASE + 158)

#define MSG_INIT (MSGBASE + 159)
#define MSG_ECHO (MSGBASE + 158)
#define MSG_STRING (MSGBASE + 160)

#define TXRXBUFSIZE 4*1024*1024



FIBITMAP *mockAmp;
int mockAmpWidth;
int mockAmpHeight;
int maxR;
float maxAmp = 0.0;

struct _sh                         //Шапка массивов
{
	unsigned int word1;
	unsigned int word2;
	unsigned int date;
	unsigned int times;
	unsigned int reserved;
	unsigned int type;
	unsigned int dlina;
};
union UTCtime {
	WORD ta[8];
	SYSTEMTIME st;
};
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
	char          resv2[888];
};

struct RDRTRACKS
{
	int N;
};

struct RDRTRACK
{
	int numTrack;// номер траектории
	float Amp;   // амплитуда последнего измерения
	double X;     // коорданата X м
	double Y;     // координата Y м
	double Z;     // координата Z м
	double vX;    // скорость, м/сек X
	double vY;    // Y
	double vZ;    // Z
	double time;  // время последнего подтверждения
	int countTrue;      // счётчик подтверждений
	int countFalse;      // счётчик отсутствия подтверждений
	char resv1[8];

};
struct RPOINT
{

	float Amp;
	unsigned int  R;
	short B;
	short E;
	unsigned int T;   //1==200mkC
};

struct RPOINTS
{
	int N;
	short d1;
	short d2;
	short D;
	short resv1;
	unsigned int ObserveCount;
};



RDR_INITCL GetInit();
_sh GetSh(unsigned int type, unsigned int dlina);
char *GetTrack(int numTrack, int N);
char *GetPoints(short d1, short d2, int N, unsigned int R);
int SendData(SOCKET s, char *buf, int size, int flags);
float GetAmp(unsigned int R, short B);

RDR_INITCL init;

std::default_random_engine generator;
std::normal_distribution<double> distribution(0, R_DEVIATION);

int __cdecl main(void)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(ListenSocket);

	/* START DATA TRANSFER: */

	mockAmp = FreeImage_Load(FreeImage_GetFileType("mockAmp.png", 0), "mockAmp.png");
	mockAmpWidth = FreeImage_GetWidth(mockAmp);
	mockAmpHeight = FreeImage_GetHeight(mockAmp);

	if (!mockAmp)
	{
		printf("mockAmp.png not found!");
		return -1;
	}

	_sh shapka = GetSh(MSG_INIT, sizeof(RDR_INITCL));

	init = GetInit();
	maxR = init.maxR;
	char to_send[sizeof(_sh) + sizeof(RDR_INITCL)];

	memcpy(to_send, &shapka, sizeof(_sh));
	memcpy(to_send + sizeof(_sh), &init, sizeof(RDR_INITCL));


	iSendResult = send(ClientSocket, to_send, sizeof(_sh) + sizeof(RDR_INITCL), 0);
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}
	printf("Bytes sent: %d\n", iSendResult);

	char *track = GetTrack(0, 15);
	_sh* __sh = (_sh*)track;
	SendData(ClientSocket, track, __sh->dlina, 0);
	delete[] track;

	track = GetTrack(1, 7);
	__sh = (_sh*)track;
	SendData(ClientSocket, track, __sh->dlina, 0);
	delete[] track;

	char *points;

	points = GetPoints(0, 255, 59, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(256, 511, 79, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(512, 767, 30, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(768, 1023, 39, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(1024, 1279, 92, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(1280, 1535, 73, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(1536, 1791, 50, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(1792, 2047, 88, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(2048, 2303, 59, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(2304, 2559, 92, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(2560, 2815, 44, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(2816, 3071, 95, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(3072, 3327, 118, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(3328, 3583, 77, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(3584, 3839, 52, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(3840, 4095, 67, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(4096, 4351, 54, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(4352, 4607, 78, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(4608, 4863, 4, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(4864, 5119, 57, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(5120, 5375, 88, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(5376, 5631, 96, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(5632, 5887, 61, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(5888, 6143, 79, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(6144, 6399, 36, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(6400, 6655, 24, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(6656, 6911, 68, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(6912, 7167, 68, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(7168, 7423, 53, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(7424, 7679, 56, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(7680, 7935, 78, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;
	points = GetPoints(7936, 8191, 66, 2500);	__sh = (_sh*)points;	SendData(ClientSocket, points, __sh->dlina, 0);	delete[] points;




	printf("max Amp = %f\n", maxAmp);

	// Receive until the peer shuts down the connection
	do {
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}
RDR_INITCL GetInit() {

	RDR_INITCL TI; /* TI = Test Init */

	TI.Nazm = 8092;
	TI.Nelv = 1;
	TI.dAzm = 2 * M_PI / TI.Nazm;
	TI.dElv = 0;
	TI.begAzm = BEG_AZM;
	TI.begElv = BEG_ELV;
	TI.dR = 0.9375;
	TI.NR = 8192;
	TI.minR = 60;
	TI.maxR = TI.dR*TI.NR;
	std::fill(TI.resv1, TI.resv1 + 32, 0);
	TI.ViewStep = 256;
	TI.Proto[0] = 8;
	TI.Proto[1] = 0;
	TI.ScanMode = 0;


	TI.srvTime.ta[0] = 0;
	TI.srvTime.ta[1] = 0;
	TI.srvTime.ta[2] = 0;
	TI.srvTime.ta[3] = 0;
	TI.srvTime.ta[4] = 0;
	TI.srvTime.ta[5] = 0;
	TI.srvTime.ta[6] = 0;
	TI.srvTime.ta[7] = 0;
	GetSystemTime(&(TI.srvTime.st));


	TI.MaxNumSectPt = 0;
	TI.MaxNumSectImg = 0;
	TI.blankR1 = 0;
	TI.blankR2 = 0;
	std::fill(TI.resv2, TI.resv2 + 888, 0);

	return TI;
}

char* GetTrack(int numTrack, int N)
{
	if (N<=0)
	{
		return nullptr;
	}
	RDRTRACKS	t;
	t.N = N;
	RDRTRACK *ts = new RDRTRACK[N];

	if (numTrack == 0) {				
		if (N > 0)
		{
			ts[0].numTrack = 0;
			ts[0].Amp = 255;
			ts[0].X = -1920;
			ts[0].Y = 960;
			ts[0].Z = 112.4998627;
			ts[0].vX = 0;
			ts[0].vY = 0;
			ts[0].vZ = 0;
			ts[0].time = 0;
			ts[0].countTrue = 15;
			ts[0].countFalse = 0;
		}
		if (N > 1)
		{
			ts[1].numTrack = 0;
			ts[1].Amp = 255;
			ts[1].X = -1824;
			ts[1].Y = 1248;
			ts[1].Z = 115.8256978;
			ts[1].vX = 0;
			ts[1].vY = 0;
			ts[1].vZ = 0;
			ts[1].time = 0;
			ts[1].countTrue = 14;
			ts[1].countFalse = 0;
		}
		if (N > 2)
		{
			ts[2].numTrack = 0;
			ts[2].Amp = 255;
			ts[2].X = -1632;
			ts[2].Y = 1536;
			ts[2].Z = 117.4533049;
			ts[2].vX = 0;
			ts[2].vY = 0;
			ts[2].vZ = 0;
			ts[2].time = 0;
			ts[2].countTrue = 13;
			ts[2].countFalse = 0;
		}
		if (N > 3)
		{
			ts[3].numTrack = 0;
			ts[3].Amp = 255;
			ts[3].X = -1344;
			ts[3].Y = 1728;
			ts[3].Z = 114.7277991;
			ts[3].vX = 0;
			ts[3].vY = 0;
			ts[3].vZ = 0;
			ts[3].time = 0;
			ts[3].countTrue = 12;
			ts[3].countFalse = 0;
		}
		if (N > 4)
		{
			ts[4].numTrack = 0;
			ts[4].Amp = 255;
			ts[4].X = -1056;
			ts[4].Y = 1824;
			ts[4].Z = 110.4563046;
			ts[4].vX = 0;
			ts[4].vY = 0;
			ts[4].vZ = 0;
			ts[4].time = 0;
			ts[4].countTrue = 11;
			ts[4].countFalse = 0;
		}
		if (N > 5)
		{
			ts[5].numTrack = 0;
			ts[5].Amp = 255;
			ts[5].X = -768;
			ts[5].Y = 1824;
			ts[5].Z = 103.7197486;
			ts[5].vX = 0;
			ts[5].vY = 0;
			ts[5].vZ = 0;
			ts[5].time = 0;
			ts[5].countTrue = 10;
			ts[5].countFalse = 0;
		}
		if (N > 6)
		{
			ts[6].numTrack = 0;
			ts[6].Amp = 255;
			ts[6].X = -480;
			ts[6].Y = 1728;
			ts[6].Z = 93.98957891;
			ts[6].vX = 0;
			ts[6].vY = 0;
			ts[6].vZ = 0;
			ts[6].time = 0;
			ts[6].countTrue = 9;
			ts[6].countFalse = 0;
		}
		if (N > 7)
		{
			ts[7].numTrack = 0;
			ts[7].Amp = 255;
			ts[7].X = -192;
			ts[7].Y = 1536;
			ts[7].Z = 81.12480472;
			ts[7].vX = 0;
			ts[7].vY = 0;
			ts[7].vZ = 0;
			ts[7].time = 0;
			ts[7].countTrue = 8;
			ts[7].countFalse = 0;
		}
		if (N > 8)
		{
			ts[8].numTrack = 0;
			ts[8].Amp = 255;
			ts[8].X = 192;
			ts[8].Y = 1440;
			ts[8].Z = 76.13506653;
			ts[8].vX = 0;
			ts[8].vY = 0;
			ts[8].vZ = 0;
			ts[8].time = 0;
			ts[8].countTrue = 7;
			ts[8].countFalse = 0;
		}
		if (N > 9)
		{
			ts[9].numTrack = 0;
			ts[9].Amp = 255;
			ts[9].X = 576;
			ts[9].Y = 1536;
			ts[9].Z = 85.9722744;
			ts[9].vX = 0;
			ts[9].vY = 0;
			ts[9].vZ = 0;
			ts[9].time = 0;
			ts[9].countTrue = 6;
			ts[9].countFalse = 0;
		}
		if (N > 10)
		{
			ts[10].numTrack = 0;
			ts[10].Amp = 255;
			ts[10].X = 864;
			ts[10].Y = 1632;
			ts[10].Z = 96.77604118;
			ts[10].vX = 0;
			ts[10].vY = 0;
			ts[10].vZ = 0;
			ts[10].time = 0;
			ts[10].countTrue = 5;
			ts[10].countFalse = 0;
		}
		if (N > 11)
		{
			ts[11].numTrack = 0;
			ts[11].Amp = 255;
			ts[11].X = 1248;
			ts[11].Y = 1632;
			ts[11].Z = 107.6712437;
			ts[11].vX = 0;
			ts[11].vY = 0;
			ts[11].vZ = 0;
			ts[11].time = 0;
			ts[11].countTrue = 4;
			ts[11].countFalse = 0;
		}
		if (N > 12)
		{
			ts[12].numTrack = 0;
			ts[12].Amp = 255;
			ts[12].X = 1536;
			ts[12].Y = 1440;
			ts[12].Z = 110.3416639;
			ts[12].vX = 0;
			ts[12].vY = 0;
			ts[12].vZ = 0;
			ts[12].time = 0;
			ts[12].countTrue = 3;
			ts[12].countFalse = 0;
		}
		if (N > 13)
		{
			ts[13].numTrack = 0;
			ts[13].Amp = 255;
			ts[13].X = 1920;
			ts[13].Y = 1344;
			ts[13].Z = 122.8259467;
			ts[13].vX = 0;
			ts[13].vY = 0;
			ts[13].vZ = 0;
			ts[13].time = 0;
			ts[13].countTrue = 2;
			ts[13].countFalse = 0;
		}
		if (N > 14)
		{
			ts[14].numTrack = 0;
			ts[14].Amp = 255;
			ts[14].X = 2400;
			ts[14].Y = 1248;
			ts[14].Z = 141.7676831;
			ts[14].vX = 0;
			ts[14].vY = 0;
			ts[14].vZ = 0;
			ts[14].time = 0;
			ts[14].countTrue = 1;
			ts[14].countFalse = 0;
		}		
	}
	if (numTrack==1)
	{
		if (N>0)
		{
			ts[0].numTrack = 1;
			ts[0].Amp = 255;
			ts[0].X = 1728;
			ts[0].Y = 3840;
			ts[0].Z = 220.6833278;
			ts[0].vX = 0;
			ts[0].vY = 0;
			ts[0].vZ = 0;
			ts[0].time = 0;
			ts[0].countTrue = 15;
			ts[0].countFalse = 0;
		}
		if (N>1)
		{
			ts[1].numTrack = 1;
			ts[1].Amp = 255;
			ts[1].X = 1824;
			ts[1].Y = 3648;
			ts[1].Z = 213.7497392;
			ts[1].vX = 0;
			ts[1].vY = 0;
			ts[1].vZ = 0;
			ts[1].time = 0;
			ts[1].countTrue = 14;
			ts[1].countFalse = 0;
		}
		if (N>2)
		{
			ts[2].numTrack = 1;
			ts[2].Amp = 255;
			ts[2].X = 1920;
			ts[2].Y = 3456;
			ts[2].Z = 207.195307;
			ts[2].vX = 0;
			ts[2].vY = 0;
			ts[2].vZ = 0;
			ts[2].time = 0;
			ts[2].countTrue = 13;
			ts[2].countFalse = 0;
		}
		if (N>3)
		{
			ts[3].numTrack = 1;
			ts[3].Amp = 255;
			ts[3].X = 2112;
			ts[3].Y = 3360;
			ts[3].Z = 207.9878769;
			ts[3].vX = 0;
			ts[3].vY = 0;
			ts[3].vZ = 0;
			ts[3].time = 0;
			ts[3].countTrue = 12;
			ts[3].countFalse = 0;
		}
		if (N>4)
		{
			ts[4].numTrack = 1;
			ts[4].Amp = 255;
			ts[4].X = 2400;
			ts[4].Y = 3264;
			ts[4].Z = 212.3239329;
			ts[4].vX = 0;
			ts[4].vY = 0;
			ts[4].vZ = 0;
			ts[4].time = 0;
			ts[4].countTrue = 11;
			ts[4].countFalse = 0;
		}
		if (N>5)
		{
			ts[5].numTrack = 1;
			ts[5].Amp = 255;
			ts[5].X = 2880;
			ts[5].Y = 3168;
			ts[5].Z = 224.3801231;
			ts[5].vX = 0;
			ts[5].vY = 0;
			ts[5].vZ = 0;
			ts[5].time = 0;
			ts[5].countTrue = 10;
			ts[5].countFalse = 0;
		}
		if (N>6)
		{
			ts[6].numTrack = 1;
			ts[6].Amp = 255;
			ts[6].X = 3648;
			ts[6].Y = 3072;
			ts[6].Z = 249.9421884;
			ts[6].vX = 0;
			ts[6].vY = 0;
			ts[6].vZ = 0;
			ts[6].time = 0;
			ts[6].countTrue = 9;
			ts[6].countFalse = 0;
		}
	}
	char *ret = new char[sizeof(_sh) + sizeof(RDRTRACKS) + N * sizeof(RDRTRACK)];

	_sh shapka = GetSh(MSG_OBJTRK, sizeof(RDRTRACKS) + N * sizeof(RDRTRACK));

	memcpy(ret, &shapka, sizeof(_sh));
	memcpy(ret + sizeof(_sh), &t, sizeof(RDRTRACKS));
	memcpy(ret + sizeof(_sh) + sizeof(RDRTRACKS), ts, N * sizeof(RDRTRACK));

	return ret;	
}

char* GetPoints(short d1, short d2, int N, unsigned int R)
{
	if (N <= 0)
	{
		return nullptr;
	}

	_sh shapka = GetSh(MSG_RPOINTS, sizeof(RPOINTS) + N * sizeof(RPOINT));

	

	RPOINTS p;

	p.N = N;
	p.d1 = d1;
	p.d2 = d2;
	p.D = BEG_ELV;
	p.ObserveCount = 1;
	p.resv1 = 0;

	RPOINT * ps = new RPOINT[N];

	for (int i=0; i<N; i++)
	{		 

		
		ps[i].B = d1 + 0 + (rand() % (int)(255 - 0 + 1));

		float a = 2 * M_PI * ps[i].B / init.Nazm;

		ps[i].E = BEG_ELV;
		ps[i].R = R * (2 + cos(4 * a) / 4 + 0.1*sin(5 * a)) + distribution(generator);
		ps[i].T = 0;
		ps[i].Amp = GetAmp(ps[i].R, ps[i].B);
	}

	char *ret = new char[sizeof(_sh) + sizeof(RDRTRACKS) + N * sizeof(RDRTRACK)];

	memcpy(ret, &shapka, sizeof(_sh));
	memcpy(ret + sizeof(_sh), &p, sizeof(RPOINTS));
	memcpy(ret + sizeof(_sh) + sizeof(RPOINTS), ps, N * sizeof(RPOINT));

	return ret;
}

int SendData(SOCKET s, char* buf, int size, int flags)
{
	int iSendResult = send(s, buf, size, flags);
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(s);
		WSACleanup();
	}
	else 
	{
		printf("Bytes sent: %d\n", iSendResult);
	}
	return iSendResult;
}

float GetAmp(unsigned int R, short B)
{
	float a = 2 * M_PI * B / init.Nazm;
	int x = mockAmpWidth / 2 + (mockAmpWidth / 2) * (R * sin(a) / init.maxR);
	int y = mockAmpHeight / 2 + (mockAmpHeight / 2) * (R * cos(a) / init.maxR);
	
	RGBQUAD pixelcolor;


	FreeImage_GetPixelColor(mockAmp, x, y, &pixelcolor);
	//0.21 R + 0.72 G + 0.07 B
	float amp =  0.21 * pixelcolor.rgbRed + 0.72 * pixelcolor.rgbGreen + 0.07 * pixelcolor.rgbBlue;
	if (amp > maxAmp)
	{
		maxAmp = amp;
	}
	return amp;
}

_sh GetSh(unsigned type, unsigned datalength)
{
	_sh shapka;

	shapka.word1 = 0xAAAAAAAA;
	shapka.word2 = 0x55555555;
	shapka.date = 0;
	shapka.times = 0;
	shapka.reserved = 0;
	shapka.type = type;
	shapka.dlina = sizeof(_sh) + datalength;

	return shapka;
}
