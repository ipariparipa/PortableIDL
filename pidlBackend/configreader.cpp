/*
    This file is part of pidlBackend.

    pidlBackend is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidlBackend is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidlBackend.  If not, see <http://www.gnu.org/licenses/>
 */

#include "include/pidlBackend/configreader.h"
#include <pidlCore/errorcollector.h>

namespace PIDL {

//virtual
ConfigReader::Status ConfigReader::get(const std::string & name, std::string & ret, ErrorCollector & ec)
{
    return getAsString(name, ret, ec);
}

//virtual
ConfigReader::Status ConfigReader::get(const std::string & name, long long & ret, ErrorCollector & ec)
{
    std::string str;
    switch(getAsString(name, str, ec))
    {
    case Status::OK:
        try
        { ret = std::stoll(str); }
        catch (...)
        {
            ec << "config entry '"+name+"' could not be recognized as integer";
            return Status::Error;
        }
        return Status::OK;
    case Status::NotFound:
        return Status::NotFound;
    case Status::Error:
        return Status::Error;
    }
}

//virtual
ConfigReader::Status ConfigReader::get(const std::string & name, double & ret, ErrorCollector & ec)
{
    std::string str;
    switch(getAsString(name, str, ec))
    {
    case Status::OK:
        try
        { ret = std::stod(str); }
        catch (...)
        {
            ec << "config entry '"+name+"' could not be recognized as float";
            return Status::Error;
        }
        return Status::OK;
    case Status::NotFound:
        return Status::NotFound;
    case Status::Error:
        return Status::Error;
    }
}

//virtual
ConfigReader::Status ConfigReader::get(const std::string & name, bool & ret, ErrorCollector & ec)
{
    std::string str;
    switch(getAsString(name, str, ec))
    {
    case Status::OK:
        if(str == "true")
            ret = true;
        else if(str == "false")
            ret = true;
        else
            try
            { ret = std::stoll(str); }
            catch (...)
            {
                ec << "config entry '"+name+"' could not be recognized as bool";
                return Status::Error;
            }
        return Status::OK;
    case Status::NotFound:
        return Status::NotFound;
    case Status::Error:
        return Status::Error;
    }
}

}

