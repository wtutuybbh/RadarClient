#pragma once
#include "stdafx.h"


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

#define MNNAK 0.5 //коэффициент накопления от ширины ДН
//#define LENDATAOTOBR 16 //размер массива Data[LENDATAOTOBR] в структуре STROTOBR
//int LENDATAOTOBR = 1;
//int counter = 0;


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
struct STRCONSTANT
{
	float HeightDN;  //ширина ДН град
	float DeltaB;    //единица кода УСУК по В град 
	float DeltaR;    //элемент разрешения по R м
	float DeltaACP;  //дискрета по дальности АЦП
	float SpeedObj;  //макcимальная скорость объекта(м/с)
	float TimeCode;  //единица кода времени от ПЛИС(мкС)

};

//---------------------------------------------------------------------------
struct STRRAB
{
	int vidRab;  //0-работа 1-кимс на доске вн.прерывания и данные АЦП как шум
				 //2-кимс на доске без вн.прерываний и данных АЦП 3-кимс на хосте
	int Bmin;    //код УСУК
	int Bmax;    //код УСУК
	int Rmin;    //код АЦП
	int Rmax;    //код АЦП
	float SpeedB;//скорость опу (дискрета по В код УСУК/единица кода времени от ПЛИС)
	int tNakopl; // время накопления (код времени ПЛИС)
	int delBN;   // дискрета отображения (накопления) (код УСУК)

	STRCONSTANT constProg;
};
//---------------------------------------------------------------------------
union USOST
{
	unsigned int iSost;
	struct srtuctSost
	{
		int Pr : 8;
		int Per : 8;
		int Sinch : 8;
		int Afu : 8;
	}Device;
};
//---------------------------------------------------------------------------
struct STRCAPTION
{
	unsigned int regim;     // режим работы, код аварийного завершения(>100) 
							// 101-ошибка чтения из ХОСТ исходных данных
							// 102-ошибка при открытии АЦП
							// 103-ошибка инициализации АЦП
							// 0-конец работы
	USOST uSost;
	int BeginB;             //начальный азимут точек код УСУК
	int EndB;               //конечный азимут точек код УСУК
	unsigned int sizePoint; // размер массива с данными (STRPOINT)
	unsigned int sizeOtobr; // размер массива с данными (STROTOBR)
	int reserv1;
	int reserv2;
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
/*typedef struct
{
	int NumSektor;                //номер кадра
	int Btek;                     // номер элемента разрешения от 0 по азимуту
	unsigned int CountSamer;      //количество обработанных замеров 
	unsigned int CountSamerFalse; //количество пропущенных замеров 
	float Data[LENDATAOTOBR];
}STROTOBR, *PTROTOBR;*/
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
struct STRONSINCH
{
	unsigned kIsl : 1;    //команда на формирование имп излучения
	unsigned kStart : 1;  //команда на формирование имп старта АЦП 
	unsigned kInt : 1;    //команда на формирование имп прерываний

	unsigned kAtt : 1;    //команда задания кода аттенюаторов
	unsigned kAtt1 : 4;    //код установки аттенюатора 1
	unsigned kAtt2 : 4;    //код установки аттенюатора 2

						   // управление параметрами пачки
	unsigned pCount : 8;   // количество импульсов в пачке
	unsigned pVid : 8;     // период импульсов в пачке *0.1 мкС

	unsigned reserv1 : 7;    //

							 // управление ОПУ        
	unsigned kOpu : 1;    //команда задания положения ОПУ 
	unsigned oKod : 32;   //код опу
						  //unsigned KuAP:1;    //движение по азимуту направо
						  //unsigned KuAL:1;    //движение по азимуту налево
	unsigned oVa : 32;      //скорость движения по азимуту

	unsigned reserv2 : 32;    //

};
						  //---------------------------------------------------------------------------

struct STRWITHSINCH
{
	unsigned kB : 32;     //код положения антенны по азимуту
	unsigned kTime : 32;  //код времени
	unsigned kAtt1 : 4;    //код установки аттенюатора 1
	unsigned kAtt2 : 4;    //код установки аттенюатора 2

	unsigned reserv1 : 12; //

	unsigned opuL : 1;    //состояние ОПУ движение влево
	unsigned opuR : 1;    //состояние ОПУ движение вправо

	unsigned Va : 32;      //скорость движения по азимуту

};//от синхронизатора

							 //---------------------------------------------------------------------------
struct STRNACHVEKT
{
	double r0;   //дальность
	double b0;   //азимут
	double v0;   //скорость продольная
	double psi;  //угол между проекцией л.в. на сферу и вектором скорости
	double sc;   //ЭПР
};///начальный вектор цели

			 //---------------------------------------------------------------------------



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

struct RIMAGE
{
	int N;
	short d1;
	short d2;
	short D;
	int NR;
	short resv1[1];
};




struct STRPOINTS
{

	float Amp;
	int R;        //код АЦП
	int B;        //код УСУК
	int time;   //1==200mkC

};


struct Nchannel_Data
{
	int SampleCount; // количество отсчетов в канале
	int SampleSize;  // в байтах на 1 канал
	int AZMT;       // 8200 едениц = 360 градусов
	float* ch1offs{ nullptr }; // резерв
	float* ch2offs{ nullptr }; // резерв

	unsigned int type; //= 0 - олдскул записи. поля ниже не существуют.
					   //  1 - имитатор. версия 1.1 / 4байта float, 2 канала, ch1=ch2, после модуля опт фильтр /


					   //unsigned __int64 prectimer; // 1 ед = 200 мкс, время получения данных
					   //unsigned __int64 begprectimer; // 1 ед = 200 мкс, время начала сеанса
					   //int Nazm,Nelv; // штук
					   //double dAzm,dElv;// в радианах
					   //double begAzm,begElv;
					   //time_t begCTime;
					   //unsigned int middle_t;

	int strRab_Bmax;
	int strRab_Bmin;
	float strRab_constProg_DeltaACP;
	float strRab_constProg_DeltaB;
	float strRab_constProg_DeltaR;
	float strRab_constProg_HeightDN;
	float strRab_constProg_SpeedObj;
	float strRab_constProg_TimeCode;
	int strRab_delBN;
	int strRab_Rmax;
	int strRab_Rmin;
	float strRab_SpeedB;
	int strRab_tNakopl;
	int strRab_vidRab;

	char reserv[128 - 20 * 4];


};




//---------------------------------------------------------------------------
//******************обмен по сети******************************************//


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
	unsigned int offset { 0 };
	vector<send_queue_element> send_queue;  // очередь отправляемых данных
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
	int LENDATAOTOBR {1};

public:
	static const std::string requestID;

	bool Initialized {false};
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
	RDRCURRPOS *CurrentPosition { nullptr };

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

	bool ReadLogEnabled{ false };
	bool PostDataLogEnabled{ true };
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