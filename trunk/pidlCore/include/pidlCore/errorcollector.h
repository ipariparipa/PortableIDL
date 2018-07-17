/*
    This file is part of pidlCore.

    pidlCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidlCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidlCore.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef pidlCore__errorcollector_h
#define pidlCore__errorcollector_h

#include "config.h"
#include <memory>
#include <string>

namespace PIDL
{
	class PIDL_CORE__CLASS ErrorCollector
	{
		PIDL_COPY_PROTECTOR(ErrorCollector)
		struct Priv;
		Priv * priv;
	public:
		ErrorCollector();
		virtual ~ErrorCollector();
		ErrorCollector & operator << (const std::string & msg);
		std::string add(long core, const std::string & msg);
		static std::string toString(long errorCode, const std::string & errorText);
	protected:
		virtual void append(long errorCode, const std::string & errorText) = 0;
	};
}

#endif // pidlCore__errorcollector_h
