/*
    This file is part of pidlCore.

    pidlCore is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidlCore is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidlCore.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef pidlCore__jsontools_h
#define pidlCore__jsontools_h

#include "config.h"
#include "nullable.h"
#include "datetime.h"

#include <rapidjson/document.h>

#include <vector>

namespace PIDL { namespace JSONTools {

	extern PIDL_CORE__FUNCTION std::string getErrorText(rapidjson::ParseErrorCode code);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, rapidjson::Value *& ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, std::string & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, int & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, long long & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, double & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, bool & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, tm & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, DateTime & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, std::vector<char> & ret);


	template<typename T>
	bool getValue(const rapidjson::Value & r, const char * name, T & ret)
	{
		rapidjson::Value * v;
		if (!getValue(r, name, v))
			return false;
		return getValue(*v, ret);
	}

	template <typename T>
	bool getValue(const rapidjson::Value & v, std::vector<T> & ret)
	{
		if (!v.IsArray())
			return false;
		ret.resize(v.Size());

		size_t i(0);
		for (auto it = v.Begin(); it != v.End(); ++it)
			if (!getValue(*it, ret[i++]))
				return false;

		return true;
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value setString(rapidjson::Document & doc, const char * str);

	extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, rapidjson::Value & v);

	extern PIDL_CORE__FUNCTION void addNull(rapidjson::Document & doc, rapidjson::Value & r, const char * name);
	extern PIDL_CORE__FUNCTION rapidjson::Value createNull(rapidjson::Document & doc, rapidjson::Value & r);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const char * str);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const std::string & str);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, int num);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, long long num);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, double num);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, bool b);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const tm & t);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const DateTime & t);

	template<typename T>
	void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const T & v)
	{
		auto tmp = createValue(doc, v);
		addValue(doc, r, name, tmp);
	}

	extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<char> & b);
	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const std::vector<char> & b);

	template<typename T>
	void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const NullableConstRef<T> & v);

	template<typename T>
	void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<T> & values);

	template<typename T>
	void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const Nullable<T> & v)
	{
		if (v.isNull())
			addNull(doc, r, name);
		else
			addValue(doc, r, name, *v);
	}

	template<typename T>
	void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const NullableConstRef<T> & v)
	{
		if (v.isNull())
			addNull(doc, r, name);
		else
			addValue(doc, r, name, *v);
	}

	template<typename T>
	rapidjson::Value createValue(rapidjson::Document & doc, const std::vector<T> & values)
	{
		rapidjson::Value v(rapidjson::kArrayType);
		for (auto & _v : values)
		{
			auto tmp = createValue(doc, _v);
			v.PushBack(tmp, doc.GetAllocator());
		}

		return v;
	}

	template<typename T>
	void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<T> & values)
	{
		auto tmp = createValue<T>(doc, values);
		addValue(doc, r, name, tmp);
	}

}}

#endif // pidlCore__jsontools_h
