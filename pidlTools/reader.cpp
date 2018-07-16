
#include "include/pidlTools/reader.h"
#include <pidlCore/errorcollector.h>

#include <fstream>
#include <sstream>

namespace PIDL
{

	//struct Reader::Priv { };
	Reader::Reader() : priv(nullptr)
	{ }

	Reader::~Reader() = default;

	bool Reader::readFromFile(const std::string & filename, std::string & str, ErrorCollector & ec)
	{
		std::ifstream file(filename);
		std::stringstream ss;
		if (file) {
			ss << file.rdbuf();
			file.close();
		}
		else
		{
			ec << "unable to open file";
			return false;
		}

		str = ss.str();
		return true;
	}

}
