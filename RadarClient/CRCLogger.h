#include "stdafx.h"



namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

class CRCLogger
{
private:
	static src::severity_logger<boost::log::trivial::severity_level> logger;
public:
	static void Log(std::string context, std::string msg);
	static void Init();
	static void GetDateTimeString(std::string &out);
};
