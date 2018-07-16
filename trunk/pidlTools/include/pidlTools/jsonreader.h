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

#ifndef pidlTools__jsonparser_h
#define pidlTools__jsonparser_h

#include "reader.h"
#include <memory>
#include <vector>
#include <string>

namespace PIDL {

	class ErrorCollector;
	class Writer;

	class JSONReader : public Reader
	{
		PIDL_COPY_PROTECTOR(JSONReader)
		struct Priv;
		Priv * priv;
	public:
		JSONReader();
		virtual ~JSONReader();

		bool read(const std::string & json_stream, ErrorCollector & ec);

		virtual std::vector<std::shared_ptr<Language::TopLevel>> topLevels() const override;

		static bool compile(Writer * writer, const std::string & json_stream, std::string & ret, ErrorCollector & ec);

	};

}

#endif // pidlTools__jsonparser_h
