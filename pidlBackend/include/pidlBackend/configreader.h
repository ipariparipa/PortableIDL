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

#ifndef pidlBackend__CONFIGREADER_H
#define pidlBackend__CONFIGREADER_H

#include "config.h"

#include <string>

namespace PIDL {

    class ErrorCollector;

    class PIDL_BACKEND__CLASS ConfigReader
    {
    public:
        inline virtual ~ConfigReader() = default;

        enum class Status {
            OK,
            NotFound,
            Error
        };

        virtual Status getAsString(const std::string & name, std::string & ret, ErrorCollector & ec) = 0;

        virtual Status get(const std::string & name, std::string & ret, ErrorCollector & ec);
        virtual Status get(const std::string & name, long long & ret, ErrorCollector & ec);
        virtual Status get(const std::string & name, double & ret, ErrorCollector & ec);
        virtual Status get(const std::string & name, bool & ret, ErrorCollector & ec);
    };

}

#endif // pidlBackend__CONFIGREADER_H
