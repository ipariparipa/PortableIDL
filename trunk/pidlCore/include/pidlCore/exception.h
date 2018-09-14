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

#ifndef pidlCore__exception_h
#define pidlCore__exception_h

#include "config.h"

#include <exception>
#include <list>
#include <string>

namespace PIDL {

	class PIDL_CORE__CLASS Exception : public std::exception
	{
		PIDL_COPY_PROTECTOR(Exception)
		struct Priv;
		Priv * priv;
	public:
		typedef std::pair<long, std::string> Error;


		Exception(const std::list<Error> & errors);
		Exception(const Error & error);
		Exception(long code, const std::string & msg);
		virtual ~Exception() throw();

		virtual const char * what() const throw();

		void add(const Error & error);
		void add(long code, const std::string & msg);

		const std::list<Error> & errors() const;

		template<class EC>
		void get(EC & ec)
		{
			for (auto & e : errors())
				ec.add(e.first, e.second);
		}
	};


	template<class Base_EC>
	class ExceptionErrorCollector : public Base_EC
	{
		std::list<Exception::Error> _errors;
	public:
		ExceptionErrorCollector() { }
		virtual ~ExceptionErrorCollector() { }

		const std::list <Exception::Error> & errors() const
		{
			return _errors;
		}

		void throwException()
		{
			throw new Exception(_errors);
		}

		virtual void clear() override
		{
			_errors.clear();
		}

	protected:
		virtual void append(long errorCode, const std::string & errorText) override
		{
			_errors.push_back(std::make_pair(errorCode, errorText));
		}
	};

}

#endif // pidlCore__exception_h
