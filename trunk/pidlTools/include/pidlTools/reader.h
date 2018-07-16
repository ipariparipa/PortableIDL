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

#ifndef pidlTools__reader_h
#define pidlTools__reader_h

#include "config.h"
#include <memory>
#include <vector>
#include <string>

namespace PIDL
{

	namespace Language {
		class TopLevel;
	}

	class ErrorCollector;

	class Reader
	{
		PIDL_COPY_PROTECTOR(Reader)
		struct Priv;
		Priv * priv;
	public:
		Reader();
		virtual ~Reader();
		virtual std::vector<std::shared_ptr<Language::TopLevel>> topLevels() const = 0;

		static bool readFromFile(const std::string & filename, std::string & str, ErrorCollector & ec);
	};

}

#endif // pidlTools__reader_h