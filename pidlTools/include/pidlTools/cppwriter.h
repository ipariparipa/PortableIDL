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

#ifndef pidlTools__cppwriter_h
#define pidlTools__cppwriter_h

#include "writer.h"
#include <string>

namespace PIDL {

	class CPPCodeGen;

	class CPPWriter : public Writer
	{
		PIDL_COPY_PROTECTOR(CPPWriter)
		struct Priv;
		Priv * priv;
	public:
		enum class Mode {Include, Source, Combo};
		enum class Role { Server, Client };

		CPPWriter(Mode mode, Role role, const std::shared_ptr<CPPCodeGen> & codegen, std::ostream & o, const std::string & name);
		virtual ~CPPWriter();
		virtual bool write(Reader * reader, ErrorCollector & ec) override;
	};

}

#endif // pidlTools__cppwriter_h
