//usage:
//
//CRCLogger::Info(requestID, context, (boost::format("%1%") % 1).str());
//CRCLogger::Warn(requestID, context, (boost::format("%1%") % 1).str());
//CRCLogger::Error(requestID, context, (boost::format("%1%") % 1).str());

#include "stdafx.h"

#define CRCSocketReadLogEnabled  false
#define CRCSocketPostDataLogEnabled  false

#define CSectorRefreshLogEnabled false
#define CSector_GetPoint_LogInfo false

#define CRImageSetRefreshLogEnabled false

//(experimental method of log management)
//Errors are always logged!
#define LOG_ENABLED true
#define CRCAltitudeDataFile_v1_LOG false // constructor CRCAltitudeDataFile(const std::string& dt2FileName);
#define CRCAltitudeDataFile_v2_LOG false // constructor CRCAltitudeDataFile(double lon0, double lat0, double lon1, double lat1, int width, int height);
#define CRCAltitudeDataFile_DESTRUCTOR_LOG true // destructor info log
#define CRCAltitudeDataFile_ApplyIntersection_LOG false
#define CRCAltitudeDataFile_Open_LOG false
#define CRCAltitudeDataFile_Close_LOG false
#define CRCAltitudeDataFile_ValueAt_v1_LOG false
#define CRCAltitudeDataFile_ValueAt_v2_LOG false
#define CRCAltitudeDataFile_size_set_max_LOG false
#define CRCAltitudeDataFile_size_set_LOG false
#define CRCAltitudeDataFile_CalculateBlindZone_LOG false

#define CSettings_Change_Log true

#ifndef _DEBUG

#define LOG(severity, requestID, context, text, ...)

#define LOG_INFO(requestID, context, text, ...)
#define LOG_INFO_(context, text, ...)
#define LOG_INFO__(text, ...)

#define LOG_WARN(requestID, context, text, ...)
#define LOG_WARN_(context, text, ...)
#define LOG_WARN__(text, ...)

#define LOG_ERROR(requestID, context, text, ...)
#define LOG_ERROR_(context, text, ...)
#define LOG_ERROR__(text, ...)

#endif


#ifdef _DEBUG

#define Info_ Info
#define Warn_ Warn
#define Error_ Error

#define LOG(severity, requestID, context, text, ...) CRCLogger::##severity(requestID, context, format(text, __VA_ARGS__))

#define LOG_INFO(requestID, context, text, ...) LOG(Info_, requestID, context, text, __VA_ARGS__)
#define LOG_INFO_(context, text, ...) LOG_INFO(requestID, context, text, __VA_ARGS__)
#define LOG_INFO__(text, ...) LOG_INFO(requestID, context, text, __VA_ARGS__)

#define LOG_WARN(requestID, context, text, ...) LOG(Warn_, requestID, context, text, __VA_ARGS__)
#define LOG_WARN_(context, text, ...) LOG_WARN(requestID, context, text, __VA_ARGS__)
#define LOG_WARN__(text, ...) LOG_WARN(requestID, context, text, __VA_ARGS__)

#define LOG_ERROR(requestID, context, text, ...) LOG(Error_, requestID, context, text, __VA_ARGS__)
#define LOG_ERROR_(context, text, ...) LOG_ERROR(requestID, context, text, __VA_ARGS__)
#define LOG_ERROR__(text, ...) LOG_ERROR(requestID, context, text, __VA_ARGS__)

#define INFO  BOOST_LOG_SEV(my_logger::get(), boost::log::trivial::info)
#define WARN  BOOST_LOG_SEV(my_logger::get(), boost::log::trivial::warning)
#define ERROR_ BOOST_LOG_SEV(my_logger::get(), boost::log::trivial::error)

#define SYS_LOGFILE "example.log"

//declares a global logger with a custom initialization
BOOST_LOG_GLOBAL_LOGGER(my_logger, logger_t)


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

class CRCLogger
{
	static void Info(std::string context, std::string msg);
	static void Warn(std::string context, std::string msg);
	static void Error(std::string context, std::string msg);
	static void GetDateTimeString(std::string &out);
public:	
	static void Info(std::string requestID, std::string context, std::string msg);	
	static void Warn(std::string requestID, std::string context, std::string msg);	
	static void Error(std::string requestID, std::string context, std::string msg);	
};
#endif