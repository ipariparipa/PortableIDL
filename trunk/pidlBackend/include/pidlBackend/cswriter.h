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

#ifndef pidlBackend__cswriter_h
#define pidlBackend__cswriter_h

#include "writer.h"
#include <string>

namespace PIDL {

	class CSCodeGen;

	class PIDL_BACKEND__CLASS CSWriter : public Writer
	{
		PIDL_COPY_PROTECTOR(CSWriter)
		struct Priv;
		Priv * priv;
	public:
		enum class Role { Server, Client };

		CSWriter(Role role, const std::shared_ptr<CSCodeGen> & codegen, const std::shared_ptr<std::ostream> & o);
		virtual ~CSWriter();
		virtual bool write(Reader * reader, ErrorCollector & ec) override;
	};

}

#endif // pidlBackend__cswriter_h
