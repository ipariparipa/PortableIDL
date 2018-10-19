/*
    This file is part of pidlBackend.

    pidlBackend is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidlBackend is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidlBackend.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef pidlBackend__config_h
#define pidlBackend__config_h

#include <pidlCore/config.h>

#if PIDL_OS == PIDL_OS_LINUX 
#  define PIDL_BACKEND__CLASS 
#  define PIDL_BACKEND__FUNCTION 
#elif PIDL_OS == PIDL_OS_WINDOWS 
#  ifdef PIDL_BACKEND__IMPL
#    define PIDL_BACKEND__CLASS __declspec(dllexport)
#    define PIDL_BACKEND__FUNCTION __declspec(dllexport)
#  else
#    define PIDL_BACKEND__CLASS __declspec(dllimport)
#    define PIDL_BACKEND__FUNCTION __declspec(dllimport)
#  endif
#endif

#define PIDL_JSON_MARSHALLING_VERSION 2

#endif // pidlBackend__config_h
