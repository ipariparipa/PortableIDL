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

#include "include/pidlBackend/xmlreader.h"
#include "include/pidlBackend/writer.h"
#include "include/pidlBackend/language.h"

#include <pidlCore/errorcollector.h>

#include <fstream>
#include <sstream>
#include <list>
#include <map>

#include <string.h>

#include "rapidxml/rapidxml.hpp"

namespace PIDL
{

	struct XMLReader::Priv
	{
		Priv(const std::string & xml_stream_) : xml_stream(xml_stream_) 
		{ }

		std::string xml_stream;

		std::map<std::string, std::shared_ptr<Language::TopLevel>> topLevels;

		static rapidxml::xml_attribute<> * getAttribute(const rapidxml::xml_node<> * v, const char * name)
		{
			return v->first_attribute(name);
		}

		static rapidxml::xml_node<> * getNode(const rapidxml::xml_node<> * v, const char * name)
		{
			return v->first_node(name);
		}

		static bool getAttribute(const rapidxml::xml_node<> * v, const char * name, rapidxml::xml_attribute<> *& ret)
		{
			return (ret = getAttribute(v, name)) != nullptr;
		}

		static bool getNode(const rapidxml::xml_node<> * v, const char * name, rapidxml::xml_node<> *& ret)
		{
			return (ret = getNode(v, name)) != nullptr;
		}

		static bool getOptionalBooleanAttr(const rapidxml::xml_node<> * v, const char * name, bool & ret)
		{
			rapidxml::xml_attribute<> * tmp;
			if (!getAttribute(v, name, tmp))
			{
				ret = false;
				return true;
			}
			ret = std::string(tmp->value()) == "true";
			return true;
		}

		static bool getStringAttr(const rapidxml::xml_node<> * v, const char * name, std::string & ret)
		{
			rapidxml::xml_attribute<> * tmp;
			if (!(getAttribute(v, name, tmp)))
				return false;

			ret = tmp->value();
			return true;
		}

		static bool getOptionalStringAttr(const rapidxml::xml_node<> * v, const char * name, std::string & ret)
		{
			rapidxml::xml_attribute<> * tmp;
			if (!getAttribute(v, name, tmp))
			{
				ret.clear();
				return true;
			}
			ret = tmp->value();
			return true;
		}

		static bool getNotNullStringAttr(const rapidxml::xml_node<> * v, const char * name, std::string & ret)
		{
			if (!getStringAttr(v, name, ret))
				return false;
			return ret.length() > 0;
		}

		static bool getNature(const rapidxml::xml_node<> * v, std::string & ret)
		{
			ret = v->name();
			return true;
		}

		static bool getName(const rapidxml::xml_node<> * v, std::string & ret)
		{
			return getNotNullStringAttr(v, "name", ret);
		}

		static bool getTypeElem(const rapidxml::xml_node<> * v, rapidxml::xml_base<> *& ret)
		{
			return (ret = getNode(v, "type")) || (ret = getAttribute(v, "type"));
		}

		struct InterfaceElementRegistry
		{
			InterfaceElementRegistry()
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
			std::map<std::string, std::shared_ptr<Language::Type>> types;
			std::map<std::string, std::shared_ptr<Language::Function>> functions;

			std::map<std::string, std::shared_ptr<Language::Definition>> definitions;
			std::list<std::shared_ptr<Language::Definition>> definitions_list;
		};

		struct ObjectElementRegistry
		{
			std::string path;

			std::map<std::string, std::shared_ptr<Language::Definition>> definitions;
			std::list<std::shared_ptr<Language::Definition>> definitions_list;
		};


		bool readDocumentation(const rapidxml::xml_base<> * v, Language::DocumentationProvider::Documentation & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (dynamic_cast<const rapidxml::xml_node<>*>(v))
			{
				auto *_v = dynamic_cast<const rapidxml::xml_node<>*>(v);
				auto doc_n = _v->first_node("documentation");
				if (doc_n)
				{
					auto brief_n = doc_n->first_node("brief");
					if (brief_n)
						ret.brief = brief_n->value();

					auto desc_n = doc_n->first_node("description");
					if (desc_n)
						ret.details[Language::DocumentationProvider::Documentation::Description] = desc_n->value();

					auto ret_n = doc_n->first_node("return");
					if (ret_n)
						ret.details[Language::DocumentationProvider::Documentation::Return] = ret_n->value();

					if (!brief_n && !desc_n && !ret_n)
						ret.brief = doc_n->value();
					else if (!brief_n)
					{
						ec << (error_path + ": 'brief' is not defined for documentation");
						return false;
					}
				}
				else
				{
					auto doc_a = _v->first_attribute("documentation");
					if (doc_a)
						ret.brief = doc_a->value();
				}
			}

			return true;
		}

		bool readType(InterfaceElementRegistry & registry, const rapidxml::xml_base<> * v, std::shared_ptr<Language::Type> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (dynamic_cast<const rapidxml::xml_attribute<>*>(v))
			{
				std::string name = v->value();

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
			else if (dynamic_cast<const rapidxml::xml_node<>*>(v))
			{
				auto *_v = dynamic_cast<const rapidxml::xml_node<>*>(v);
				std::string name;
				if (!getName(_v, name))
				{
					ec << (error_path + ": name of type is not specified");
					return false;
				}
				return readType(registry, name, _v, ret, error_path + "." + name, ec);
			}

			ec << (error_path + ": invalid type specifier");
			return false;
		}

		bool readType(InterfaceElementRegistry & registry, const std::string & name, const rapidxml::xml_node<> * v, std::shared_ptr<Language::Type> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (name == "nullable")
			{
				rapidxml::xml_base<> * t;
				if (!getTypeElem(v, t))
				{
					ec << (error_path + ": type of '" + name + "' is not specified");
					return false;
				}
				std::shared_ptr<Language::Type> tmp;
				if (!readType(registry, t, tmp, error_path, ec))
					return false;
				ret = std::make_shared<Language::Nullable>(tmp);
			}
			else if (name == "struct")
			{
				rapidxml::xml_node<> * ms;
				std::list<std::shared_ptr<Language::Structure::Member>> members;
				if (getNode(v, "members", ms))
				{
					size_t i(0);
					for (auto m = ms->first_node(); m; m = m->next_sibling())
					{
						if (std::string(m->name()) != "member" && std::string(m->name()) != "memb")
						{
							ec << (error_path + ": invalid node #" + std::to_string(i + 1) + ": '" + m->name() + "'");
							return false;
						}
						std::string m_name;
						if (!getName(m, m_name))
						{
							ec << (error_path + ": name of member #" + std::to_string(i + 1) + " is not specified");
							return false;
						}
						rapidxml::xml_base<> * t;
						if (!getTypeElem(m, t))
						{
							ec << (error_path + ": type of '" + m_name + "' is not specified");
							return false;
						}
						std::shared_ptr<Language::Type> tmp;
						if (!readType(registry, t, tmp, error_path, ec))
							return false;

						Language::DocumentationProvider::Documentation doc;
						if (!readDocumentation(m, doc, error_path, ec))
							return false;

						members.push_back(std::make_shared<Language::Structure::Member>(tmp, m_name, doc));
						++i;
					}
				}
				ret = std::make_shared<Language::Structure>(members);
			}
			else if (name == "array")
			{
				rapidxml::xml_base<> * t;
				if (!getTypeElem(v, t))
				{
					ec << (error_path + ": type is not specified");
					return false;
				}
				std::shared_ptr<Language::Type> tmp;
				if (!readType(registry, t, tmp, error_path, ec))
					return false;
				ret = std::make_shared<Language::Array>(tmp);
			}
			else
			{
				ec << ("name '" + name + "' of type '" + error_path + "' is invalid");
				return false;
			}
			return true;
		}

		bool readTypeDefinition(InterfaceElementRegistry & registry, const std::vector<std::string> & scope, std::string & name, const rapidxml::xml_node<> * v, std::shared_ptr<Language::TypeDefinition> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			rapidxml::xml_base<> * t;
			if (!getTypeElem(v, t))
			{
				ec << (error_path + ": type of '" + name + "' is not specified");
				return false;
			}
			std::shared_ptr<Language::Type> tmp;
			if (!readType(registry, t, tmp, error_path, ec))
				return false;

			if (registry.definitions.count(name))
			{
				ec << (error_path + ": name '" + name + "' is already registered");
				return false;
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, error_path, ec))
				return false;

			registry.types[name] = ret = std::make_shared<Language::TypeDefinition>(name, tmp, scope, doc);
			registry.definitions[name] = ret;
			registry.definitions_list.push_back(ret);
			return true;
		}

		bool readMethod(InterfaceElementRegistry & registry, ObjectElementRegistry & objectRegistry, const std::vector<std::string> & scope, const std::string & name, const rapidxml::xml_node<> * v, std::shared_ptr<Language::Method> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			rapidxml::xml_base<> * r;
			if (!getTypeElem(v, r))
			{
				ec << (error_path + ": type is not specified");
				return false;
			}
			std::shared_ptr<Language::Type> ret_type;
			if (!readType(registry, r, ret_type, error_path, ec))
				return false;

			rapidxml::xml_node<> * as;
			std::list<std::shared_ptr<Language::Function::Argument>> arguments;
			if (getNode(v, "arguments", as))
			{
				size_t i(0);
				for (auto a = as->first_node(); a; a = a->next_sibling())
				{
					if (std::string(a->name()) != "argument" && std::string(a->name()) != "arg")
					{
						ec << (error_path + ": invalid node #" + std::to_string(i + 1) + ": '" + a->name() + "'");
						return false;
					}
					std::string a_name;
					if (!getName(a, a_name))
					{
						ec << (error_path + ": name of argument #" + std::to_string(i + 1) + " is not specified");
						return false;
					}
					rapidxml::xml_base<> * t;
					if (!getTypeElem(a, t))
					{
						ec << (error_path + ": type of '" + a_name + "' is not specified");
						return false;
					}
					std::shared_ptr<Language::Type> tmp;
					if (!readType(registry, t, tmp, error_path + "." + "a_name", ec))
						return false;
					Language::Function::Argument::Direction direction;
					{
						std::string direction_str;
						if (!getOptionalStringAttr(a, "direction", direction_str))
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
							if (!getOptionalBooleanAttr(a, "out", is_out))
							{
								ec << (error_path + ": 'out' of '" + a_name + "' is invalid");
								return false;
							}
							direction = is_out ? Language::Function::Argument::Direction::Out : Language::Function::Argument::Direction::In;
						}
					}

					Language::DocumentationProvider::Documentation doc;
					if (!readDocumentation(a, doc, error_path, ec))
						return false;

					arguments.push_back(std::make_shared<Language::Function::Argument>(tmp, a_name, direction, doc));
					++i;
				}
			}

			if (objectRegistry.definitions.count(name))
			{
				ec << (error_path + ": name '" + name + "' is already registered");
				return false;
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, error_path, ec))
				return false;

			objectRegistry.definitions[name] = ret = std::make_shared<Language::Method>(ret_type, scope, name, arguments, doc);
			objectRegistry.definitions_list.push_back(ret);
			return true;
		}

		bool readProperty(InterfaceElementRegistry & registry, ObjectElementRegistry & objectRegistry,
				const std::vector<std::string> & scope, const std::string & name,
				const rapidxml::xml_node<> * v, std::shared_ptr<Language::Property> & ret,
				const std::string & error_path, ErrorCollector & ec)
		{
			rapidxml::xml_base<> * r;
			if (!getTypeElem(v, r))
			{
				ec << (error_path + ": type is not specified");
				return false;
			}
			std::shared_ptr<Language::Type> type;
			if (!readType(registry, r, type, error_path, ec))
				return false;

			if (objectRegistry.definitions.count(name))
			{
				ec << (error_path + ": name '" + name + "' is already registered");
				return false;
			}

			bool readonly;
			if (!getOptionalBooleanAttr(v, "readonly", readonly))
			{
				ec << (error_path + ": unexpected: could not get optional attribute 'readonly'");
				return false;
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, error_path, ec))
				return false;

			objectRegistry.definitions[name] = ret = std::make_shared<Language::Property>(type, scope, name, readonly, doc);
			objectRegistry.definitions_list.push_back(ret);
			return true;
		}

		bool readObject(InterfaceElementRegistry & interfaceRegistry, std::vector<std::string> scope, const std::string & name, const rapidxml::xml_node<> * v, std::shared_ptr<Language::Object> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			rapidxml::xml_node<> * b;

			ObjectElementRegistry registry;
			registry.path = name;

			if (getNode(v, "body", b))
			{
				size_t i(0);
				for (auto e = b->first_node(); e; e = e->next_sibling())
				{
					++i;
					std::string e_name;
					if (!getName(e, e_name))
					{
						ec << ("name of element #" + std::to_string(i) + " of object '" + error_path + "' is not specified");
						return false;
					}

					std::string e_nature;
					if (!getNature(e, e_nature))
					{
						ec << ("nature of element '" + error_path + "." + e_name + "' is not specified");
						return false;
					}

					if (e_nature == "property")
					{
						std::shared_ptr<Language::Property> tmp;
						auto _scope = scope;
						_scope.push_back(name);
						if (!readProperty(interfaceRegistry, registry, _scope, e_name, e, tmp, error_path + "." + e_name, ec))
							return false;
					}
					else if (e_nature == "method")
					{
						std::shared_ptr<Language::Method> tmp;
						auto _scope = scope;
						_scope.push_back(name);
						if (!readMethod(interfaceRegistry, registry, _scope, e_name, e, tmp, error_path + "." + e_name, ec))
							return false;
					}
					else
					{
						ec << ("nature '" + e_nature + "' of element '" + name + "." + e_name + "' is invalid");
						return false;
					}
				}
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, error_path, ec))
				return false;

			ret = std::make_shared<Language::Object>(name, registry.definitions_list, scope, doc);
			interfaceRegistry.types[name] = ret;
			interfaceRegistry.definitions[name] = ret;
			interfaceRegistry.definitions_list.push_back(ret);

			return true;
		}

		bool readFunction(InterfaceElementRegistry & registry, const std::vector<std::string> & scope, const std::string & name, const rapidxml::xml_node<> * v, std::shared_ptr<Language::Function> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			rapidxml::xml_base<> * r;
			if (!getTypeElem(v, r))
			{
				ec << (error_path + ": type is not specified");
				return false;
			}
			std::shared_ptr<Language::Type> ret_type;
			if (!readType(registry, r, ret_type, error_path, ec))
				return false;

			rapidxml::xml_node<> * as;
			std::list<std::shared_ptr<Language::Function::Argument>> arguments;
			if (getNode(v, "arguments", as))
			{
				size_t i(0);
				for (auto a = as->first_node(); a; a = a->next_sibling())
				{
					if (std::string(a->name()) != "argument" && std::string(a->name()) != "arg")
					{
						ec << (error_path + ": invalid node #" + std::to_string(i + 1) + ": '" + a->name() + "'");
						return false;
					}
					std::string a_name;
					if (!getName(a, a_name))
					{
						ec << (error_path + ": name of argument #" + std::to_string(i + 1) + " is not specified");
						return false;
					}
					rapidxml::xml_base<> * t;
					if (!getTypeElem(a, t))
					{
						ec << (error_path + ": type of '" + a_name + "' is not specified");
						return false;
					}
					std::shared_ptr<Language::Type> tmp;
					if (!readType(registry, t, tmp, error_path + "." + "a_name", ec))
						return false;
					Language::Function::Argument::Direction direction;
					{
						std::string direction_str;
						if (!getOptionalStringAttr(a, "direction", direction_str))
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
							if (!getOptionalBooleanAttr(a, "out", is_out))
							{
								ec << (error_path + ": 'out' of '" + a_name + "' is invalid");
								return false;
							}
							direction = is_out ? Language::Function::Argument::Direction::Out : Language::Function::Argument::Direction::In;
						}
					}

					Language::DocumentationProvider::Documentation doc;
					if (!readDocumentation(a, doc, error_path, ec))
						return false;

					arguments.push_back(std::make_shared<Language::Function::Argument>(tmp, a_name, direction, doc));
					++i;
				}
			}

			if (registry.definitions.count(name))
			{
				ec << (error_path + ": name '" + name + "' is already registered");
				return false;
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, error_path, ec))
				return false;

			registry.functions[name] = ret = std::make_shared<Language::Function>(ret_type, scope, name, arguments, doc);
			registry.definitions[name] = ret;
			registry.definitions_list.push_back(ret);
			return true;
		}

		bool readInterface(std::vector<std::string> scope, const std::string & name, const rapidxml::xml_node<> * v, std::shared_ptr<Language::Interface> & ret, ErrorCollector & ec)
		{
			rapidxml::xml_node<> * b;

			InterfaceElementRegistry registry;

			if (getNode(v, "body", b))
			{
				size_t i(0);
				for (auto e = b->first_node(); e; e = e->next_sibling())
				{
					++i;
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
						auto _scope = scope;
						_scope.push_back(name);
						std::shared_ptr<Language::TypeDefinition> tmp;
						if (!readTypeDefinition(registry, _scope, e_name, e, tmp, name + "." + e_name, ec))
							return false;
					}
					else if (e_nature == "object")
					{
						auto _scope = scope;
						_scope.push_back(name);
						std::shared_ptr<Language::Object> tmp;
						if (!readObject(registry, _scope, e_name, e, tmp, name + "." + e_name, ec))
							return false;
					}
					else if (e_nature == "function")
					{
						auto _scope = scope;
						_scope.push_back(name);
						std::shared_ptr<Language::Function> tmp;
						if (!readFunction(registry, _scope, e_name, e, tmp, name + "." + e_name, ec))
							return false;
					}
					else
					{
						ec << ("nature '" + e_nature + "' of element '" + name + "." + e_name + "' is invalid");
						return false;
					}
				}
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, name, ec))
				return false;

			ret = std::make_shared<Language::Interface>(name, registry.definitions_list, scope, doc);

			return true;
		}

		bool readModule(const std::string name, const rapidxml::xml_node<> * v, std::shared_ptr<Language::Module> & ret, ErrorCollector & ec)
		{
			rapidxml::xml_node<> * b;

			std::map<std::string, std::shared_ptr<Language::TopLevel>> elements;

			if (getNode(v, "body", b))
			{
				size_t i(0);
				for (auto e = b->first_node(); e; e = e->next_sibling())
				{
					std::string e_name;
					if (!getName(e, e_name))
					{
						ec << ("name of element #" + std::to_string(i + 1) + " of module '" + name + "' is not specified");
						return false;
					}

					std::shared_ptr<Language::TopLevel> tmp;
					if (!readTopLevel(e_name, e, tmp, ec))
						return false;
					elements[e_name] = tmp;

					++i;
				}
			}

			std::vector<std::shared_ptr<Language::TopLevel>> _elements(elements.size());
			{
				size_t i(0);
				for (auto & d : elements)
					_elements[i++] = d.second;
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, name, ec))
				return false;

			ret = std::make_shared<Language::Module>(name, _elements, doc);

			return true;
		}

		bool readTopLevel(const std::string name, const rapidxml::xml_node<> * v, std::shared_ptr<Language::TopLevel> & ret, ErrorCollector & ec)
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
				ec << "nature of toplevel element '" + name + "' is invalid";
				return false;
			}

			return true;
		}

		bool read(const rapidxml::xml_document<> * doc, ErrorCollector & ec)
		{
			for (auto n = doc->first_node("module"); n; n = n->next_sibling("module"))
			{
				std::string name;
				if (!getName(n, name))
				{
					ec << "name of interface is not toplevel element";
					return false;
				}

				std::shared_ptr<Language::TopLevel> tmp;
				if (!readTopLevel(name, n, tmp, ec))
					return false;

				topLevels[name] = tmp;
			}

			return true;
		}

		bool read(const std::string & json_stream, ErrorCollector & ec)
		{
			std::vector<char> buffer(json_stream.length() + 1);
			memcpy(buffer.data(), json_stream.c_str(), json_stream.length());

			rapidxml::xml_document<> doc;

			try
			{
				doc.parse<0>(buffer.data());
			}
			catch (const std::runtime_error & e)
			{
				ec << (std::string("XML parse error (") + e.what() + ")");
				return false;
			}
			catch (const rapidxml::parse_error & e)
			{
				ec << (std::string("XML parse error (") + e.where<char>() + ":" + e.what() + ")");
				return false;
			}

			return read(&doc, ec);
		}
	};

	XMLReader::XMLReader(const std::string & xml_stream) : priv(new Priv(xml_stream))
	{ }

	XMLReader::~XMLReader()
	{
		delete priv;
	}

	bool XMLReader::read(ErrorCollector & ec)
	{
		return priv->read(priv->xml_stream, ec);
	}

	std::vector<std::shared_ptr<Language::TopLevel>> XMLReader::topLevels() const
	{
		std::vector<std::shared_ptr<Language::TopLevel>> ret(priv->topLevels.size());
		size_t idx(0);
		for (auto & i : priv->topLevels)
			ret[idx++] = i.second;
		return ret;
	}

	bool XMLReader::compile(Writer * writer, const std::string & xml_stream, std::string & ret, ErrorCollector & ec)
	{
		XMLReader p(xml_stream);
		if (!p.read(ec))
			return false;

		return writer->write(&p, ec);
	}

}
