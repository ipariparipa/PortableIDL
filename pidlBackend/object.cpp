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

#include "include/pidlBackend/object.h"

#include <map>
#include <string>

#include <pidlCore/errorcollector.h>

namespace PIDL {

	//struct Object::Priv { };
	Object::Object() : priv(nullptr) { }
	Object::~Object() = default;

	//struct ObjectFactory::Priv { }
	ObjectFactory::ObjectFactory() : priv(nullptr) { }
	ObjectFactory::~ObjectFactory() = default;

	//struct ObjectFactoryRegistry::Priv { }
	ObjectFactoryRegistry::ObjectFactoryRegistry() : priv(nullptr) { }
	ObjectFactoryRegistry::~ObjectFactoryRegistry() = default;


	struct ObjectRegistry::Priv
	{
		std::map<std::string, std::map<std::string, std::shared_ptr<Object>>> data;
	};

	ObjectRegistry::ObjectRegistry() : priv(new Priv)
	{ }

	ObjectRegistry::~ObjectRegistry()
	{
		delete priv;
	}

	bool ObjectRegistry::add(const char * name, const std::shared_ptr<Object> & obj, ErrorCollector & ec)
	{
		if(priv->data.count(obj->type()))
		{
			if(priv->data[obj->type()].count(name))
			{
				ec.add(-1, std::string() + "object '"+name+"' as type '"+obj->type()+"' has been already registered");
				return false;
			}
		}

		priv->data[obj->type()][name] = obj;

		return true;
	}

	std::vector<std::shared_ptr<Object>> ObjectRegistry::_get(const char * type) const
	{
		if(!priv->data.count(type))
			return std::vector<std::shared_ptr<Object>>();

		auto & tmp = priv->data[type];
		std::vector<std::shared_ptr<Object>> ret(tmp.size());
		size_t i = 0;
		for(auto & o : tmp)
			ret[i++] = o.second;
		return ret;
	}

	std::shared_ptr<Object> ObjectRegistry::_get(const char * type, const char * name) const
	{
		if(!priv->data.count(type))
			return std::shared_ptr<Object>();

		auto & tmp = priv->data[type];
		if(!tmp.count(name))
			return std::shared_ptr<Object>();

		return tmp[name];
	}


}
