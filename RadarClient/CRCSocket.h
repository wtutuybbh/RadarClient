﻿#pragma once
#ifndef global1h
#define global1h

#include <WinSock2.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <string>
#include <ctime>

#define WM_SOCKET 104

#define MPI               3.14159265358979323846   /* Pi               */
#define MPI2             1.57079632679489661923   /* Pi/2     */
#define M_PI_180      0.01745329251994329      /* Pi/180   */
#include <vector>
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
#define LENDATAOTOBR 16 //размер массива Data[LENDATAOTOBR] в структуре STROTOBR


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

#define MSG_ECHO (MSGBASE + 158)

#define MSG_INIT (MSGBASE + 159)


//---------------------------------------------------------------------------
typedef struct
{
	float HeightDN;  //ширина ДН град
	float DeltaB;    //единица кода УСУК по В град 
	float DeltaR;    //элемент разрешения по R м
	float DeltaACP;  //дискрета по дальности АЦП
	float SpeedObj;  //макcимальная скорость объекта(м/с)
	float TimeCode;  //единица кода времени от ПЛИС(мкС)

}STRCONSTANT, *PTRCONSTANT;

//---------------------------------------------------------------------------
typedef struct
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
}STRRAB, *PTRRAB;
//---------------------------------------------------------------------------
typedef union
{
	unsigned int iSost;
	struct srtuctSost
	{
		int Pr : 8;
		int Per : 8;
		int Sinch : 8;
		int Afu : 8;
	}Device;
}USOST, *PTRuSOST;
//---------------------------------------------------------------------------
typedef struct
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
}STRCAPTION, *PTRCAPTION;



//---------------------------------------------------------------------------
typedef struct
{
	float Amp;
	int R;  //код АЦП
	int B;  //код УСУК
	int time;//1==200mkC
}STRPOINT, *PTRPOINT;
//---------------------------------------------------------------------------
typedef struct
{
	int NumSektor;                //номер кадра
	int Btek;                     // номер элемента разрешения от 0 по азимуту
	unsigned int CountSamer;      //количество обработанных замеров 
	unsigned int CountSamerFalse; //количество пропущенных замеров 
	float Data[LENDATAOTOBR];
}STROTOBR, *PTROTOBR;
//---------------------------------------------------------------------------
typedef struct
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

}STRTRACK, *PTRTRACK;
//---------------------------------------------------------------------------
typedef struct
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

}STRONSINCH, *PTRONSINCH; //на синхронизатор
						  //---------------------------------------------------------------------------

typedef struct
{
	unsigned kB : 32;     //код положения антенны по азимуту
	unsigned kTime : 32;  //код времени
	unsigned kAtt1 : 4;    //код установки аттенюатора 1
	unsigned kAtt2 : 4;    //код установки аттенюатора 2

	unsigned reserv1 : 12; //

	unsigned opuL : 1;    //состояние ОПУ движение влево
	unsigned opuR : 1;    //состояние ОПУ движение вправо

	unsigned Va : 32;      //скорость движения по азимуту

}STRWITHSINCH, *PTRWITHSINCH;//от синхронизатора

							 //---------------------------------------------------------------------------
typedef struct
{
	double r0;   //дальность
	double b0;   //азимут
	double v0;   //скорость продольная
	double psi;  //угол между проекцией л.в. на сферу и вектором скорости
	double sc;   //ЭПР
}STRNACHVEKT;///начальный вектор цели

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


typedef struct tagRPOINT
{

	float Amp;
	unsigned int  R;
	short B;
	short E;
	unsigned int T;   //1==200mkC
}  RPOINT;

typedef struct tagRPOINTS
{
	int N;
	short d1;
	short d2;
	short D;
	short resv1[3];
} RPOINTS;

typedef struct tagRIMAGE
{
	int N;
	short d1;
	short d2;
	short D;
	int NR;
	short resv1[1];
} RIMAGE;




typedef struct tagSTRPOINTS
{

	float Amp;
	int R;        //код АЦП
	int B;        //код УСУК
	int time;   //1==200mkC

}  STRPOINTS, *PTRPOINTS;


typedef struct
{
	int SampleCount; // количество отсчетов в канале
	int SampleSize;  // в байтах на 1 канал
	int AZMT;       // 8200 едениц = 360 градусов
	float* ch1offs; // резерв
	float* ch2offs; // резерв

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


} Nchannel_Data;




//---------------------------------------------------------------------------
//******************обмен по сети******************************************//
//Byte hole[65535];


/* очередь отправляемых данных
подразумевается 1 вызов SendArray => отправка 1 порции данных => 1 элемент очереди
*/

typedef struct _send_queue_element
{
	char* data; // массив + заголовок точнее то что от них осталось отправить
	int length; // размер
} send_queue_element;

struct _client                     //Клиенты
{
	char *buff;
	SOCKET *Socket;
	unsigned int offset;
	vector<send_queue_element> send_queue;  // очередь отправляемых данных
};




typedef struct tagRDR_INITCL
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
	char          resv2[1024 - 112]; // offs 112
}  RDR_INITCL;











class CRCSocket
{
	bool OnceClosed;
public:
	//used when receiving data:
	std::string ErrorText;
	WSADATA WsaDat;
	SOCKET Socket;
	struct hostent *host;
	SOCKADDR_IN SockAddr;
	HWND hWnd;
	char szHistory[10000];
	_client *client;

	//used when processing data:
	char *tm;
	RDR_INITCL* s_rdrinit;
	RPOINTS* info_p;
	RPOINT* pts;

	RIMAGE* info_i;

	void* PTR_D;
	int b1, b2;
	int CurrScan;

	CRCSocket(HWND hWnd);
	~CRCSocket();

	bool IsConnected;

	void Init();
	int Connect();
	int Read();
	int Close();
	void PostData(WPARAM wParam, LPARAM lParam);

	//__event void ConnectionStateChange(bool isConnected);
};


#endif // global1h