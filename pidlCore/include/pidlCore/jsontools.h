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

#include <rapidjson/document.h>

#include <vector>

namespace PIDL { namespace JSONTools {

	extern bool getValue(const rapidjson::Value & r, const char * name, rapidjson::Value *& ret);

	extern bool getValue(const rapidjson::Value & r, const char * name, std::string & ret);
	extern bool getValue(const rapidjson::Value & v, std::string & ret);

	extern bool getValue(const rapidjson::Value & r, const char * name, long long & ret);
	extern bool getValue(const rapidjson::Value & v, long long & ret);

	extern bool getValue(const rapidjson::Value & r, const char * name, double & ret);
	extern bool getValue(const rapidjson::Value & v, double & ret);

	extern bool getValue(const rapidjson::Value & r, const char * name, bool & ret);
	extern bool getValue(const rapidjson::Value & v, bool & ret);

	extern bool getValue(const rapidjson::Value & r, const char * name, tm & ret);
	extern bool getValue(const rapidjson::Value & v, tm & ret);

	extern bool getValue(const rapidjson::Value & r, const char * name, std::vector<char> & ret);
	extern bool getValue(const rapidjson::Value & v, std::vector<char> & ret);

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

	extern rapidjson::Value setString(rapidjson::Document & doc, const char * str);

	extern void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, rapidjson::Value & v);

	extern void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const char * str);
	extern rapidjson::Value createValue(rapidjson::Document & doc, const char * str);

	extern void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::string & str);
	extern rapidjson::Value createValue(rapidjson::Document & doc, const std::string & str);

	extern void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, long long num);
	extern rapidjson::Value createValue(rapidjson::Document & doc, long long num);

	extern void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, double num);
	extern rapidjson::Value createValue(rapidjson::Document & doc, double num);

	extern void addNull(rapidjson::Document & doc, rapidjson::Value & r, const char * name);
	extern rapidjson::Value createNull(rapidjson::Document & doc, rapidjson::Value & r);

	extern void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, bool b);
	extern rapidjson::Value createValue(rapidjson::Document & doc, bool b);

	extern void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const tm & t);
	extern rapidjson::Value createValue(rapidjson::Document & doc, const tm & t);

	extern void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<char> & b);
	extern rapidjson::Value createValue(rapidjson::Document & doc, const std::vector<char> & b);

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
			auto v_ = createValue(doc, _v);
			v.PushBack(v_, doc.GetAllocator());
		}

		return v;
	}

	template<typename T>
	void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<T> & values)
	{
		auto v = createValue<T>(doc, values);
		addValue(doc, r, name, v);
	}

}}

#endif // pidlCore__jsontools_h
