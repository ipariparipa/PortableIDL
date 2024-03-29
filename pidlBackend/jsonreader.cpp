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


        struct InterfaceElementRegistry;

        struct BaseElementRegistry
        {
            std::string path;

            std::map<std::string, std::shared_ptr<Language::Type>> types;

            std::map<std::string, std::shared_ptr<Language::Definition>> definitions;
            std::list<std::shared_ptr<Language::Definition>> definitions_list;

            InterfaceElementRegistry * interfaceRegistry =  nullptr;

            const std::shared_ptr<Language::Type> & getType(const std::string & name) const
            {
                assert(interfaceRegistry);
                if (interfaceRegistry->embedded_types.count(name))
                    return interfaceRegistry->embedded_types.at(name);
                else if (types.count(name))
                    return types.at(name);
                else if (interfaceRegistry->types.count(name))
                    return interfaceRegistry->types.at(name);

                static const std::shared_ptr<Language::Type> null;
                return null;
            }

        protected:
            BaseElementRegistry() = default;
        };

        struct InterfaceElementRegistry : public BaseElementRegistry
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
                interfaceRegistry = this;
			}

			std::map<std::string, std::shared_ptr<Language::Type>> embedded_types;
			std::map<std::string, std::shared_ptr<Language::Function>> functions;
		};

        struct ObjectElementRegistry : public BaseElementRegistry
		{
            ObjectElementRegistry(InterfaceElementRegistry * reg)
            {
                interfaceRegistry = reg;
            }
		};

        std::string appendLoggerName(const std::string & base, const std::string name)
        {
            if(base.length())
                return base + "." + name;

            return name;
        }

		bool readDocumentation(const rapidjson::Value & v, Language::DocumentationProvider::Documentation & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (v.IsObject())
			{
				rapidjson::Value * doc_v;
                if (JSONTools::getValue(v, "documentation", doc_v))
				{
					if (doc_v->IsString())
						ret.brief = doc_v->GetString();
					else if (doc_v->IsObject())
					{
						if (!JSONTools::getValue(*doc_v, "brief", ret.brief))
						{
							ec << (error_path + ": 'brief' is not defined for documentation");
							return false;
						}

						std::string tmp;

						if (JSONTools::getValue(*doc_v, "description", tmp))
							ret.details[Language::DocumentationProvider::Documentation::Description] = tmp;

						if (JSONTools::getValue(*doc_v, "return", tmp))
							ret.details[Language::DocumentationProvider::Documentation::Return] = tmp;

                        if (JSONTools::getValue(*doc_v, "group", tmp))
                            ret.details[Language::DocumentationProvider::Documentation::Group] = tmp;
                    }
				}
                else
                {
                    JSONTools::getValue(v, "brief", ret.brief);

                    std::string tmp;
                    if(JSONTools::getValue(v, "doc_group", tmp))
                        ret.details[Language::DocumentationProvider::Documentation::Group] = tmp;
                }
            }

			return true;
		}

        bool readType(BaseElementRegistry & registry, const rapidjson::Value & v, std::vector<Language::Type::Ptr> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (v.IsNull() || !v.IsArray())
			{
				ec << (error_path + ": value of 'types' is not array or null");
				return false;
			}
			ret.resize(v.Size());
			bool has_error = false;
			for (rapidjson::SizeType i = 0, l = v.Size(); i < l; ++i)
			{
				if (!readType(registry, v[i], ret[i], error_path, ec))
					has_error = true;
			}

			return !has_error;
		}

        bool readType(BaseElementRegistry & registry, const rapidjson::Value & v, Language::Type::Ptr & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (v.IsNull())
			{
				ec << (error_path + ": type is null");
				return false;
			}
			if (v.IsString())
			{
				std::string name = v.GetString();

                if (!(ret = registry.getType(name)))
                {
                    ec << (error_path + ": type '" + name + "' is not found in '" + registry.path + "'");
					return false;
				}

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

        bool readType(BaseElementRegistry & registry, const std::string & name, const rapidjson::Value & v, Language::Type::Ptr & ret, const std::string & error_path, ErrorCollector & ec)
		{
			if (v.IsNull() || !v.IsObject())
			{
				ec << (error_path + ": value of 'type' is not object or null");
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
				Language::Type::Ptr tmp;
				if (!readType(registry, *t, tmp, error_path, ec))
					return false;
				ret = std::make_shared<Language::Nullable>(tmp);
			}
			else if (name == "structure")
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

						Language::DocumentationProvider::Documentation doc;
						if (!readDocumentation(m, doc, error_path, ec))
							return false;

						members[i] = std::make_shared<Language::Structure::Member>(tmp, m_name, doc);
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
				Language::Type::Ptr tmp;
				if (!readType(registry, *t, tmp, error_path, ec))
					return false;
				ret = std::make_shared<Language::Array>(tmp);
			}
			else if (name == "tuple")
			{
				rapidjson::Value * t;
				if (!JSONTools::getValue(v, "types", t) || t->IsNull())
				{
					ec << (error_path + ": types are not specified");
					return false;
				}
				std::vector<Language::Type::Ptr> tmp;
				if (!readType(registry, *t, tmp, error_path, ec))
					return false;
				ret = std::make_shared<Language::Tuple>(tmp);
			}
			else
			{
                if (!(ret = registry.getType(name)))
                {
                    ec << (error_path + ": type '" + name + "' is not found in '" + registry.path + "'");
                    return false;
                }
            }
			return true;
		}

        bool readTypeDefinition(BaseElementRegistry & registry, const std::vector<std::string> & scope, std::string & name, const rapidjson::Value & v, std::shared_ptr<Language::TypeDefinition> & ret, const std::string & error_path, ErrorCollector & ec)
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

            if (registry.interfaceRegistry->embedded_types.count(name))
            {
                ec << (error_path + ": name '" + name + "' is already registered as enbedded type");
                return false;
            }

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

        bool readFunction(InterfaceElementRegistry & registry, const std::vector<std::string> & scope, const std::string & name, const rapidjson::Value & v, Language::Function::Variant::Ptr & ret, const std::string & error_path, ErrorCollector & ec)
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

			std::vector<std::shared_ptr<Language::Function::Variant::Argument>> arguments;
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
					Language::Function::Variant::Argument::Direction direction;
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
								direction = Language::Function::Variant::Argument::Direction::In;
							else if (direction_str == "out")
								direction = Language::Function::Variant::Argument::Direction::Out;
							else if (direction_str == "in-out")
								direction = Language::Function::Variant::Argument::Direction::InOut;
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
							direction = is_out ? Language::Function::Variant::Argument::Direction::Out : Language::Function::Variant::Argument::Direction::In;
						}
					}

					Language::DocumentationProvider::Documentation doc;
					if (!readDocumentation(a, doc, error_path, ec))
						return false;

					arguments[i] = std::make_shared<Language::Function::Variant::Argument>(tmp, a_name, direction, doc);
				}
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, error_path, ec))
				return false;

			Language::Function::Ptr func;
			if (!registry.definitions.count(name))
				registry.definitions[name] = registry.functions[name] = func = std::make_shared<Language::Function>(scope, name);
			else if (!(func = std::dynamic_pointer_cast<Language::Function>(registry.definitions[name])))
			{
				ec << (error_path + ": unexpected: name '" + name + "' has different definition type");
				return false;
			}

			auto var = std::make_shared<Language::Function::Variant>(func, ret_type, name, arguments, doc);

			if (func->variants().count(var->variantId()))
			{
				ec << (error_path + ": name '" + name + "' variant id '" + var->variantId() + "' is already registered");
				return false;
			}

			registry.definitions_list.push_back(func->variants()[var->variantId()] = ret = var);
			return true;
		}

        bool readMethod(ObjectElementRegistry & registry, const std::vector<std::string> & scope, const std::string & name, const rapidjson::Value & v, Language::Method::Variant::Ptr & ret, const std::string & error_path, ErrorCollector & ec)
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

			std::vector<std::shared_ptr<Language::Function::Variant::Argument>> arguments;
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
					Language::Function::Variant::Argument::Direction direction;
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
								direction = Language::Function::Variant::Argument::Direction::In;
							else if (direction_str == "out")
								direction = Language::Function::Variant::Argument::Direction::Out;
							else if (direction_str == "in-out")
								direction = Language::Function::Variant::Argument::Direction::InOut;
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
							direction = is_out ? Language::Function::Variant::Argument::Direction::Out : Language::Function::Variant::Argument::Direction::In;
						}
					}

					Language::DocumentationProvider::Documentation doc;
					if (!readDocumentation(a, doc, error_path, ec))
						return false;

					arguments[i] = std::make_shared<Language::Function::Variant::Argument>(tmp, a_name, direction, doc);
				}
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, error_path, ec))
				return false;

			Language::Method::Ptr meth;
            if (!registry.definitions.count(name))
                registry.definitions[name] = meth = std::make_shared<Language::Method>(scope, name);
            else if (!(meth = std::dynamic_pointer_cast<Language::Method>(registry.definitions[name])))
			{
				ec << (error_path + ": unexpected: name '" + name + "' has different definition type");
				return false;
			}

			auto var = std::make_shared<Language::Method::Variant>(meth, ret_type, name, arguments, doc);

			if (meth->variants().count(var->variantId()))
			{
				ec << (error_path + ": name '" + name + "' with variant ID '" + var->variantId() + "' is already registered");
				return false;
			}

            registry.definitions_list.push_back(meth->variants()[var->variantId()] = ret = var);
			return true;
		}

        bool readProperty(ObjectElementRegistry & registry, const std::vector<std::string> & scope, const std::string & name, const rapidjson::Value & v, std::shared_ptr<Language::Property> & ret, const std::string & error_path, ErrorCollector & ec)
		{
			rapidjson::Value * r;
			if (!getTypeValue(v, r))
			{
				ec << (error_path + ": type is not specified");
				return false;
			}
			std::shared_ptr<Language::Type> type;
			if (!readType(registry, *r, type, error_path, ec))
				return false;

			bool read_only = false;
			JSONTools::getValue(v, "readonly", read_only);

            if (registry.definitions.count(name))
			{
				ec << (error_path + ": name '" + name + "' is already registered");
				return false;
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, error_path, ec))
				return false;

            registry.definitions[name] = ret = std::make_shared<Language::Property>(type, scope, name, read_only, doc);
            registry.definitions_list.push_back(ret);
			return true;
		}

        bool readObject(const std::string & baseLoggerName, BaseElementRegistry & parentRegistry, const std::vector<std::string> & scope, const std::string & name, const rapidjson::Value & v, Language::Object::Ptr & ret, const std::string & error_path, ErrorCollector & ec)
		{
            ObjectElementRegistry registry(parentRegistry.interfaceRegistry);
            registry.path = registry.path + "." + name;

            auto loggerName = appendLoggerName(baseLoggerName, name);

            Language::DocumentationProvider::Documentation doc;
            if (!readDocumentation(v, doc, error_path, ec))
                return false;

            if(parentRegistry.interfaceRegistry->embedded_types.count(name))
            {
                ec << (error_path + ": '" + name + "' has been already registered as embedded type");
                return false;
            }

            if(parentRegistry.types.count(name))
            {
                if(!(ret = std::dynamic_pointer_cast<Language::Object>(parentRegistry.types[name])))
                {
                    ec << (error_path + ": '" + name + "' has been already registered as a different type");
                    return false;
                }
                if(ret->initialized())
                {
                    ec << (error_path + ": object '" + name + "' has been already registered");
                    return false;
                }
                ret->init(scope, doc, loggerName);
            }
            else
                parentRegistry.types[name] = ret = std::make_shared<Language::Object>(name, scope, doc, loggerName);

            parentRegistry.definitions[name] = ret;
            parentRegistry.definitions_list.push_back(ret);

			rapidjson::Value * b;
			if (JSONTools::getValue(v, "body", b))
			{
				if (!b->IsArray())
				{
					ec << ("body of object '" + name + "' is not array");
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
                            if (!readObject(loggerName, registry, _scope, e_name, e, tmp, name + "." + e_name, ec))
                                return false;
                        }
                        else if (e_nature == "property")
                        {
							std::shared_ptr<Language::Property> tmp;
							auto _scope = scope;
							_scope.push_back(name);
                            if (!readProperty(registry, _scope, e_name, e, tmp, error_path + "." + e_name, ec))
								return false;
						}
						else if (e_nature == "method")
						{
							Language::Method::Variant::Ptr tmp;
							auto _scope = scope;
							_scope.push_back(name);
                            if (!readMethod(registry, _scope, e_name, e, tmp, error_path + "." + e_name, ec))
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

            ret->setDefinitions(registry.definitions_list);

            return true;
		}

        bool readInterface(const std::string & baseLoggerName, const std::vector<std::string> & scope, const std::string & name, const rapidjson::Value & v, std::shared_ptr<Language::Interface> & ret, ErrorCollector & ec)
		{
            InterfaceElementRegistry registry;
			registry.path = name;

            auto loggerName = baseLoggerName;

			rapidjson::Value * b;
			if (JSONTools::getValue(v, "body", b))
			{
				if (!b->IsArray())
				{
					ec << ("body of interface '" + name + "' is not array");
					return false;
				}

                //prebuild registry
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

                        if (e_nature == "object")
                        {
                            if(registry.types.count(e_name))
                            {
                                if(!(std::dynamic_pointer_cast<Language::Object>(registry.types[e_name])))
                                {
                                    ec << (e_name+": type definition is alread found but not as an object'");
                                    return false;
                                }
                            }
                            else
                            {
                                auto obj = std::make_shared<Language::Object>(e_name);
                                registry.types[e_name] = obj;
                            }
                        }
                    }
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
							auto _scope = scope;
							_scope.push_back(name);
							if (!readTypeDefinition(registry, _scope, e_name, e, tmp, name + "." + e_name, ec))
								return false;
						}
						else if (e_nature == "object")
						{
							std::shared_ptr<Language::Object> tmp;
							auto _scope = scope;
							_scope.push_back(name);
                            if (!readObject(loggerName, registry, _scope, e_name, e, tmp, name + "." + e_name, ec))
								return false;
						}
						else if (e_nature == "function")
						{
							Language::Function::Variant::Ptr tmp;
							auto _scope = scope;
							_scope.push_back(name);
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
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, name, ec))
				return false;

            ret = std::make_shared<Language::Interface>(name, registry.definitions_list, scope, doc, loggerName);

			return true;
		}

        bool readModule(const std::string & baseLoggerName, const std::string name, const rapidjson::Value & v, std::shared_ptr<Language::Module> & ret, ErrorCollector & ec)
		{
            InterfaceElementRegistry registry;
			registry.path = name;

			Language::TopLevel::Info info;

			rapidjson::Value * info_v;
			if (JSONTools::getValue(v, "info", info_v))
			{
				if (!info_v->IsArray())
				{
					ec << ("info of module '" + name + "' is not array");
					return false;
				}
				for (rapidjson::SizeType i(0), l(info_v->Size()); i < l; ++i)
				{
					auto & e = (*info_v)[i];
					if (!e.IsNull())
					{
						std::string e_name;
						if (!getName(e, e_name))
						{
							ec << ("name of info #" + std::to_string(i) + " of module '" + name + "' is not specified");
							return false;
						}

						JSONTools::getValue(e, "value", info[e_name]);
					}
				}
			}

			std::map<std::string, Language::TopLevel::Ptr> elements;

			rapidjson::Value * body_v;
			if (JSONTools::getValue(v, "body", body_v))
			{
				if (!body_v->IsArray())
				{
					ec << ("body of module '" + name + "' is not array");
					return false;
				}
				for (rapidjson::SizeType i(0), l(body_v->Size()); i < l; ++i)
				{
					auto & e = (*body_v)[i];
					if (!e.IsNull())
					{
						std::string e_name;
						if (!getName(e, e_name))
						{
							ec << ("name of element #" + std::to_string(i) + " of module '" + name + "' is not specified");
							return false;
						}

						std::shared_ptr<Language::TopLevel> tmp;
                        if (!readTopLevel(baseLoggerName, e_name, e, tmp, ec))
							return false;
						elements[e_name] = tmp;
					}
				}
			}

			std::vector<Language::TopLevel::Ptr> _elements(elements.size());
			{
				size_t i(0);
				for (auto & d : elements)
					_elements[i++] = d.second;
			}

			Language::DocumentationProvider::Documentation doc;
			if (!readDocumentation(v, doc, name, ec))
				return false;

			ret = std::make_shared<Language::Module>(name, _elements, doc, info);

			return true;
		}

        bool readTopLevel(const std::string & baseLoggerName, const std::string name, const rapidjson::Value & v, std::shared_ptr<Language::TopLevel> & ret, ErrorCollector & ec)
		{
			std::string nature;
			if (!getNature(v, nature))
			{
				ec << "nature of the toplevel element is not specified";
				return false;
			}

            auto loggerName = appendLoggerName(baseLoggerName, name);

			if (nature == "interface")
			{
				std::shared_ptr<Language::Interface> tmp;
				std::vector<std::string> scope;
                if (!readInterface(loggerName, scope, name, v, tmp, ec))
					return false;
				ret = tmp;
			}
			else if (nature == "module")
			{
				std::shared_ptr<Language::Module> tmp;
                if (!readModule(loggerName, name, v, tmp, ec))
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

		bool read(const rapidjson::Value & root, ErrorCollector & ec)
		{
			if (!root.IsObject())
			{
				ec << "toplevel element is not object";
				return false;
			}

			topLevels.clear();

			std::string name;
			if (!getName(root, name))
			{
				ec << "name of interface is not toplevel element";
				return false;
			}

			std::shared_ptr<Language::TopLevel> tmp;
            if (!readTopLevel(std::string(), name, root, tmp, ec))
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
		if (!priv->read(priv->json_stream, ec))
			return false;

		return completeInfo(ec);
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
        (void)ret;
		JSONReader p(json_stream);
		if (!p.read(ec))
			return false;

		return writer->write(&p, ec);
	}

}
