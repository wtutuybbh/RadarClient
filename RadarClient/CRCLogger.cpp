#include "stdafx.h"
#include "CRCLogger.h"

void CRCLogger::GetDateTimeString(std::string& out)
{
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[80];
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "%d-%m-%Y %I:%M:%S", timeinfo);
	std::string tmp(buffer);
	out.append(tmp);
}

void CRCLogger::Info(std::string context, std::string msg)
{
	std::stringstream s;
	s << context << " : " << msg;
	INFO << s.str();
}

void CRCLogger::Warn(std::string context, std::string msg)
{
	std::stringstream s;
	s << context << " : " << msg;
	WARN << s.str();
}

void CRCLogger::Error(std::string context, std::string msg)
{
	std::stringstream s;
	s << context << " : " << msg;
	ERROR << s.str();
}
namespace attrs = boost::log::attributes;
namespace expr = boost::log::expressions;
namespace logging = boost::log;

//Defines a global logger initialization routine
BOOST_LOG_GLOBAL_LOGGER_INIT(my_logger, logger_t)
{
	logger_t lg;

	logging::add_common_attributes();

	logging::add_file_log(
		boost::log::keywords::file_name = SYS_LOGFILE,
		boost::log::keywords::format = (
			expr::stream << expr::format_date_time<     boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
			<< " [" << expr::attr<     boost::log::trivial::severity_level >("Severity") << "]: "
			<< expr::smessage
			)
	);

	logging::add_console_log(
		std::cout,
		boost::log::keywords::format = (
			expr::stream << expr::format_date_time<     boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
			<< " [" << expr::attr<     boost::log::trivial::severity_level >("Severity") << "]: "
			<< expr::smessage
			)
	);

	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::info
	);

	return lg;
}