#pragma once



#ifndef WINSOCK2_H_INCLUDED
#define WINSOCK2_H_INCLUDED
#include <winsock2.h>
#endif
#ifndef WINDOWS_H_INCLUDED
#define WINDOWS_H_INCLUDED
#include <windows.h>
#endif
#ifndef WINDOWSX_H_INCLUDED
#define WINDOWSX_H_INCLUDED
#include <windowsx.h>
#endif

#include <memory>
#include <stdio.h>
#include <cstdio>
#include <cstdarg>
#include <tchar.h>
#include <stdlib.h>
#include <FreeImage.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>
#include <boost/format.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/log/sinks/text_multifile_backend.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

//Narrow-char thread-safe logger.
typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger_t;

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include <iostream>
#include <ctime>

#define _USE_MATH_DEFINES 
#include <math.h>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <map>
#define GLM_FORCE_RADIANS

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/intersect.hpp"
#include "glm/detail/type_mat.hpp"
#include "glm/gtx/vector_angle.hpp"

#include <fstream>
#include <iostream>

#include <tchar.h>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include <io.h>
#include <fcntl.h>

#include <fstream>
#include <sstream>
#include <iomanip>



#include <tchar.h>

#include <GL/glew.h>
#include <GL/GL.h>



#include <ctime>

#include <thread>

#include "logging.h"

#include <codecvt>

#include <mutex>

//for CRCGeoDataProvider and CRCTextureDataFile:
#define DATFILE_MAXLINELENGTH 256

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

typedef std::basic_string<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> >  tstring;

inline std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
inline std::string wstring_to_utf8(const std::wstring& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
	return myconv.to_bytes(str);
}

#ifdef UNICODE
#define to_tstring(s) utf8_to_wstring(s)
#define to__string(s) wstring_to_utf8(s)
#else
#define to_tstring(s) s
#define to__string(s) s
#endif

#define MSG_MESH_LOADED (0x0400 + 512 + 256)