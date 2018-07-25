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

#ifndef INCLUDE_PIDLBACKEND_JSON_FACTORIES_CPPCODEGENFACTORY_JSON_H_
#define INCLUDE_PIDLBACKEND_JSON_FACTORIES_CPPCODEGENFACTORY_JSON_H_

#include "objectfactory_json.h"

#include "cppcodegen.h"

#include <memory>

#include <rapidjson/document.h>

namespace PIDL {

	class ErrorCollector;

	class PIDL_BACKEND__CLASS CPPCodeGenLoggingFactory_JSON : public ObjectFactory_JSON
	{
	public:
		virtual ~CPPCodeGenLoggingFactory_JSON() = default;

		virtual const char * objectType() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN_LOGGING; }

		virtual bool build(const rapidjson::Value & value, std::shared_ptr<CPPCodeGenLogging> & ret, ErrorCollector & ec) = 0;
	};

	class PIDL_BACKEND__CLASS CPPCodeGenHelperFactory_JSON : public ObjectFactory_JSON
	{
	public:
		virtual ~CPPCodeGenHelperFactory_JSON() = default;

		virtual const char * objectType() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN_HELPER; }

		virtual bool build(const rapidjson::Value & value, std::shared_ptr<CPPCodeGenHelper> & ret, ErrorCollector & ec) = 0;
	};

	class PIDL_BACKEND__CLASS CPPCodeGenFactory_JSON : public ObjectFactory_JSON
	{
	public:
		virtual ~CPPCodeGenFactory_JSON() = default;

		virtual const char * objectType() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN; }

		virtual bool build(const rapidjson::Value & value, std::shared_ptr<CPPCodeGen> & ret, ErrorCollector & ec) = 0;
	};

}


#endif /* INCLUDE_PIDLBACKEND_JSON_FACTORIES_CPPCODEGENFACTORY_JSON_H_ */
