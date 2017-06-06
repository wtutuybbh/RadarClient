// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <iostream>


#include <cstdlib>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <codecvt>
#include <windows.h>
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


// TODO: reference additional headers your program requires here
