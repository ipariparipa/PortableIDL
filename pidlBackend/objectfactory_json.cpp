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

#include "include/pidlBackend/objectfactory_json.h"
#include "include/pidlBackend/cppcodegenfactory_json.h"
#include "include/pidlBackend/cscodegenfactory_json.h"
#include "include/pidlBackend/writerfactory_json.h"
#include "include/pidlBackend/readerfactory_json.h"
#include "include/pidlBackend/cppwriter.h"
#include "include/pidlBackend/xmlreader.h"
#include "include/pidlBackend/jsonreader.h"
#include "include/pidlBackend/cscodegen.h"
#include "include/pidlBackend/cswriter.h"
#include "include/pidlBackend/jsonwriter.h"
#include "include/pidlBackend/json_stl_codegen.h"
#include "include/pidlBackend/json_cscodegen.h"
#include "include/pidlBackend/operationfactory_json.h"

#include "include/pidlBackend/language.h"

#include <pidlCore/errorcollector.h>
#include <pidlCore/jsontools.h>

#include <map>
#include <string>
#include <iostream>
#include <fstream>

namespace PIDL {

	struct ObjectFactoryRegistry_JSON::Priv
	{
		std::map<std::string, std::list<std::shared_ptr<ObjectFactory_JSON>>> data;
	};

	ObjectFactoryRegistry_JSON::ObjectFactoryRegistry_JSON() : priv(new Priv)
	{ }

	ObjectFactoryRegistry_JSON::~ObjectFactoryRegistry_JSON()
	{
		delete priv;
	}

	void ObjectFactoryRegistry_JSON::add(const std::shared_ptr<ObjectFactory_JSON> & fac)
	{
		priv->data[fac->objectType()].push_back(fac);
	}

	const std::list<std::shared_ptr<ObjectFactory_JSON>> & ObjectFactoryRegistry_JSON::_get(const char * type) const
	{
		static const std::list<std::shared_ptr<ObjectFactory_JSON>> empty;
		return priv->data.count(type) ? priv->data[type] : empty;
	}

	std::shared_ptr<ObjectFactory_JSON> ObjectFactoryRegistry_JSON::_getValid(const char * type, const rapidjson::Value & value) const
	{
		auto & lst = _get(type);
		for(auto & f : lst)
			if(f->isValid(value))
				return f;
		return std::shared_ptr<ObjectFactory_JSON>();
	}

	namespace Factories_JSON
	{
		struct Context
		{
			Context(const std::shared_ptr<ObjectFactoryRegistry_JSON> & facreg_, const std::shared_ptr<ObjectRegistry> & objreg_) :
				facreg(facreg_),
				objreg(objreg_)
			{ }

			std::shared_ptr<ObjectFactoryRegistry_JSON> facreg;
			std::shared_ptr<ObjectRegistry> objreg;

			static bool getStringOptional(const rapidjson::Value & r, const char * name, /*in-out*/ std::string & ret, ErrorCollector & ec)
			{
				rapidjson::Value * v;
				if (!JSONTools::getValue(r, name, v) || v->IsNull())
					return true;

				if (!v->IsString())
				{
					ec << std::string() + "value '" + name + "' is not string";
					return false;
				}

				ret = v->GetString();
				return true;
			}

			static bool getName(const rapidjson::Value & r, std::string & ret, ErrorCollector & ec)
			{
				if (!JSONTools::getValue(r, "name", ret))
				{
					ec << "value 'name' is not found or invalid";
					return false;
				}
				return true;
			}

			template<class Object_T, class Factory_T>
			bool _get_object(const char * type_name, const rapidjson::Value & value, std::shared_ptr<Object_T> & ret, ErrorCollector & ec)
			{
				if(value.IsString())
				{
					if(!(ret = this->objreg->get<Object_T>(type_name, value.GetString())))
					{
						ec << std::string() + "the referred object '"+value.GetString()+"' is not registered";
						return false;
					}
				}
				else if (value.IsObject())
				{
					auto fac = this->facreg->getValid<Factory_T>(type_name, value);
					if(!fac)
					{
						ec << "could not find valid factory";
						return false;
					}

					if(!fac->build(value, ret, ec))
						return false;

					std::string name;
					if (JSONTools::getValue(value, "name", name) && !objreg->add(name.c_str(), ret, ec))
						return false;
				}
				else
				{
					ec << "value is not string and not object";
					return false;
				}

				return true;
			}

			bool get_object(const rapidjson::Value & value, std::shared_ptr<CPPCodeGen> & ret, ErrorCollector & ec)
			{
				return _get_object<CPPCodeGen, CPPCodeGenFactory_JSON>(PIDL_OBJECT_TYPE__CPP_CODEGEN, value, ret, ec);
			}

			bool get_object(const rapidjson::Value & value, std::shared_ptr<CPPCodeGenHelper> & ret, ErrorCollector & ec)
			{
				return _get_object<CPPCodeGenHelper, CPPCodeGenHelperFactory_JSON>(PIDL_OBJECT_TYPE__CPP_CODEGEN_HELPER, value, ret, ec);
			}

			bool get_object(const rapidjson::Value & value, std::shared_ptr<CPPCodeGenLogging> & ret, ErrorCollector & ec)
			{
				return _get_object<CPPCodeGenLogging, CPPCodeGenLoggingFactory_JSON>(PIDL_OBJECT_TYPE__CPP_CODEGEN_LOGGING, value, ret, ec);
			}

			bool get_object(const rapidjson::Value & value, std::shared_ptr<Writer> & ret, ErrorCollector & ec)
			{
				return _get_object<Writer, WriterFactory_JSON>(PIDL_OBJECT_TYPE__WRITER, value, ret, ec);
			}

			bool get_object(const rapidjson::Value & value, std::shared_ptr<Reader> & ret, ErrorCollector & ec)
			{
				return _get_object<Reader, ReaderFactory_JSON>(PIDL_OBJECT_TYPE__READER, value, ret, ec);
			}

			bool get_object(const rapidjson::Value & value, std::shared_ptr<CSCodeGen> & ret, ErrorCollector & ec)
			{
				return _get_object<CSCodeGen, CSCodeGenFactory_JSON>(PIDL_OBJECT_TYPE__CS_CODEGEN, value, ret, ec);
			}

			bool get_object(const rapidjson::Value & value, std::shared_ptr<CSCodeGenHelper> & ret, ErrorCollector & ec)
			{
				return _get_object<CSCodeGenHelper, CSCodeGenHelperFactory_JSON>(PIDL_OBJECT_TYPE__CS_CODEGEN_HELPER, value, ret, ec);
			}

			bool get_object(const rapidjson::Value & value, std::shared_ptr<CSCodeGenLogging> & ret, ErrorCollector & ec)
			{
				return _get_object<CSCodeGenLogging, CSCodeGenLoggingFactory_JSON>(PIDL_OBJECT_TYPE__CS_CODEGEN_LOGGING, value, ret, ec);
			}

			template<class Operation_T>
			bool get_object(const rapidjson::Value & value, std::shared_ptr<Operation_T> & ret, ErrorCollector & ec)
			{
				std::shared_ptr<Operation> op;
				if (!_get_object<Operation, OperationFactory_JSON>(PIDL_OBJECT_TYPE__OPERATION, value, op, ec))
					return false;
				ret = std::dynamic_pointer_cast<Operation_T>(op);
				return true;
			}

			bool get_out(const rapidjson::Value & r, std::shared_ptr<std::ostream> & o, std::string & filename, ErrorCollector & ec)
			{
				std::string out_str = "file";
				if (!getStringOptional(r, "out", out_str, ec))
					return false;

				if (out_str == "stdout")
					o = std::shared_ptr<std::ostream>(&std::cout, [](void*) {});
				else if (out_str == "file")
				{
					if (!JSONTools::getValue(r, "filename", filename))
					{
						ec << "value 'filename' is not found or invalid";
						return false;
					}

					auto file = std::make_shared<std::ofstream>();
					file->open(filename, std::ios::binary);
					if (file->fail())
					{
						ec << std::string() << "could not open file '" + filename + "'";
						return false;
					}

					o = file;
				}
				else
				{
					ec << std::string() << "unsupported output '" + out_str + "'";
					return false;
				}

				return true;
			}

			static bool get_data(const rapidjson::Value & r, std::string & str, ErrorCollector & ec)
			{
				rapidjson::Value * v;
				if (JSONTools::getValue(r, "data", v))
				{
					if (!JSONTools::getValue(r, str))
					{
						ec << "value 'data' is invalid";
						return false;
					}
				}
				else if (JSONTools::getValue(r, "filename", v))
				{
					std::string filename;
					if (!JSONTools::getValue(*v, filename))
					{
						ec << "value 'filename' is invalid";
						return false;
					}
					if (!Reader::readFromFile(filename, str, ec))
						return false;
				}
				else
				{
					ec << "no valid data source is specified";
					return false;
				}

				return true;
			}

			static bool get_include(rapidjson::Value & r, CPPCodeGenHelper::Include & incl, ErrorCollector & ec)
			{
				if (!r.IsObject())
				{
					ec << "value of include is not object";
					return false;
				}

				std::string type_str;
				if (!JSONTools::getValue(r, "type", type_str))
				{
					ec << "value 'type' is not found or invalid";
					return false;
				}
				if (type_str == "global")
					incl.first = CPPCodeGenHelper::IncludeType::GLobal;
				else if (type_str == "local")
					incl.first = CPPCodeGenHelper::IncludeType::Local;
				else
				{
					ec << "invalid type: '" + type_str + "'";
					return false;
				}
				if (!JSONTools::getValue(r, "path", incl.second))
				{
					ec << "value 'type' is not found or invalid";
					return false;
				}

				return true;
			}

		};


		class CPPWriterFactory : public WriterFactory_JSON
		{
			Context ctx;
		public:
			CPPWriterFactory(const Context & ctx_) :
				WriterFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~CPPWriterFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<Writer> & ret, ErrorCollector & ec) override
			{
				if(!isValid(r))
				{
					ec.add(-1, std::string() + "unexpected: json object is invalid");
					return false;
				}

				std::shared_ptr<CPPCodeGen> codegen;
				rapidjson::Value * codegen_v;
				if (!JSONTools::getValue(r, "codegen", codegen_v))
				{
					ec << "value 'codegen' is not found or invalid";
					return false;
				}

				if(!ctx.get_object(*codegen_v, codegen, ec))
					return false;

				std::string mode_str;
				if (!JSONTools::getValue(r, "mode", mode_str))
				{
					ec << "value 'mode' is not found or invalid";
					return false;
				}

				CPPWriter::Mode mode;
				if (mode_str == "include")
					mode = CPPWriter::Mode::Include;
				else if (mode_str == "source")
					mode = CPPWriter::Mode::Source;
				else if (mode_str == "combo")
					mode = CPPWriter::Mode::Combo;
				else
				{
					ec << std::string() << "unsupported mode '" + mode_str + "'";
					return false;
				}

				std::string role_str;
				if (!JSONTools::getValue(r, "role", role_str))
				{
					ec << "value 'mode' is not found or invalid";
					return false;
				}

				CPPWriter::Role role;
				if (role_str == "server")
					role = CPPWriter::Role::Server;
				else if (role_str == "client")
					role = CPPWriter::Role::Client;
				else
				{
					ec << std::string() << "unsupported role '" + role_str + "'";
					return false;
				}

				std::shared_ptr<std::ostream> o;
				std::string filename;
				if (!ctx.get_out(r, o, filename, ec))
					return false;

				std::string name;
				if (filename.length())
				{
					name = filename;
					if (!ctx.getStringOptional(r, "name", name, ec))
						return false;
				}
				else if (!ctx.getName(r, name, ec))
						return false;


				ret = std::make_shared<CPPWriter>(mode, role, codegen, o, name);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "c++";
			}
		};

		class CSWriterFactory : public WriterFactory_JSON
		{
			Context ctx;
		public:
			CSWriterFactory(const Context & ctx_) :
				WriterFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~CSWriterFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<Writer> & ret, ErrorCollector & ec) override
			{
				if(!isValid(r))
				{
					ec.add(-1, std::string() + "unexpected: json object is invalid");
					return false;
				}

				std::shared_ptr<CSCodeGen> codegen;
				rapidjson::Value * codegen_v;
				if(!JSONTools::getValue(r, "codegen", codegen_v))
				{
					ec << "value 'codegen' is not found or invalid";
					return false;
				}
				if (!ctx.get_object(*codegen_v, codegen, ec))
					return false;

				std::string role_str;
				if (!JSONTools::getValue(r, "role", role_str))
				{
					ec << "value 'role' is not found or invalid";
					return false;
				}
				CSWriter::Role role;
				if (role_str == "server")
					role = CSWriter::Role::Server;
				else if (role_str == "client")
					role = CSWriter::Role::Client;
				else
				{
					ec << std::string() << "unsupported role '" + role_str + "'";
					return false;
				}

				std::shared_ptr<std::ostream> o;
				std::string filename;
				if (!ctx.get_out(r, o, filename, ec))
					return false;

				ret = std::make_shared<CSWriter>(role, codegen, o);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "c#";
			}
		};

		class JSONReaderFactory : public ReaderFactory_JSON
		{
			Context ctx;
		public:
			JSONReaderFactory(const Context & ctx_) :
				ReaderFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~JSONReaderFactory() = default;

			virtual bool build(const rapidjson::Value & value, std::shared_ptr<Reader> & ret, ErrorCollector & ec) override
			{
				std::string str;
				if (!ctx.get_data(value, str, ec))
					return false;
				ret = std::make_shared<JSONReader>(str);
				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "json";
			}
		};

		class XMLReaderFactory : public ReaderFactory_JSON
		{
			Context ctx;
		public:
			XMLReaderFactory(const Context & ctx_) :
				ReaderFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~XMLReaderFactory() = default;

			virtual bool build(const rapidjson::Value & value, std::shared_ptr<Reader> & ret, ErrorCollector & ec) override
			{
				std::string str;
				if (!ctx.get_data(value, str, ec))
					return false;
				ret = std::make_shared<XMLReader>(str);
				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "xml";
			}
		};

		class CPPBasicCodegenHelperFactory : public CPPCodeGenHelperFactory_JSON
		{
			Context ctx;
		public:
			CPPBasicCodegenHelperFactory(const Context & ctx_) :
				CPPCodeGenHelperFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~CPPBasicCodegenHelperFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<CPPCodeGenHelper> & ret, ErrorCollector & ec) override
			{
				if(!isValid(r))
				{
					ec.add(-1, std::string() + "unexpected: json object is invalid");
					return false;
				}

				std::vector<CPPCodeGenHelper::Include> includes;
				rapidjson::Value * includes_v;
				if (JSONTools::getValue(r, "includes", includes_v))
				{
					if (!includes_v->IsArray())
					{
						ec << "value 'includes' is not array";
						return false;
					}
					includes.resize(includes_v->Size());
					bool has_error = false;
					for (rapidjson::SizeType i = 0, l = includes_v->Size(); i < l; ++i)
					{
						if (!ctx.get_include((*includes_v)[i], includes[i], ec))
							has_error = true;
					}
					if (has_error)
						return false;
				}

				ret = std::make_shared<CPPBasicCodegenHelper>(includes);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "basic";
			}
		};

		class CSBasicCodegenHelperFactory : public CSCodeGenHelperFactory_JSON
		{
			Context ctx;
		public:
			CSBasicCodegenHelperFactory(const Context & ctx_) :
				CSCodeGenHelperFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~CSBasicCodegenHelperFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<CSCodeGenHelper> & ret, ErrorCollector & ec) override
			{
				if(!isValid(r))
				{
					ec.add(-1, std::string() + "unexpected: json object is invalid");
					return false;
				}

				ret = std::make_shared<CSBasicCodegenHelper>();

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "basic";
			}
		};

		class JSON_STL_CodeGenFactory : public CPPCodeGenFactory_JSON
		{
			Context ctx;
		public:
			JSON_STL_CodeGenFactory(const Context & ctx_) :
				CPPCodeGenFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~JSON_STL_CodeGenFactory() = default;

			virtual bool build(const rapidjson::Value & value, std::shared_ptr<CPPCodeGen> & ret, ErrorCollector & ec) override
			{
				if(!isValid(value))
				{
					ec.add(-1, std::string() + "unexpected: json object is invalid");
					return false;
				}

				std::shared_ptr<CPPCodeGenHelper> helper;
				rapidjson::Value * helper_v;
				if(JSONTools::getValue(value, "helper", helper_v))
				{
					if(!ctx.get_object(*helper_v, helper, ec))
						return false;
				}
				else
					helper = std::make_shared<CPPBasicCodegenHelper>();

				ret = std::make_shared<JSON_STL_CodeGen>(helper);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "json_stl";
			}
		};

		class JSON_CSCodeGenFactory : public CSCodeGenFactory_JSON
		{
			Context ctx;
		public:
			JSON_CSCodeGenFactory(const Context & ctx_) :
				CSCodeGenFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~JSON_CSCodeGenFactory() = default;

			virtual bool build(const rapidjson::Value & value, std::shared_ptr<CSCodeGen> & ret, ErrorCollector & ec) override
			{
				if(!isValid(value))
				{
					ec.add(-1, std::string() + "unexpected: json object is invalid");
					return false;
				}

				std::shared_ptr<CSCodeGenHelper> helper;
				rapidjson::Value * helper_v;
				if(JSONTools::getValue(value, "helper", helper_v))
				{
					if(!ctx.get_object(*helper_v, helper, ec))
						return false;
				}
				else
					helper = std::make_shared<CSBasicCodegenHelper>();

				ret = std::make_shared<JSON_CSCodeGen>(helper);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "json";
			}
		};

		class CustomCPPHelperFactory : public CPPCodeGenHelperFactory_JSON
		{
			Context ctx;

			class CustomCPPHelper : public CPPCodeGenHelper
			{
			public:
				struct Settings
				{
					char tabChar = '\t';
					short tabLength = 1;

					std::shared_ptr<CPPCodeGenLogging> logging;
					std::vector<Include> includes;
					Include coreIncludePath = { IncludeType::GLobal, "pidlCore" };

					std::string moduleSuffix;
					std::string interfaceSuffix;
				};

				CustomCPPHelper(const Settings & settings_) : settings(settings_)
				{ }

				virtual ~CustomCPPHelper() = default;

				virtual short tabDefinition(char & ch) const override
				{
					ch = settings.tabChar;
					return settings.tabLength;
				}

				virtual std::shared_ptr<CPPCodeGenLogging> logging() const override
				{
					return settings.logging;
				}

				virtual std::vector<Include> includes() const override
				{
					return settings.includes;
				}

				virtual Include coreIncludePath() const override
				{
					return settings.coreIncludePath;
				}

				virtual std::string getName(const Language::TopLevel * t) const
				{
					if (dynamic_cast<const Language::Interface*>(t))
						return settings.interfaceSuffix.length() ? (t->name() + settings.interfaceSuffix) : t->name();

					if (dynamic_cast<const Language::Module*>(t))
						return settings.moduleSuffix.length() ? (t->name() + settings.moduleSuffix) : t->name();

					return t->name();
				}

			private:
				Settings settings;
			};

		public:
			CustomCPPHelperFactory(const Context & ctx_) :
				CPPCodeGenHelperFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~CustomCPPHelperFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<CPPCodeGenHelper> & ret, ErrorCollector & ec) override
			{
				if(!isValid(r))
				{
					ec.add(-1, std::string() + "unexpected: json object is invalid");
					return false;
				}

				CustomCPPHelper::Settings settings;

				rapidjson::Value * v;
				if (JSONTools::getValue(r, "tab", v))
				{
					if (!v->IsObject())
					{
						ec << "value 'tab' is not object";
						return false;
					}
					std::string tab_str(settings.tabChar, 1);
					ctx.getStringOptional(*v, "char", tab_str, ec);

					if (tab_str.length() != 1)
					{
						ec << "invalid char definition: '" + tab_str + "'";
						return false;
					}
					settings.tabChar = tab_str[0];

					rapidjson::Value * v_;
					if (JSONTools::getValue(*v, "length", v_))
					{
						if (!v_->IsNumber())
						{
							ec << "value 'tab.length' is not number";
							return false;
						}
						settings.tabLength = (short)v_->GetInt();
					}
				}

				if (JSONTools::getValue(r, "logging", v))
				{
					if(!ctx.get_object(*v, settings.logging, ec))
						return false;
				}
				else
					settings.logging = std::make_shared<CPPVoidLogging>();

				if (JSONTools::getValue(r, "includes", v))
				{
					if (!v->IsArray())
					{
						ec << "value 'includes' is not array";
						return false;
					}

					settings.includes.resize(v->Size());
					bool has_error = false;
					for (rapidjson::SizeType i = 0, l = v->Size(); i < l; ++i)
					{
						if (!ctx.get_include((*v)[i], settings.includes[i], ec))
							has_error = true;
					}
					if (has_error)
						return false;
				}

				if (JSONTools::getValue(r, "coreIncludePath", v))
				{
					if (!v->IsObject())
					{
						ec << "value 'coreIncludePath' is not object";
						return false;
					}
					if (!ctx.get_include(*v, settings.coreIncludePath, ec))
						return false;
				}

				if (!ctx.getStringOptional(r, "moduleSuffix", settings.moduleSuffix, ec))
					return false;

				if (!ctx.getStringOptional(r, "interfaceSuffix", settings.interfaceSuffix, ec))
					return false;

				ret = std::make_shared<CustomCPPHelper>(settings);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "custom";
			}
		};

		class CustomCSHelperFactory : public CSCodeGenHelperFactory_JSON
		{
			Context ctx;

			class CustomCSHelper : public CSCodeGenHelper
			{
			public:
				struct Settings
				{
					char tabChar = '\t';
					short tabLength = 1;

					std::shared_ptr<CSCodeGenLogging> logging;

					std::string moduleSuffix;
					std::string interfaceSuffix;
				};

				CustomCSHelper(const Settings & settings_) : settings(settings_)
				{ }

				virtual ~CustomCSHelper() = default;

				virtual short tabDefinition(char & ch) const override
				{
					ch = settings.tabChar;
					return settings.tabLength;
				}

				virtual std::shared_ptr<CSCodeGenLogging> logging() const override
				{
					return settings.logging;
				}

				virtual std::string getName(const Language::TopLevel * t) const
				{
					if (dynamic_cast<const Language::Interface*>(t))
						return settings.interfaceSuffix.length() ? (t->name() + settings.interfaceSuffix) : t->name();

					if (dynamic_cast<const Language::Module*>(t))
						return settings.moduleSuffix.length() ? (t->name() + settings.moduleSuffix) : t->name();

					return t->name();
				}

			private:
				Settings settings;
			};

		public:
			CustomCSHelperFactory(const Context & ctx_) :
				CSCodeGenHelperFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~CustomCSHelperFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<CSCodeGenHelper> & ret, ErrorCollector & ec) override
			{
				if(!isValid(r))
				{
					ec.add(-1, std::string() + "unexpected: json object is invalid");
					return false;
				}

				CustomCSHelper::Settings settings;

				rapidjson::Value * v;
				if (JSONTools::getValue(r, "tab", v))
				{
					if (!v->IsObject())
					{
						ec << "value 'tab' is not object";
						return false;
					}
					std::string tab_str(settings.tabChar, 1);
					ctx.getStringOptional(*v, "char", tab_str, ec);

					if (tab_str.length() != 1)
					{
						ec << "invalid char definition: '" + tab_str + "'";
						return false;
					}
					settings.tabChar = tab_str[0];

					rapidjson::Value * v_;
					if (JSONTools::getValue(*v, "length", v_))
					{
						if (!v_->IsNumber())
						{
							ec << "value 'tab.length' is not number";
							return false;
						}
						settings.tabLength = (short)v_->GetInt();
					}
				}

				if (JSONTools::getValue(r, "logging", v))
				{
					if(!ctx.get_object(*v, settings.logging, ec))
						return false;
				}
				else
					settings.logging = std::make_shared<CSVoidLogging>();

				if (!ctx.getStringOptional(r, "moduleSuffix", settings.moduleSuffix, ec))
					return false;

				if (!ctx.getStringOptional(r, "interfaceSuffix", settings.interfaceSuffix, ec))
					return false;

				ret = std::make_shared<CustomCSHelper>(settings);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string type_str;
				if(!value.IsObject() || !JSONTools::getValue(value, "type", type_str))
					return false;

				return type_str == "custom";
			}
		};

		class JSONWriterFactory : public WriterFactory_JSON
		{
			Context ctx;

		public:
			JSONWriterFactory(const Context & ctx_) :
				WriterFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~JSONWriterFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<Writer> & ret, ErrorCollector & ec) override
			{
				if (!isValid(r))
				{
					ec << "unexpected: json object is invalid";
					return false;
				}

				std::shared_ptr<std::ostream> o;
				std::string filename;
				if (!ctx.get_out(r, o, filename, ec))
					return false;

				ret = std::make_shared<JSONWriter>(o);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string nature_str;
				if (!value.IsObject() || !JSONTools::getValue(value, "type", nature_str))
					return false;

				return nature_str == "json";
			}

		};

		class ReadFactory : public OperationFactory_JSON
		{
			Context ctx;

		public:
			ReadFactory(const Context & ctx_) :
				OperationFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~ReadFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<Operation> & ret, ErrorCollector & ec) override
			{
				if (!isValid(r))
				{
					ec << "unexpected: json object is invalid";
					return false;
				}

				std::shared_ptr<Reader> reader;
				if (!ctx.get_object(r, reader, ec))
					return false;

				ret = std::make_shared<Read>(reader);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string nature_str;
				if (!value.IsObject() || !JSONTools::getValue(value, "nature", nature_str))
					return false;

				return nature_str == "read";
			}

		};
		
		class WriteFactory : public OperationFactory_JSON
		{
			Context ctx;

		public:
			WriteFactory(const Context & ctx_) :
				OperationFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~WriteFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<Operation> & ret, ErrorCollector & ec) override
			{
				if (!isValid(r))
				{
					ec << "unexpected: json object is invalid";
					return false;
				}

				std::shared_ptr<Writer> writer;
				if (!ctx.get_object(r, writer, ec))
					return false;

				std::shared_ptr<Read> read_op;
				rapidjson::Value * read_v;
				if (JSONTools::getValue(r, "read", read_v))
				{
					if (!ctx.get_object(*read_v, read_op, ec))
						return false;
				}

				std::shared_ptr<Reader> reader;
				rapidjson::Value * reader_v;
				if (JSONTools::getValue(r, "reader", reader_v))
				{
					if (!ctx.get_object(*reader_v, reader, ec))
						return false;
				}
				else if (read_op)
					reader = read_op->reader();
				else
				{
					ec << "read operation or reader must be defined";
					return false;
				}

				ret = std::make_shared<Write>(reader, read_op, writer);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string nature_str;
				if (!value.IsObject() || !JSONTools::getValue(value, "nature", nature_str))
					return false;

				return nature_str == "write";
			}

		};

		class OperationGroupFactory : public OperationFactory_JSON
		{
			Context ctx;

		public:
			OperationGroupFactory(const Context & ctx_) :
				OperationFactory_JSON(),
				ctx(ctx_)
			{ }

			virtual ~OperationGroupFactory() = default;

			virtual bool build(const rapidjson::Value & r, std::shared_ptr<Operation> & ret, ErrorCollector & ec) override
			{
				if (!isValid(r))
				{
					ec << "unexpected: json object is invalid";
					return false;
				}

				rapidjson::Value * ops_v;
				if (!JSONTools::getValue(r, "operations", ops_v))
				{
					ec << "value 'reader' is not found";
					return false;
				}
				if (!ops_v->IsArray())
				{
					ec << "value 'operations' is not array";
					return false;
				}

				std::vector<std::shared_ptr<Operation>> ops(ops_v->Size());
				bool has_error = false;
				for (rapidjson::SizeType i = 0, l = ops_v->Size(); i < l; ++i)
				{
					if (!ctx.get_object((*ops_v)[i], ops[i], ec))
						has_error = true;
				}
				if (has_error)
					return false;

				ret = std::make_shared<OperationGroup>(ops);

				return true;
			}

			virtual bool isValid(const rapidjson::Value & value) const override
			{
				std::string nature_str;
				if (!value.IsObject() || !JSONTools::getValue(value, "nature", nature_str))
					return false;

				return nature_str == "group";
			}

		};
	}

	//static
	std::shared_ptr<ObjectFactoryRegistry_JSON> ObjectFactoryRegistry_JSON::build(const std::shared_ptr<ObjectRegistry> & objreg)
	{
		auto ret = std::make_shared<ObjectFactoryRegistry_JSON>();

		Factories_JSON::Context ctx(ret, objreg);

		ret->add(std::make_shared<Factories_JSON::CPPWriterFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::CSWriterFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::JSONReaderFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::XMLReaderFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::CPPBasicCodegenHelperFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::CSBasicCodegenHelperFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::JSON_STL_CodeGenFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::JSON_CSCodeGenFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::CustomCPPHelperFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::CustomCSHelperFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::JSONWriterFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::OperationGroupFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::ReadFactory>(ctx));
		ret->add(std::make_shared<Factories_JSON::WriteFactory>(ctx));

		return ret;
	}

}
