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

#ifndef pidlCore__nullable_h
#define pidlCore__nullable_h

#include "config.h"

#include <memory>

namespace PIDL {

	template<typename T>
	class AbstractNullable
	{
	public:
		virtual inline ~AbstractNullable() { }

		inline bool isNull() const { return ptr() == nullptr; }
		inline operator bool () const { return ptr() != nullptr; }

		inline T & ref() { return *ptr(); }
		inline const T & ref() const { return *ptr(); }

		inline T * operator -> () const { return ptr(); }

		inline T & operator * () { return *ptr(); }
		inline const T & operator * () const { return *ptr(); }

		virtual T * ptr() const = 0;

	};

	template <typename T>
	class Nullable : public AbstractNullable<T>
	{
	public:
		inline Nullable(const T * o) : val(o ? new T(*o) : nullptr)
		{ }

		inline Nullable(const Nullable<T> & o) : val(o.val ? new T(*o.val) : nullptr)
		{ }

		inline Nullable(const T & a) : val(new T(a))
		{ }

		inline Nullable() : val(nullptr)
		{ }

		virtual inline ~Nullable()
		{
			if(val)
				delete val;
		}

		void setNull()
		{
			if(val)
				delete val;
			val = nullptr;
		}

		T & setNotNull()
		{
			if(val)
				delete val;
			val = new T();
			return *val;
		}

		Nullable<T> & operator = (const Nullable<T> & o)
		{
			if(o.val)
			{
				if(val)
					*val = *o.val;
				else
					val = new T(*o.val);
			}
			else if(val)
			{
				delete val;
				val = nullptr;
			}
			return *this;
		}

		Nullable<T> & operator = (const T & o)
		{
			if(val)
				*val = o;
			else
				val = new T(o);
			return *this;
		}

		virtual inline T * ptr() const override { return val; }

	private:
		T * val;
	};

	template <typename T>
	class NullableConstRef : public AbstractNullable<const T>
	{
	public:
		inline NullableConstRef(const Nullable<T> & o) : val(o.ptr())
		{ }

		inline NullableConstRef(const NullableConstRef<T> & o) : val(o.val)
		{ }

		inline NullableConstRef(const T & a) : val(&a)
		{ }

		inline NullableConstRef(T & a) : val(&a)
		{ }

		inline NullableConstRef(T * a) : val(a)
		{ }

		inline NullableConstRef() : val(nullptr)
		{ }

		NullableConstRef<T> & operator = (const NullableConstRef<T> & o)
		{
			val = o.val;
			return *this;
		}

		NullableConstRef<T> & operator = (const Nullable<T> & o)
		{
			val = o.val;
			return *this;
		}

		NullableConstRef<T> & operator = (T & o)
		{
			val = &o;
			return *this;
		}

		NullableConstRef<T> & operator = (T * o)
		{
			val = o;
			return *this;
		}

		inline const T * ptr() const override { return val; }

	private:
		const T * val;
	};

}

#endif // pidlCore__nullable_h
