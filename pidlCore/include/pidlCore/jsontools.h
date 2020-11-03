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

	namespace _internal
	{
		template<int... Is>
		struct seq { };

		template<int N, int... Is>
		struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };

		template<int... Is>
		struct gen_seq<0, Is...> : seq<Is...>{};

		template<typename T, typename F, int... Is>
		void for_each(T&& t, F f, seq<Is...>)
		{
			auto l = { (f(std::get<Is>(t)), 0)... };
            (void)l;
		}
	}

	template<typename... Ts, typename F>
	void for_each_in_tuple(std::tuple<Ts...> const& t, F f)
	{
		_internal::for_each(t, f, _internal::gen_seq<sizeof...(Ts)>());
	}

	template<typename... Ts, typename F>
	void for_each_in_tuple(std::tuple<Ts...> & t, F f)
	{
		_internal::for_each(t, f, _internal::gen_seq<sizeof...(Ts)>());
	}

	extern PIDL_CORE__FUNCTION std::string getErrorText(rapidjson::ParseErrorCode code);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & r, const char * name, rapidjson::Value *& ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, std::string & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, int & ret);

    extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, unsigned long long & ret);

    extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, long long & ret);

    extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, unsigned int & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, double & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, bool & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, tm & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, DateTime & ret);

	extern PIDL_CORE__FUNCTION bool getValue(const rapidjson::Value & v, std::vector<char> & ret);

    template <typename T>
    bool getValue(const rapidjson::Value & v, Nullable<T> & ret)
    {
        if(v.IsNull())
        {
            ret.setNull();
            return true;
        }

        ret.setNotNull();
        return getValue(v, *ret);
    }

	template <typename T>
	bool getValue(const rapidjson::Value & v, std::vector<T> & ret)
	{
		if (!v.IsArray())
			return false;
		ret.resize(v.Size());

		size_t i(0);
		bool has_error = false;
		for (auto it = v.Begin(); it != v.End(); ++it)
			if (!getValue(*it, ret[i++]))
				has_error = true;

		return !has_error;
	}

    template<typename T>
    bool getValue(const rapidjson::Value & r, const char * name, T & ret)
    {
        rapidjson::Value * v;
        if (!getValue(r, name, v))
            return false;
        return getValue(*v, ret);
    }

	namespace _internal {

		struct tuple_getValue_functor
		{
			tuple_getValue_functor(const rapidjson::Value & r_, bool & has_error_) :
				r(r_), has_error(has_error_)
			{ }

			const rapidjson::Value & r;
			bool & has_error;
			rapidjson::SizeType idx = 0;

			template<typename T>
			void operator () (T && v)
			{
				if (!getValue(r[idx++], v))
					has_error = true;
			}
		};

	}

	template<typename ...T>
	bool getValue(const rapidjson::Value & v, std::tuple<T...> & ret)
	{
		rapidjson::SizeType idx = 0;
		bool has_error = false;
		for_each_in_tuple(ret, _internal::tuple_getValue_functor(v, has_error));
		return !has_error;
	}

	extern PIDL_CORE__FUNCTION rapidjson::Value setString(rapidjson::Document & doc, const char * str);

	extern PIDL_CORE__FUNCTION void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, rapidjson::Value & v);

	extern PIDL_CORE__FUNCTION void addNull(rapidjson::Document & doc, rapidjson::Value & r, const char * name);
	extern PIDL_CORE__FUNCTION rapidjson::Value createNull(rapidjson::Document & doc, rapidjson::Value & r);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const char * str);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, const std::string & str);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, int num);

    extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, unsigned int num);

	extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, long long num);

    extern PIDL_CORE__FUNCTION rapidjson::Value createValue(rapidjson::Document & doc, unsigned long long num);

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

	namespace _internal
	{
		struct tuple_createValue_functor
		{
			tuple_createValue_functor(rapidjson::Document & doc_, rapidjson::Value & r_) : doc(doc_), r(r_)
			{ }

			rapidjson::Document & doc;
			rapidjson::Value & r;

			template<typename T>
			void operator () (T && v)
			{
				r.PushBack(createValue(doc, v), doc.GetAllocator());
			}
		};
	}

	template<typename ...T>
	rapidjson::Value createValue(rapidjson::Document & doc, const std::tuple<T...> & values)
	{
		rapidjson::Value v(rapidjson::kArrayType);
		for_each_in_tuple(values, _internal::tuple_createValue_functor(doc, v));
		return v;
	}

	template<typename ...T>
	void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::tuple<T...> & values)
	{
		auto tmp = createValue(doc, values);
		addValue(doc, r, name, tmp);
	}

}}

#endif // pidlCore__jsontools_h
