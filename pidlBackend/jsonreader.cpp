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

#include "include/pidlBackend/jsonreader.h"
#include "include/pidlBackend/writer.h"
#include "include/pidlBackend/language.h"

#include <pidlCore/errorcollector.h>
#include <pidlCore/jsontools.h>

#include <fstream>
#include <sstream>
#include <list>
#include <map>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>

namespace PIDL
{

	struct JSONReader::Priv 
	{
		Priv(const std::string & json_stream_) : json_stream(json_stream_)
		{ }

		const std::string json_stream;

		std::map<std::string, std::shared_ptr<Language::TopLevel>> topLevels;

		static bool getOptionalBoolean(const rapidjson::Value & v, const char * name, bool & ret)
		{
			rapidjson::Value * tmp;
			if (!JSONTools::getValue(v, name, tmp))
			{
				ret = false;
				return true;
			}
			if(!tmp->IsBool())
				return false;
			ret = tmp->IsNull() ? false : tmp->GetBool();
			return true;
		}

		static bool getOptionalString(const rapidjson::Value & v, const char * name, std::string & ret)
		{
			if (!v.HasMember(name))
			{
				ret.clear();
				return true;
			}
			rapidjson::Value * tmp;
			if (!JSONTools::getValue(v, name, tmp) || !tmp->IsString())
				return false;

			ret = tmp->IsNull() ? std::string() : tmp->GetString();
			return true;
		}

		static bool getNotNullString(const rapidjson::Value & v, const char * name, std::string & ret)
		{
			if (!JSONTools::getValue(v, name, ret))
				return false;
			return ret.length() > 0;
		}

		static bool getNature(const rapidjson::Value & v, std::string & ret)
		{
			return getNotNullString(v, "nature", ret);
		}

		static bool getName(const rapidjson::Value & v, std::string & ret)
		{
			return getNotNullString(v, "name", ret);
		}

		static bool getTypeValue(const rapidjson::Value & v, rapidjson::Value *& ret)
		{
			if (!JSONTools::getValue(v, "type", ret))
				return false;
			return !ret->IsNull();
		}

		struct ElementRegistry
		{
			ElementRegistry()
			{
				embedded_types["integer"].reset(new Language::Integer());
				embedded_types["float"].reset(new Language::Float());
				embedded_types["string"].reset(new Language::String());
				embedded_types["datetime"].reset(new Language::DateTime());
				embedded_types["void"].reset(new Language::Void());
				embedded_types["boolean"].reset(new Language::Boolean());
				embedded_types["blob"].reset(new Language::Blob());
			}

			std::string path;
			std::map<std::string, std::shared_ptr<Language::Type>> embedded_types;
			std::map<std::string, std::shared_ptr<Language::TypeDefinition>> types;
			std::map<std::string, std::shared_ptr<Language::Function>> functions;

			std::map<std::string, std::shared_ptr<Language::Definition>> definitions;
			std::list<std::shared_ptr<Language::Definition>> definitions_list;
		};


		bool readType(ElementRegistry & registry, const rapidjson::Value & v, std::shared_ptr<Language::Type> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (v.IsNull())
			{
				ec << (error_path + ": type is null");
				return false;
			}
			if (v.IsString())
			{
				std::string name = v.GetString();

				if (registry.embedded_types.count(name))
				{
					ret = registry.embedded_types[name];
					return true;
				}

				if (!registry.types.count(name))
				{
					ec << (error_path + ": type '" + name + "' is not found in '" + registry.path + "'");
					return false;
				}
				ret = registry.types[name];
				return true;
			}
			else if (v.IsObject())
			{
				std::string name;
				if (!getName(v, name))
				{
					ec << (error_path + ": name of type is not specified");
					return false;
				}
				return readType(registry, name, v, ret, error_path + "." + name, ec);
			}
			ec << (error_path + ": invalid type specifier");
			return false;
		}

		bool readType(ElementRegistry & registry, const std::string & name, const rapidjson::Value & v, std::shared_ptr<Language::Type> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (v.IsNull() || !v.IsObject())
			{
				ec << (error_path + ": value of 'type' is not object");
				return false;
			}

			if (name == "nullable")
			{
				rapidjson::Value * t;
				if (!getTypeValue(v, t))
				{
					ec << (error_path + ": type of '" + name + "' is not specified");
					return false;
				}
				std::shared_ptr<Language::Type> tmp;
				if (!readType(registry, *t, tmp, error_path, ec))
					return false;
				ret = std::make_shared<Language::Nullable>(tmp);
			}
			else if (name == "struct")
			{
				rapidjson::Value * ms;
				std::vector<std::shared_ptr<Language::Structure::Member>> members;
				if (JSONTools::getValue(v, "members", ms) && !ms->IsNull())
				{
					if (!ms->IsArray())
					{
						ec << (error_path + ": value of 'members' is invalid");
						return false;
					}
					members.resize(ms->Size());
					for (rapidjson::SizeType i(0), l(ms->Size()); i < l; ++i)
					{
						auto & m = (*ms)[i];
						if (!m.IsObject())
						{
							ec << (error_path + ": value of member #"+std::to_string(i+1)+" is invalid");
							return false;
						}
						std::string m_name;
						if(!getName(m, m_name))
						{
							ec << (error_path + ": name of member #" + std::to_string(i + 1) + " is not specified");
							return false;
						}
						rapidjson::Value * t;
						if (!getTypeValue(m, t))
						{
							ec << (error_path + ": type of '" + m_name + "' is not specified");
							return false;
						}
						std::shared_ptr<Language::Type> tmp;
						if (!readType(registry, *t, tmp, error_path, ec))
							return false;
						members[i] = std::make_shared<Language::Structure::Member>(tmp, m_name);
					}
				}
				ret = std::make_shared<Language::Structure>(members);
			}
			else if (name == "array")
			{
				rapidjson::Value * t;
				if (!getTypeValue(v, t))
				{
					ec << (error_path + ": type is not specified");
					return false;
				}
				std::shared_ptr<Language::Type> tmp;
				if (!readType(registry, *t, tmp, error_path, ec))
					return false;
				ret = std::make_shared<Language::Array>(tmp);
			}
			else
			{
				ec << ("name '"+name+"' of type '" + error_path + "' is invalid");
				return false;
			}
			return true;
		}

		bool readTypeDefinition(ElementRegistry & registry, std::string & name, const rapidjson::Value & v, std::shared_ptr<Language::TypeDefinition> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (v.IsNull() || !v.IsObject())
			{
				ec << (error_path + ": value of 'typedef' is not object");
				return false;
			}
			rapidjson::Value * t;
			if (!getTypeValue(v, t))
			{
				ec << (error_path + ": type of '" + name + "' is not specified");
				return false;
			}
			std::shared_ptr<Language::Type> tmp;
			if (!readType(registry, *t, tmp, error_path, ec))
				return false;

			if (registry.definitions.count(name))
			{
				ec << (error_path + ": name '" + name + "' is already registered");
				return false;
			}
			registry.types[name] = ret = std::make_shared<Language::TypeDefinition>(name, tmp);
			registry.definitions[name] = ret;
			registry.definitions_list.push_back(ret);
			return true;
		}

		bool readFunction(ElementRegistry & registry, const std::vector<std::string> & scope, const std::string & name, const rapidjson::Value & v, std::shared_ptr<Language::Function> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			rapidjson::Value * as;

			rapidjson::Value * r;
			if (!getTypeValue(v, r))
			{
				ec << (error_path + ": type is not specified");
				return false;
			}
			std::shared_ptr<Language::Type> ret_type;
			if (!readType(registry, *r, ret_type, error_path, ec))
				return false;

			std::vector<std::shared_ptr<Language::Function::Argument>> arguments;
			if (JSONTools::getValue(v, "arguments", as) && !as->IsNull())
			{
				if (!as->IsArray())
				{
					ec << (error_path + ": value of 'arguments' is invalid");
					return false;
				}
				arguments.resize(as->Size());
				for (rapidjson::SizeType i(0), l(as->Size()); i < l; ++i)
				{
					auto & a = (*as)[i];
					if (!a.IsObject())
					{
						ec << (error_path + ": value of argument #" + std::to_string(i + 1) + " is invalid");
						return false;
					}
					std::string a_name;
					if (!getName(a, a_name))
					{
						ec << (error_path + ": name of argument #" + std::to_string(i + 1) + " is not specified");
						return false;
					}
					rapidjson::Value * t;
					if (!getTypeValue(a, t))
					{
						ec << (error_path + ": type of '" + a_name + "' is not specified");
						return false;
					}
					std::shared_ptr<Language::Type> tmp;
					if (!readType(registry, *t, tmp, error_path + "." + "a_name", ec))
						return false;
					Language::Function::Argument::Direction direction;
					{
						std::string direction_str;
						if (!getOptionalString(a, "direction", direction_str))
						{
							ec << (error_path + ": 'direction' of '" + a_name + "' is invalid");
							return false;
						}
						if (direction_str.length())
						{
							if (direction_str == "in")
								direction = Language::Function::Argument::Direction::In;
							else if (direction_str == "out")
								direction = Language::Function::Argument::Direction::Out;
							else if (direction_str == "in-out")
								direction = Language::Function::Argument::Direction::InOut;
							else
							{
								ec << (error_path + ": 'direction' of '" + a_name + "' is invalid: " + direction_str);
								return false;
							}
						}
						else
						{
							bool is_out;
							if (!getOptionalBoolean(a, "out", is_out))
							{
								ec << (error_path + ": 'out' of '" + a_name + "' is invalid");
								return false;
							}
							direction = is_out ? Language::Function::Argument::Direction::Out : Language::Function::Argument::Direction::In;
						}
					}


					arguments[i] = std::make_shared<Language::Function::Argument>(tmp, a_name, direction);
				}
			}

			if (registry.definitions.count(name))
			{
				ec << (error_path + ": name '" + name + "' is already registered");
				return false;
			}
			registry.functions[name] = ret = std::make_shared<Language::Function>(ret_type, scope, name, arguments);
			registry.definitions[name] = ret;
			registry.definitions_list.push_back(ret);
			return true;
		}

		bool readInterface(std::vector<std::string> scope, const std::string & name, const rapidjson::Value & v, std::shared_ptr<Language::Interface> & ret, ErrorCollector & ec)
		{
			ElementRegistry registry;
			registry.path = name;

			scope.push_back(name);

			rapidjson::Value * b;
			if (JSONTools::getValue(v, "body", b))
			{
				if (!b->IsArray())
				{
					ec << ("body of interface '" + name + "' is not array");
					return false;
				}
				for (rapidjson::SizeType i(0), l(b->Size()); i < l; ++i)
				{
					auto & e = (*b)[i];
					if (!e.IsNull())
					{
						std::string e_name;
						if (!getName(e, e_name))
						{
							ec << ("name of element #" + std::to_string(i) + " of interface '" + name + "' is not specified");
							return false;
						}

						std::string e_nature;
						if (!getNature(e, e_nature))
						{
							ec << ("nature of element '" + name + "." + e_name + "' is not specified");
							return false;
						}

						if (e_nature == "typedef")
						{
							std::shared_ptr<Language::TypeDefinition> tmp;
							if (!readTypeDefinition(registry, e_name, e, tmp, name + "." + e_name, ec))
								return false;
						}
						else if (e_nature == "function")
						{
							std::shared_ptr<Language::Function> tmp;
							if (!readFunction(registry, scope, e_name, e, tmp, name + "." + e_name, ec))
								return false;
						}
						else
						{
							ec << ("nature '" + e_nature + "' of element '" + name + "." + e_name + "' is invalid");
							return false;
						}
					}
				}
			}

			ret = std::make_shared<Language::Interface>(name, registry.definitions_list);

			return true;
		}

		bool readModule(const std::string name, const rapidjson::Value & v, std::shared_ptr<Language::Module> & ret, ErrorCollector & ec)
		{
			ElementRegistry registry;
			registry.path = name;

			std::map<std::string, std::shared_ptr<Language::TopLevel>> elements;

			rapidjson::Value * b;
			if (JSONTools::getValue(v, "body", b))
			{
				if (!b->IsArray())
				{
					ec << ("body of module '" + name + "' is not array");
					return false;
				}
				for (rapidjson::SizeType i(0), l(b->Size()); i < l; ++i)
				{
					auto & e = (*b)[i];
					if (!e.IsNull())
					{
						std::string e_name;
						if (!getName(e, e_name))
						{
							ec << ("name of element #" + std::to_string(i) + " of module '" + name + "' is not specified");
							return false;
						}

						std::shared_ptr<Language::TopLevel> tmp;
						if (!readTopLevel(e_name, e, tmp, ec))
							return false;
						elements[e_name] = tmp;
					}
				}
			}

			std::vector<std::shared_ptr<Language::TopLevel>> _elements(elements.size());
			{
				size_t i(0);
				for (auto & d : elements)
					_elements[i++] = d.second;
			}

			ret = std::make_shared<Language::Module>(name, _elements);

			return true;
		}

		bool readTopLevel(const std::string name, const rapidjson::Value & v, std::shared_ptr<Language::TopLevel> & ret, ErrorCollector & ec)
		{
			std::string nature;
			if (!getNature(v, nature))
			{
				ec << "nature of the toplevel element is not specified";
				return false;
			}

			if (nature == "interface")
			{
				std::shared_ptr<Language::Interface> tmp;
				std::vector<std::string> scope;
				if (!readInterface(scope, name, v, tmp, ec))
					return false;
				ret = tmp;
			}
			else if (nature == "module")
			{
				std::shared_ptr<Language::Module> tmp;
				if (!readModule(name, v, tmp, ec))
					return false;
				ret = tmp;
			}
			else
			{
				ec << "nature of toplevel element '"+name+"' is invalid";
				return false;
			}

			return true;
		}

		bool read(const rapidjson::Document & doc, ErrorCollector & ec)
		{
			if (doc.IsNull())
			{
				ec << "toplevel element is null";
				return false;
			}

			std::string name;
			if (!getName(doc, name))
			{
				ec << "name of interface is not toplevel element";
				return false;
			}

			std::shared_ptr<Language::TopLevel> tmp;
			if (!readTopLevel(name, doc, tmp, ec))
				return false;

			topLevels[name] = tmp;

			return true;
		}

		bool read(const std::string & json_stream, ErrorCollector & ec)
		{
			std::vector<char> buffer(json_stream.length() + 1);
			memcpy(buffer.data(), json_stream.c_str(), json_stream.length());

			rapidjson::Document doc;

			if (doc.ParseInsitu(buffer.data()).HasParseError())
			{
				ec << ("JSON parse error (" + std::to_string(doc.GetParseError()) + ")");
				return false;
			}

			return read(doc, ec);
		}
	};

	JSONReader::JSONReader(const std::string & json_stream) : priv(new Priv(json_stream))
	{ }

	JSONReader::~JSONReader()
	{
		delete priv;
	}

	bool JSONReader::read(ErrorCollector & ec)
	{
		return priv->read(priv->json_stream, ec);
	}

	std::vector<std::shared_ptr<Language::TopLevel>> JSONReader::topLevels() const
	{
		std::vector<std::shared_ptr<Language::TopLevel>> ret(priv->topLevels.size());
		size_t idx(0);
		for (auto & i : priv->topLevels)
			ret[idx++] = i.second;
		return ret;
	}

	bool JSONReader::compile(Writer * writer, const std::string & json_stream, std::string & ret, ErrorCollector & ec)
	{
		JSONReader p(json_stream);
		if (!p.read(ec))
			return false;

		return writer->write(&p, ec);
	}


}
