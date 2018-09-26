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

#include "include/pidlBackend/reader.h"
#include <pidlCore/errorcollector.h>

#include <fstream>
#include <sstream>

namespace PIDL
{

	//struct Reader::Priv { };
	Reader::Reader() : priv(nullptr)
	{ }

	Reader::~Reader() = default;

	//static
	bool Reader::readFromFile(const std::string & filename, std::string & str, ErrorCollector & ec)
	{
		std::ifstream file(filename);
		std::stringstream ss;
		if (file) {
			ss << file.rdbuf();
			file.close();
		}
		else
		{
			ec << "unable to open file";
			return false;
		}

		str = ss.str();
		return true;
	}

}
