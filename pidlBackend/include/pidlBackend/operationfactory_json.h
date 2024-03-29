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

#ifndef INCLUDE_PIDLBACKEND_OPERATIONFACTORY_JSON_H_
#define INCLUDE_PIDLBACKEND_OPERATIONFACTORY_JSON_H_

#include "objectfactory_json.h"

#include "operation.h"

#include <memory>

#include <rapidjson/document.h>

namespace PIDL {

	class ErrorCollector;
    class ConfigReader;

	class PIDL_BACKEND__CLASS OperationFactory_JSON  : public ObjectFactory_JSON
	{
	public:
        virtual inline ~OperationFactory_JSON() override = default;

		virtual const char * objectType() const override { return PIDL_OBJECT_TYPE__OPERATION; }

        virtual bool build(const rapidjson::Value & value, std::shared_ptr<Operation> & ret, ErrorCollector & ec) = 0;
	};
}

#endif /* INCLUDE_PIDLBACKEND_OPERATIONFACTORY_JSON_H_ */
