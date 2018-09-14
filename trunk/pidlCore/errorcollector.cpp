
#include "include/pidlCore/errorcollector.h"

#include <sstream>

namespace PIDL
{

	ErrorCollector::ErrorCollector() : priv(nullptr)
	{ }

	ErrorCollector::~ErrorCollector()
	{ }

	ErrorCollector & ErrorCollector::operator << (const std::string & msg)
	{
		add(-1, msg);
		return *this;
	}

	std::string ErrorCollector::add(long errorCode, const std::string & errorText)
	{
		append(errorCode, errorText);
		return toString(errorCode, errorText);
	}

	//virtual 
	void ErrorCollector::clear()
	{ }

	//static
	std::string ErrorCollector::toString(long errorCode, const std::string & errorText)
	{
		std::stringstream ss;
		ss << "[" << errorCode << "] " << errorText;
		return ss.str();
	}

}
