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

#include <pidlBackend/job_json.h>

#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <map>

#include <pidlCore/errorcollector.h>
#include <pidlBackend/configreader.h>

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

    struct CLAConfigReader : public ConfigReader
    {
        std::map<std::string, std::string> data;

        Status getAsString(const std::string & name, std::string & ret, ErrorCollector & ec) final override
        {
            (void)ec;
            if(!data.count(name))
                return Status::NotFound;
            ret = data[name];
            return Status::OK;
        }

    };

    auto cr = std::make_shared<CLAConfigReader>();

	enum class Stat
	{
        None, File, Config
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
                std::cout << "-cfg <varname>=<value>" << std::endl;
                return 0;
			}
			if (a == "-stdin")
				in = std::shared_ptr<std::istream>(&std::cin, [](void*){});
			else if (a == "-file")
				stat = Stat::File;
            else if (a == "-cfg")
                stat = Stat::Config;
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
        case Stat::Config:
        {
            auto f = a.find('=');
            if(f == std::string::npos)
            {
                ec.add(-1, "invalid config format");
                return 1;
            }

            cr->data[a.substr(0, f)] = a.substr(f+1);
            stat = Stat::None;
        }
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

	std::shared_ptr<Job_JSON> job;

	std::stringstream ss;
	ss << in->rdbuf();
    if (!Job_JSON::build(ss.str(), cr, job, ec))
		return 1;

	if (!job->run(ec))
		return 1;

	return 0;
}
