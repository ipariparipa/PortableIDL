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

#ifndef pidlCore__cstyledocumentationfactory_json_h
#define pidlCore__cstyledocumentationfactory_json_h

#include "objectfactory_json.h"

#include "cstyledocumentation.h"

#include <memory>

#include <rapidjson/document.h>

namespace PIDL {

	class ErrorCollector;

	class PIDL_BACKEND__CLASS CStyleDocumentationFactory_JSON : public ObjectFactory_JSON
	{
	public:
		virtual ~CStyleDocumentationFactory_JSON() = default;

		virtual const char * objectType() const override { return PIDL_OBJECT_TYPE__CSTYLE_DOCUMENTATION; }

		virtual bool build(const rapidjson::Value & value, std::shared_ptr<CStyleDocumentation> & ret, ErrorCollector & ec) = 0;
	};

}

#endif //pidlCore__cstyledocumentationfactory_json_h
