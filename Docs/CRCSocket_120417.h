#pragma once
#include "stdafx.h"

/******************************************************************************

  ОБЩИЕ СТРУКТУРЫ ДАННЫХ ДЛЯ ОБМЕНА МЕЖДУ КЛИЕНТОМ И СЕРВЕРОМ
  
  ВЕРСИЯ 12.04.17
  
  описание протокола
  файл [протокол8.docx]  - версия 1.8 от 11.11.2016

******************************************************************************/



#ifndef global1h
#define global1h

#define WM_SOCKET 104

#define MPI               3.14159265358979323846   /* Pi               */
#define MPI2             1.57079632679489661923   /* Pi/2     */
#define M_PI_180      0.01745329251994329      /* Pi/180   */



//#pragma comment(lib,"ws2_32")

#include "Util.h"

using namespace std;

#define CM_POSTDATA (WM_USER + 2)
#define CM_SRVDATA (WM_USER + 3)
#define CM_SENDDATA (WM_USER + 4)
#define CM_REWINDFILE (WM_USER + 5)
#define CM_CONNECT (WM_USER + 6)

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define D2R(x) ((x)*M_PI/180.0)
#define R2D(x) ((x)*180.0/M_PI)



#define MSGBASE (0x0400 + 512)

#define MSG_RPOINTS (MSGBASE + 150)
#define MSG_RIMAGE (MSGBASE + 151)

#define MSG_PTSTRK (MSGBASE + 105)  // список подтвержденных точек
#define MSG_OBJTRK (MSGBASE + 106)    // список траекторий
#define MSG_DELTRK (MSGBASE + 107) // список удаленных траекторий
#define MSG_LOCATION (MSGBASE + 161)

#define MSG_ECHO (MSGBASE + 158)

#define MSG_INIT (MSGBASE + 159)
#define MSG_ECHO (MSGBASE + 158)
#define MSG_STRING (MSGBASE + 160)

#define TXRXBUFSIZE 64*(1024*1024) + 512 // макс размер блока данных при обмене по сети

// точки траектории

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

//---------------------------------------------------------------------------
struct STRPOINT
{
	float Amp;
	int R;  //код АЦП
	int B;  //код УСУК
	int time;//1==200mkC
};
//---------------------------------------------------------------------------
struct STRTRACK
{
	int numTrack;// номер траектории
	float Amp;   // амплитуда последнего измерения
	float X;     // координата X м
	float Y;     // координата Y м
	float vX;    // скорость по Х м/сек
	float vY;    // скорость по Y м/сек
	float time;  // время последнего подтверждения сек
	int countTrue;      // счетчик подтверждений
	int countFalse;      // счетчик отсутствия подтверждений

};

//---------------------------------------------------------------------------

struct RPOINT  // ОТМЕТКА ОБЪЕКТА (после первичной обработки)
{

	float Amp;
	unsigned int  R;
	short B;
	short E;
	unsigned int T;   // еденице соответствует интервал времени  равный 1 микросекунде
};

struct RPOINTS  // ОБЩАЯ СТРУКТУРА НА НАБОР ОТМЕТОК. ЗА НЕЙ СЛЕДОМ ИДУТ RPOINTS[n]
{
	int N;
	short d1;
	short d2;
	short D;
	short resv1;
	unsigned int ObserveCount;
};

struct RIMAGE  // РАДИОИЗОБРАЖЕНИЕ
{
	int N;        // количество строчек РЛИ в дискрете радиоизображения
	short d1;     // Начало дискрета отображения по B или E 
	short d2;     // Конец дискрета отображения по B или E
	short D;      // Другой угол, который не меняется
 	int NR;       // Длина сканов задана в NR
                  // uint16 scan[N][NR],   uint16  scan[i][0] - положение по B, 
				  // ugol= INIT.begAzm+B*INIT.dAzm,[рад]
                  // Или угол места соотв.
	short resv1[1];
};


//******************обмен по сети******************************************//

struct _sh  //  ЗАГОЛОВОК ВСЕХ ПАКЕТОВ ДАННЫХ ПЕРЕДАВАЕМЫХ ПО СЕТИ 
{
	unsigned int word1;
	unsigned int word2;
	unsigned int date;
	unsigned int times;
	unsigned int reserved;
	unsigned int type;
	unsigned int dlina;
};


/* очередь отправляемых данных
подразумевается 1 вызов SendArray => отправка 1 порции данных => 1 элемент очереди
*/

struct send_queue_element
{
	char* data{ nullptr }; // массив + заголовок точнее то что от них осталось отправить
	int length; // размер
};

struct _client                     //Клиенты
{
	char *buff{ nullptr };
	SOCKET *Socket{ nullptr };
	unsigned int recv_offset{ 0 };
	vector<send_queue_element> send_queue;  // очередь отправляемых данных
//#ifdef _DEBUG
	unsigned long read_number{ 0 };
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


struct RDRCURRPOS
{
	double northdir;  // направление на сервер в радианах относительно устройства
	char currstate; // 0-не получены коорд, 1-2D, 2-3D
	char resv1[7];
	double lon; // широта
	double lat; // долгота
	double elv; // высота над уровнем моря
	double direction; // угол
	UTCtime       srvTime;
	double E0;
	char resv2[128 - 7 * 8 - 16];
};

class TRK
{
public:
	int id; // id 
	bool Found;
	vector<RDRTRACK *> P; // точки трека
	TRK(int _id);
	void InsertPoints(RDRTRACK* pt, int N);
	~TRK();
};

class CRCSocket
{


	bool OnceClosed;
	char *hole{ nullptr };
	int LENDATAOTOBR{ 1 };
	bool test_sh(struct _sh *ptr);
public:
	static const std::string requestID;

	bool Initialized{ false };
	bool PointOK, TrackOK, ImageOK;
	vector<TRK*> Tracks;
	vector<TRK*> trak;
	float MMAXP;
	int IDX, NumViewSct;
	//used when receiving data:
	std::string ErrorText;
	WSADATA WsaDat;
	SOCKET Socket;
	struct hostent *host{ nullptr };
	SOCKADDR_IN SockAddr;
	HWND hWnd;
	_client *client{ nullptr };

	//used when processing data:
	RDR_INITCL* s_rdrinit{ nullptr };
	RPOINTS* info_p{ nullptr };
	RPOINT* pts{ nullptr };
	RDRCURRPOS *CurrentPosition{ nullptr };

	RIMAGE* info_i{ nullptr };
	void * pixels{ nullptr };

	long Buf[TXRXBUFSIZE];

	void* PTR_D{ nullptr };

	int n_view_in_scan;
	int ActiveSeans;
	int CurrScan;
	int scan_w;

	float Max_Amp, Min_Amp;
	int b1, b2;

	CRCSocket(HWND hWnd);
	~CRCSocket();

	bool IsConnected;

	void Init();
	int Connect();

	
	int Read();

	int Close();
	unsigned int PostData(WPARAM wParam, LPARAM lParam);
	void OnSrvMsg_RDRTRACK(RDRTRACK* info, int N);
	void OnSrvMsg_LOCATION(RDRCURRPOS* d);
	void OnSrvMsg_INIT(RDR_INITCL* s_rdrinit);
	void DoInit(RDR_INITCL* init);
	unsigned int _IMG_MapAmp2ColorRGB255(float Amp, float Min);
	unsigned int MapAmp2ColorRGB255(float Amp, float Min);
	int FindTrack(int id);
	void FreeMemory(char *ptr) const;

#ifdef _DEBUG
	DebugWindowInfo *dwi{ nullptr };
#endif
};


#endif // global1h