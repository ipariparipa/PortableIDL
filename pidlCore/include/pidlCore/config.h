/*
    This file is part of pidlCore.

    pidlCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidlCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidlCore.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef pidlCore__config_h
#define pidlCore__config_h

#include "platform.h"

#define PIDL_OS_LINUX 1
#define PIDL_OS_WINDOWS 2

#ifdef PIDL_WINDOWS
#  define PIDL_OS PIDL_OS_WINDOWS
#else
#  define PIDL_OS PIDL_OS_LINUX
#endif

#if PIDL_OS == PIDL_OS_LINUX 
#  define PIDL_CORE__CLASS 
#  define PIDL_CORE__FUNCTION 
#elif PIDL_OS == PIDL_OS_WINDOWS 
#  ifdef PIDL_CORE__IMPL
#    define PIDL_CORE__CLASS __declspec(dllexport)
#    define PIDL_CORE__FUNCTION __declspec(dllexport)
#  else
#    define PIDL_CORE__CLASS __declspec(dllimport)
#    define PIDL_CORE__FUNCTION __declspec(dllimport)
#  endif
#endif


#define PIDL_COPY_PROTECTOR(class_name) class_name(const class_name &); class_name & operator = (const class_name &);

#if __cplusplus >= 201703L
#  define PIDL__HAS_OPTIONAL
#endif


#endif // pidlCore__config_h
