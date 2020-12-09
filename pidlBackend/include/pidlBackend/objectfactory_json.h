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

#ifndef INCLUDE_PIDLBACKEND_JSON_OBJECTFACTORY_H_
#define INCLUDE_PIDLBACKEND_JSON_OBJECTFACTORY_H_

#include "object.h"

#include <rapidjson/document.h>

#include <memory>
#include <list>
#include <vector>

namespace PIDL {

	class ErrorCollector;
    class ConfigReader;

	class PIDL_BACKEND__CLASS ObjectFactory_JSON : public ObjectFactory
	{
	public:
		virtual ~ObjectFactory_JSON() = default;

		virtual bool isValid(const rapidjson::Value & value) const = 0;

	};

	class PIDL_BACKEND__CLASS ObjectFactoryRegistry_JSON
	{
		PIDL_COPY_PROTECTOR(ObjectFactoryRegistry_JSON)
		struct Priv;
		Priv * priv;
	public:
		ObjectFactoryRegistry_JSON();
		virtual ~ObjectFactoryRegistry_JSON();

		void add(const std::shared_ptr<ObjectFactory_JSON> & fac);

		template<class FAC_T>
		std::vector<std::shared_ptr<FAC_T>> get(const char * type) const
		{
			auto tmp = _get(type);
			std::vector<std::shared_ptr<FAC_T>> ret(tmp.size());
			size_t i = 0;
			for(auto & f : tmp)
				ret[i++] = std::dynamic_pointer_cast<FAC_T>(f);

			return ret;
		}

		template<class FAC_T>
		std::shared_ptr<FAC_T> getValid(const char * type, const rapidjson::Value & root) const
		{
			return std::dynamic_pointer_cast<FAC_T>(_getValid(type, root));
		}

        static std::shared_ptr<ObjectFactoryRegistry_JSON> build(const std::shared_ptr<ObjectRegistry> & objreg, const std::shared_ptr<ConfigReader> & cr);

	private:
		const std::list<std::shared_ptr<ObjectFactory_JSON>> & _get(const char * type) const;
		std::shared_ptr<ObjectFactory_JSON> _getValid(const char * type, const rapidjson::Value & root) const;

	};

}


#endif /* INCLUDE_PIDLBACKEND_JSON_OBJECTFACTORY_H_ */
