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
#include <tchar.h>
#include <stdlib.h>
#include <FreeImage.h>

#include <boost/filesystem.hpp>
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
//Narrow-char thread-safe logger.
typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger_t;

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

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif