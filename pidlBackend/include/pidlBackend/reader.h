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

#ifndef pidlBackend__reader_h
#define pidlBackend__reader_h

#include "config.h"
#include <memory>
#include <vector>
#include <string>

#include "object.h"

namespace PIDL
{

	namespace Language {
		class TopLevel;
	}

	class ErrorCollector;

#define PIDL_OBJECT_TYPE__READER "reader"

	class PIDL_BACKEND__CLASS Reader : public Object
	{
		PIDL_COPY_PROTECTOR(Reader)
		struct Priv;
		Priv * priv;
	public:
		Reader();
		virtual ~Reader();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__READER; }

		virtual bool read(ErrorCollector & ec) = 0;

		virtual std::vector<std::shared_ptr<Language::TopLevel>> topLevels() const = 0;

		static bool readFromFile(const std::string & filename, std::string & str, ErrorCollector & ec);

	protected:
		bool completeInfo(ErrorCollector & ec);
	};

}

#endif // pidlBackend__reader_h
