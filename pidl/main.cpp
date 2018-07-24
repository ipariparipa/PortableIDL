/*
    This file is part of pidl.

    pidl is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidl.  If not, see <http://www.gnu.org/licenses/>
 */

#include "job.h"

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>

#include <pidlCore/errorcollector.h>

using namespace PIDL;

int main(int argc, char **argv)
{
	class MyErrorCollector : public ErrorCollector
	{
	protected:
		virtual void append(long errorCode, const std::string & errorText) override
		{
			if (errorCode >= 0)
				std::cerr << "[" << errorCode << "] ";
			std::cerr << errorText << std::endl;
		}
	} ec;
		
	std::shared_ptr<std::istream> in;

	enum class Stat
	{
		None, File
	} stat = Stat::None;

	for (int i = 1; i < argc; ++i)
	{
		std::string a(argv[i]);
		switch (stat)
		{
		case Stat::None:
			if (a == "-help")
			{
				std::cout << "-help" << std::endl;
				std::cout << "-stdin" << std::endl;
				std::cout << "-file <filename>" << std::endl;
				return 0;
			}
			if (a == "-stdin")
				in = std::shared_ptr<std::istream>(&std::cin, [](void*){});
			else if (a == "-file")
				stat = Stat::File;
			else
			{
				ec << "invalid option '" + a + "'";
				return 1;
			}
			break;
		case Stat::File:
			if (!a.length())
			{
				ec << "filename is not specified";
				return 1;
			}
			in = std::make_shared<std::ifstream>(a);
			stat = Stat::None;
			break;
		}
	}

	if (stat != Stat::None)
	{
		ec << "invalid parameters. use '-help'";
		return 1;
	}

	if (!in)
	{
		ec << "input is not specified";
		return 1;
	}

	std::shared_ptr<Job> job;

	std::stringstream ss;
	ss << in->rdbuf();
	if (!Job::build(ss.str(), job, ec))
		return 1;

	if (!job->run(ec))
		return 1;

	return 0;
}
