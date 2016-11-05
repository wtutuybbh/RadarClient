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
	
	boost::shared_ptr< logging::core > core = logging::core::get();
	boost::shared_ptr< sinks::text_multifile_backend > backend =
		boost::make_shared< sinks::text_multifile_backend >();
	// Set up the file naming pattern
	backend->set_file_name_composer
	(
		sinks::file::as_file_name_composer(expr::stream << "logs/" << expr::attr< std::string >("RequestID") << ".log")
	);
	// Wrap it into the frontend and register in the core.
	// The backend requires synchronization in the frontend.
	typedef sinks::synchronous_sink< sinks::text_multifile_backend > sink_t;
	boost::shared_ptr< sink_t > sink(new sink_t(backend));

	// Set the formatter
	sink->set_formatter
	(
		expr::stream << "[RequestID: " << expr::attr<std::string>("RequestID") << "] " << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S [") << expr::attr<boost::log::trivial::severity_level>("Severity") << "]: " << expr::smessage
	);

	core->add_sink(sink);

	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::info
	);

	return lg;
}