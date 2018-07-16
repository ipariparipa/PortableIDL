
#include "include/pidlCore/exception.h"

#include <sstream>

namespace PIDL {

	struct Exception::Priv
	{
		std::list<Error> errors;

		void updateBuffer()
		{
			std::stringstream ss;

			bool is_first = true;
			for (auto & e : errors)
			{
				if (!is_first)
					ss << std::endl;
				is_first = false;
				ss << "[" << e.first << "] " << e.second;
			}
			buff = ss.str();
		}

		std::string buff;
	};
	
	Exception::Exception(const std::list<Error> & errors) : std::exception(), priv(new Priv)
	{
		priv->errors = errors;
		priv->updateBuffer();
	}

	Exception::Exception(const Error & error) : std::exception(), priv(new Priv)
	{
		add(error);
	}

	Exception::Exception(long code, const std::string & msg) : std::exception(), priv(new Priv)
	{
		add(code, msg);
	}

	Exception::~Exception() throw()
	{
		delete priv;
	}

	const char * Exception::what() const throw()
	{
		return priv->buff.c_str();
	}

	void Exception::add(const Error & error)
	{
		priv->errors.push_back(error);
		priv->updateBuffer();
	}

	void Exception::add(long code, const std::string & msg)
	{
		add(std::make_pair(code, msg));
	}

	const std::list<Exception::Error> & Exception::errors() const
	{
		return priv->errors;
	}

}
