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

#ifndef pidlBackend__writer_h
#define pidlBackend__writer_h

#include "config.h"
#include <memory>

namespace PIDL
{

	class ErrorCollector;
	class Reader;

	class PIDL_BACKEND__CLASS Writer
	{
		PIDL_COPY_PROTECTOR(Writer)
		struct Priv;
		Priv * priv;
	public:
		Writer();
		virtual ~Writer();
		virtual bool write(Reader * reader, ErrorCollector & ec) = 0;
	};

}

#endif // pidlBackend__writer_h
