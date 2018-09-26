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

#ifndef pidlBackend__job_h
#define pidlBackend__job_h

#include "config.h"

#include "operation.h"

#include <string>

#include <rapidjson/document.h>

namespace PIDL {

	class ErrorCollector;
	class ObjectFactoryRegistry_JSON;

	class PIDL_BACKEND__CLASS Job_JSON : public Operation
	{
		struct Priv;
		Priv * priv;

	public:
		Job_JSON();

		Job_JSON(const std::shared_ptr<Operation> & op);
		virtual ~Job_JSON();

		virtual bool run(ErrorCollector & ec) override;

		bool build(const rapidjson::Value & root, ErrorCollector & ec);

		ObjectFactoryRegistry_JSON * factoryRegistry() const;

		static bool build(const std::string & json_data, std::shared_ptr<Job_JSON> & ret, ErrorCollector & ec);
		static bool build(const rapidjson::Value & root, std::shared_ptr<Job_JSON> & ret, ErrorCollector & ec);
	};

}

#endif //pidlBackend__job_h
