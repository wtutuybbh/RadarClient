// MockUpRadar.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define BEG_AZM 0.0
#define BEG_ELV 0.0

#define R_DEVIATION 500
#define NPOINTS_MEAN 50
#define NPOINTS_DEVIATION 25

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
#include <random>
#include <FreeImage.h>
#include <chrono>
#include <thread>
#include <fstream>

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
	int			MaxNAzm;
	int			MaxNElv;
	char          resv2[888 - 2 * sizeof(int)];
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
float GetAmp(float x, float y);

RDR_INITCL init;

std::default_random_engine generator;
std::normal_distribution<double> distribution(0, R_DEVIATION);
std::random_device rd;
std::mt19937 gen(rd());
std::normal_distribution<> d(0, R_DEVIATION);
std::normal_distribution<> d_npoints(NPOINTS_MEAN, NPOINTS_DEVIATION);

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds


int __cdecl main(void)
{
	std::ofstream outfile;
	outfile.open("GL_LINE_STRIP.csv");
	outfile << "i;change_mode;next_big_length;special_mode_id;mode_id;step_length;next_step;sign;x;dXtone;X" << std::endl;

	int H = 5;
	int W = 4;
	int N = ((W - 3) * 2 + 6 + 1)*(H - 1) - 1;
	int sign = -1, loop_length = (W - 2) * 2, step_length = 1, next_step = 0, change_mode = 1, mode_id = 0, special_mode_id = 4, next_big_length = loop_length, x = 0, x_prev = 0, dXtone = 1, X = 0;
	int x_before_change;
	// SEE GL_LINE_STRIP.xlsx for details
	for (int i = 0; i<N; i++)
	{
		x_before_change = x;

		change_mode = (int)(next_step == i);

		next_big_length = (step_length == loop_length && next_big_length == loop_length ? 3 : (step_length == 3 && next_big_length == 3 ? loop_length : next_big_length));

		special_mode_id = (mode_id == 4 && special_mode_id == 4 ? 5 : (mode_id == 5 && special_mode_id == 5 ? 4 : special_mode_id));

		mode_id = (i == 0 ? 0 : (i == N - 1 ? 6 : (change_mode == 1 ? (mode_id <= 2 ? mode_id + 1 : (mode_id == 3 ? special_mode_id : 1)) : mode_id))); // ЕСЛИ(C60 = $AO$55 - 1; 6; ЕСЛИ(T60 = 1; ЕСЛИ(V59 <= 2; V59 + 1; ЕСЛИ(V59 = 3; W60; 1)); V59)))

		step_length = (i < 2 || i == N - 1 ? 1 : (change_mode == 1 ? (step_length>1 ? 1 : next_big_length) : step_length));

		next_step = next_step + step_length * change_mode;

		sign = mode_id == 1 ? -1 * sign : sign;

		x = (mode_id == 1 ? x : (mode_id == 2 ? (x == x_prev ? x + sign : x) : (mode_id == 3 ? x + sign : x))); //=ЕСЛИ(V60=1;Y59;ЕСЛИ(V60=2;ЕСЛИ(Y59=Y58;Y59+P60;Y59);ЕСЛИ(V60=3;Y59+P60;Y59)))

		x_prev = x_before_change;

		X = (mode_id == 4 ? x + dXtone - 1 : (mode_id == 5 ? x + dXtone : x));

		outfile << i << ";" << change_mode << ";" << next_big_length << ";" << special_mode_id << ";" << mode_id << ";" << step_length << ";" << next_step << ";" << sign << ";" << x << ";" << dXtone << ";" << X << std::endl;

		dXtone ^= 1;
	}

	outfile.close();


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

	track = GetTrack(1, 8);
	__sh = (_sh*)track;
	SendData(ClientSocket, track, __sh->dlina, 0);
	delete[] track;

	char *points;

	int nspause = 3000000000 / (init.MaxNAzm / init.ViewStep);
	int d1 = 0;
	int avgR = 2500;

	while (true)
	{
		int npoints = d_npoints(gen);
		if (npoints <= 0)
		{
			npoints = 1;
		}
		points = GetPoints(d1, d1 + 255, npoints, avgR);
		__sh = (_sh*)points;	
		SendData(ClientSocket, points, __sh->dlina, 0);	
		delete[] points; 
		sleep_for(nanoseconds(nspause));
		d1 = d1 + init.ViewStep;
		if (d1 >= init.MaxNAzm)
		{
			d1 = 0;
		}
	}

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

	TI.Nazm = 256;
	TI.Nelv = 1;
	
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
	TI.MaxNAzm = 8192;
	TI.MaxNElv = 1;
	TI.dAzm = 2 * M_PI / TI.MaxNAzm;
	std::fill(TI.resv2, TI.resv2 + 888 - 2 * sizeof(int), 0);

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
		if (N > 0) { ts[0] = { 0, GetAmp((float)-3840, (float)864), -3840, 864, 1054.6480214089, 0, 0, 0, 0, 15, 0 }; }
		if (N > 1) { ts[1] = { 0, GetAmp((float)-3648, (float)960), -3648, 960, 1010.75834005054, 0, 0, 0, 0, 14, 0 }; }
		if (N > 2) { ts[2] = { 0, GetAmp((float)-3456, (float)1056), -3456, 1056, 968.297054214018, 0, 0, 0, 0, 13, 0 }; }
		if (N > 3) { ts[3] = { 0, GetAmp((float)-3264, (float)1056), -3264, 1056, 919.219299748552, 0, 0, 0, 0, 12, 0 }; }
		if (N > 4) { ts[4] = { 0, GetAmp((float)-2976, (float)1152), -2976, 1152, 855.076211754194, 0, 0, 0, 0, 11, 0 }; }
		if (N > 5) { ts[5] = { 0, GetAmp((float)-2688, (float)1344), -2688, 1344, 805.261106216666, 0, 0, 0, 0, 10, 0 }; }
		if (N > 6) { ts[6] = { 0, GetAmp((float)-2400, (float)1536), -2400, 1536, 763.504567921489, 0, 0, 0, 0, 9, 0 }; }
		if (N > 7) { ts[7] = { 0, GetAmp((float)-2016, (float)1632), -2016, 1632, 695.000497654543, 0, 0, 0, 0, 8, 0 }; }
		if (N > 8) { ts[8] = { 0, GetAmp((float)-1632, (float)1632), -1632, 1632, 618.425807363636, 0, 0, 0, 0, 7, 0 }; }
		if (N > 9) { ts[9] = { 0, GetAmp((float)-1248, (float)1632), -1248, 1632, 550.498860706955, 0, 0, 0, 0, 6, 0 }; }
		if (N > 10) { ts[10] = { 0, GetAmp((float)-864, (float)1728), -864, 1728, 517.667853996428, 0, 0, 0, 0, 5, 0 }; }
		if (N > 11) { ts[11] = { 0, GetAmp((float)-480, (float)1728), -480, 1728, 480.547584941946, 0, 0, 0, 0, 4, 0 }; }
		if (N > 12) { ts[12] = { 0, GetAmp((float)-96, (float)1728), -96, 1728, 463.730185214199, 0, 0, 0, 0, 3, 0 }; }
		if (N > 13) { ts[13] = { 0, GetAmp((float)288, (float)1632), 288, 1632, 444.049941869955, 0, 0, 0, 0, 2, 0 }; }
		if (N > 14) { ts[14] = { 0, GetAmp((float)672, (float)1536), 672, 1536, 449.235020989211, 0, 0, 0, 0, 1, 0 }; }


	}
	if (numTrack==1)
	{
		if (N > 0) { ts[0] = { 1, GetAmp((float)480, (float)3456), 480, 3456, 934.921386182057, 0, 0, 0, 0, 0, 0 }; }
		if (N > 1) { ts[1] = { 1, GetAmp((float)864, (float)3360), 864, 3360, 929.59819970451, 0, 0, 0, 0, -1, 0 }; }
		if (N > 2) { ts[2] = { 1, GetAmp((float)1152, (float)2976), 1152, 2976, 855.076211754194, 0, 0, 0, 0, -2, 0 }; }
		if (N > 3) { ts[3] = { 1, GetAmp((float)1248, (float)2400), 1248, 2400, 724.826289290089, 0, 0, 0, 0, -3, 0 }; }
		if (N > 4) { ts[4] = { 1, GetAmp((float)1344, (float)1632), 1344, 1632, 566.493009174641, 0, 0, 0, 0, -4, 0 }; }
		if (N > 5) { ts[5] = { 1, GetAmp((float)1440, (float)576), 1440, 576, 415.569761619896, 0, 0, 0, 0, -5, 0 }; }
		if (N > 6) { ts[6] = { 1, GetAmp((float)1056, (float)-384), 1056, -384, 301.081429317692, 0, 0, 0, 0, -6, 0 }; }
		if (N > 7) { ts[7] = { 1, GetAmp((float)576, (float)-1056), 576, -1056, 322.309800774972, 0, 0, 0, 0, -7, 0 }; }

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
		ps[i].R = R * (2 + cos(4 * a) / 4 + 0.1*sin(5 * a)) + d(gen);
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
float GetAmp(float x, float y)
{
	int _x = mockAmpWidth / 2 + (mockAmpWidth / 2) * (x / (init.maxR * init.dR));
	int _y = mockAmpHeight / 2 + (mockAmpHeight / 2) * (y / (init.maxR * init.dR));

	RGBQUAD pixelcolor;

	FreeImage_GetPixelColor(mockAmp, _x, _y, &pixelcolor);
	//0.21 R + 0.72 G + 0.07 B
	float amp = 0.21 * pixelcolor.rgbRed + 0.72 * pixelcolor.rgbGreen + 0.07 * pixelcolor.rgbBlue;
	if (amp > maxAmp)
	{
		maxAmp = amp;
	}
	return amp;
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
