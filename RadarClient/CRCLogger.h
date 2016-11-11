//usage:
//
//CRCLogger::Info(requestID, context, (boost::format("%1%") % 1).str());
//CRCLogger::Warn(requestID, context, (boost::format("%1%") % 1).str());
//CRCLogger::Error(requestID, context, (boost::format("%1%") % 1).str());

#include "stdafx.h"

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
