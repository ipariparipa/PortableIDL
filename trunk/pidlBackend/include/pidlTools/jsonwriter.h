/*
    This file is part of pidlTools.

    pidlTools is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidlTools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidlTools.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef pidlTools__jsonwriter_h
#define pidlTools__jsonwriter_h

#include "config.h"
#include "writer.h"

namespace PIDL
{
	class ErrorCollector;

	class JSONWriter : public Writer
	{
		PIDL_COPY_PROTECTOR(JSONWriter)
		struct Priv;
		Priv * priv;
	public:
		JSONWriter(std::ostream & s);
		virtual ~JSONWriter();
		virtual bool write(Reader * reader, ErrorCollector & ec);
	};

}

#endif // pidlTools__jsonwriter_h
