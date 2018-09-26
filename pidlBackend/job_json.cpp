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

#include "include/pidlBackend/job_json.h"

#include <pidlCore/errorcollector.h>
#include <pidlCore/jsontools.h>

#include "include/pidlBackend/jsonreader.h"
#include "include/pidlBackend/jsonwriter.h"
#include "include/pidlBackend/xmlreader.h"
#include "include/pidlBackend/cppwriter.h"
#include "include/pidlBackend/cswriter.h"
#include "include/pidlBackend/json_stl_codegen.h"
#include "include/pidlBackend/json_cscodegen.h"
#include "include/pidlBackend/language.h"
#include "include/pidlBackend/operationfactory_json.h"

#include <rapidjson/document.h>

#include <map>
#include <iostream>
#include <fstream>

namespace PIDL {

	struct Job_JSON::Priv
	{
		Priv(std::shared_ptr<Operation> op_) : op(op_)
		{
			facreg = ObjectFactoryRegistry_JSON::build(std::shared_ptr<ObjectRegistry>(&objreg, [](void*) {}));
		}

		std::shared_ptr<Operation> op;

		std::shared_ptr<ObjectFactoryRegistry_JSON> facreg;
		ObjectRegistry objreg;

	};

	Job_JSON::Job_JSON() : priv(new Priv(std::shared_ptr<Operation>()))
	{ }

	Job_JSON::Job_JSON(const std::shared_ptr<Operation> & op) : priv(new Priv(op))
	{ }

	Job_JSON::~Job_JSON()
	{
		delete priv;
	}

	bool Job_JSON::run(ErrorCollector & ec)
	{
		if (!priv->op)
		{
			ec << "no operation is specified";
			return false;
		}
		return priv->op->run(ec);
	}

	ObjectFactoryRegistry_JSON * Job_JSON::factoryRegistry() const
	{
		return priv->facreg.get();
	}

	bool Job_JSON::build(const rapidjson::Value & root, ErrorCollector & ec)
	{
		rapidjson::Value * tmp;
		const rapidjson::Value * pidl_root = JSONTools::getValue(root, "pidl", tmp) ? tmp : &root;
		auto fac = priv->facreg->getValid<OperationFactory_JSON>(PIDL_OBJECT_TYPE__OPERATION, *pidl_root);
		if (!fac)
			return false;
		return fac->build(*pidl_root, priv->op, ec);
	}

	//static
	bool Job_JSON::build(const std::string & json_data, std::shared_ptr<Job_JSON> & ret, ErrorCollector & ec)
	{
		std::vector<char> buffer(json_data.length() + 1);
		memcpy(buffer.data(), json_data.c_str(), json_data.length());

		rapidjson::Document doc;

		if (doc.ParseInsitu(buffer.data()).HasParseError())
		{
			ec << ("JSON parse error (" + JSONTools::getErrorText(doc.GetParseError()) + ")");
			return false;
		}

		return build(doc, ret, ec);
	}

	//static
	bool Job_JSON::build(const rapidjson::Value & root, std::shared_ptr<Job_JSON> & ret, ErrorCollector & ec)
	{
		ret = std::shared_ptr<Job_JSON>(new Job_JSON());
		return ret->build(root, ec);
	}

}
