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

#ifndef INCLUDE_PIDLBACKEND_OBJECT_H_
#define INCLUDE_PIDLBACKEND_OBJECT_H_

#include "config.h"

#include <list>
#include <vector>
#include <memory>

namespace PIDL {

	class ErrorCollector;

	class PIDL_BACKEND__CLASS Object
	{
		PIDL_COPY_PROTECTOR(Object);
		struct Priv;
		Priv * priv;
	protected:
		Object();
	public:
		virtual ~Object();

		virtual const char * type() const = 0;
	};

	class PIDL_BACKEND__CLASS ObjectRegistry
	{
		PIDL_COPY_PROTECTOR(ObjectRegistry)
		struct Priv;
		Priv * priv;
	public:
		ObjectRegistry();
		~ObjectRegistry();

		bool add(const char * name, const std::shared_ptr<Object> & obj, ErrorCollector & ec);

		template<class Object_T>
		std::vector<std::shared_ptr<Object_T>> get(const char * type) const
		{
			auto tmp = _get(type);
			std::vector<std::shared_ptr<Object_T>> ret(tmp.size());
			size_t i = 0;
			for(auto & o : tmp)
				ret[i++] = std::dynamic_pointer_cast<Object_T>(o);
			return ret;
		}

		template<class Object_T>
		std::shared_ptr<Object_T> get(const char * type, const char * name) const
		{
			return std::dynamic_pointer_cast<Object_T>(_get(type, name));
		}

	private:
		std::vector<std::shared_ptr<Object>> _get(const char * type) const;
		std::shared_ptr<Object> _get(const char * type, const char * name) const;
	};

	class PIDL_BACKEND__CLASS ObjectFactory
	{
		PIDL_COPY_PROTECTOR(ObjectFactory)
		struct Priv;
		Priv * priv;
	protected:
		ObjectFactory();
	public:
		virtual ~ObjectFactory();

		virtual const char * objectType() const = 0;
	};

	class PIDL_BACKEND__CLASS ObjectFactoryRegistry
	{
		PIDL_COPY_PROTECTOR(ObjectFactoryRegistry)
		struct Priv;
		Priv * priv;
	public:
		ObjectFactoryRegistry();
		~ObjectFactoryRegistry();
	};

}

#endif /* INCLUDE_PIDLBACKEND_OBJECT_H_ */
