/*
    This file is part of pidl.

    pidl is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidl.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef pidl__job_h
#define pidl__job_h

#include "config.h"

#include <pidlBackend/operation.h>

#include <string>

#include <rapidjson/document.h>

namespace PIDL {

	class ErrorCollector;

	class Job : public Operation
	{
		struct Priv;
		Priv * priv;

		Job();

	public:
		Job(const std::shared_ptr<Operation> & op);
		virtual ~Job();

		virtual bool run(ErrorCollector & ec) override;
		
		static bool build(const std::string & json_data, std::shared_ptr<Job> & ret, ErrorCollector & ec);
		static bool build(const rapidjson::Value & root, std::shared_ptr<Job> & ret, ErrorCollector & ec);
	};

}

#endif //pidl__job_h
