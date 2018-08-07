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

#ifndef pidlBackend__codegencontext_h
#define pidlBackend__codegencontext_h

#include "config.h"
#include <memory>
#include <ostream>

namespace PIDL {

	class ErrorCollector;

	class PIDL_BACKEND__CLASS CodeGenContext
	{
		PIDL_COPY_PROTECTOR(CodeGenContext)
		struct Priv;
		Priv * priv;
	public:
		enum class Role
		{
			Server, Client
		};

		virtual ~CodeGenContext();

		template<typename T>
		std::ostream & operator << (const T & v) const
		{
			stream() << v;
			return stream();
		}

		std::ostream & operator * () const;

		std::ostream & stream() const;

		Role role() const;

		std::ostream & writeTabs(short code_deepness);

	protected:
		CodeGenContext(short tab_length, char tab_char, std::ostream & o, Role role);

	};

}

#endif // pidlBackend__codegencontext_h
