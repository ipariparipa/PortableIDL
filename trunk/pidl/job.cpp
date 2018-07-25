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

#include "job.h"

#include <pidlCore/errorcollector.h>
#include <pidlCore/jsontools.h>

#include <pidlBackend/jsonreader.h>
#include <pidlBackend/jsonwriter.h>
#include <pidlBackend/xmlreader.h>
#include <pidlBackend/cppwriter.h>
#include <pidlBackend/cswriter.h>
#include <pidlBackend/json_stl_codegen.h>
#include <pidlBackend/json_cscodegen.h>
#include <pidlBackend/language.h>

#include <pidlBackend/operationfactory_json.h>

#include <rapidjson/document.h>

#include <map>
#include <iostream>
#include <fstream>

namespace PIDL {

	struct Job::Priv
	{
		Priv(std::shared_ptr<Operation> op_) : op(op_)
		{
			facreg = ObjectFactoryRegistry_JSON::build(std::shared_ptr<ObjectRegistry>(&objreg, [](void*) {}));
		}

		std::shared_ptr<Operation> op;

		std::shared_ptr<ObjectFactoryRegistry_JSON> facreg;
		ObjectRegistry objreg;

		bool build(const rapidjson::Value & root, ErrorCollector & ec)
		{
			auto fac = facreg->getValid<OperationFactory_JSON>(PIDL_OBJECT_TYPE__OPERATION, root);
			if (!fac)
				return false;
			return fac->build(root, op, ec);
		}
	};

	Job::Job() : priv(new Priv(std::shared_ptr<Operation>()))
	{ }

	Job::Job(const std::shared_ptr<Operation> & op) : priv(new Priv(op))
	{ }

	Job::~Job()
	{
		delete priv;
	}

	bool Job::run(ErrorCollector & ec)
	{
		if (!priv->op)
		{
			ec << "no operation is specified";
			return false;
		}
		return priv->op->run(ec);
	}

	//static
	bool Job::build(const std::string & json_data, std::shared_ptr<Job> & ret, ErrorCollector & ec)
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
	bool Job::build(const rapidjson::Value & root, std::shared_ptr<Job> & ret, ErrorCollector & ec)
	{
		ret = std::shared_ptr<Job>(new Job());
		return ret->priv->build(root, ec);

		return true;
	}

}
