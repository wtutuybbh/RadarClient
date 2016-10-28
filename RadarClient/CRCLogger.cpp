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

void CRCLogger::Init()
{
	logging::add_file_log
	(
		keywords::file_name = "sample_%N.log",
		keywords::rotation_size = 10 * 1024 * 1024,
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
		keywords::format = "[%TimeStamp%]: %Message%"
	);

	logging::core::get()->set_filter
	(
		logging::trivial::severity >= logging::trivial::info
	);

	logging::add_common_attributes();
}

void CRCLogger::Log(std::string context, std::string msg)
{
	HANDLE myConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	static DWORD cCharsWritten;
	std::stringstream s;
	std::string DateTimeString;
	GetDateTimeString(DateTimeString);
	s << context << " : " << msg << std::endl;

	std::string msgForConsole = "[" + DateTimeString + "]: " + s.str().c_str();
	WriteConsole(myConsoleHandle, &msgForConsole, msgForConsole.length(), &cCharsWritten, NULL);	

	using namespace logging::trivial;
	BOOST_LOG_SEV(logger, trace) << s.str();
}
