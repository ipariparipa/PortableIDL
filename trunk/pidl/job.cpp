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

#include <pidlCore/errorcollector.h>
#include <pidlCore/jsontools.h>

#include <pidlBackend/jsonreader.h>
#include <pidlBackend/jsonwriter.h>
#include <pidlBackend/xmlreader.h>
#include <pidlBackend/cppwriter.h>
#include <pidlBackend/cswriter.h>
#include <pidlBackend/json_stl_codegen.h>
#include <pidlBackend/json_cscodegen.h>
#include <pidlBackend/language.h>

#include <rapidjson/document.h>

#include <map>
#include <iostream>
#include <fstream>

namespace PIDL {

	struct Job::Priv
	{
		Priv(const std::shared_ptr<Operation> & op_) : op(op_)
		{ }

		std::shared_ptr<Operation> op;

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

		static bool getNatureOptional(const rapidjson::Value & r, /*in-out*/ std::string & ret, ErrorCollector & ec)
		{
			return getStringOptional(r, "nature", ret, ec);
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

		struct Context
		{
			std::map<std::string /*name*/, std::shared_ptr<Reader>> readers;
			std::map<std::string /*name*/, std::shared_ptr<CPPCodeGenHelper>> cpp_helpers;
			std::map<std::string /*name*/, std::shared_ptr<CSCodeGenHelper>> cs_helpers;

			static bool getInclude(rapidjson::Value & r, CPPCodeGenHelper::Include & incl, ErrorCollector &ec)
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

			struct CustomCPPHelper : public CPPCodeGenHelper
			{
				virtual short tabDefinition(char & ch) const override
				{
					ch = _tabChar;
					return _tabLength;
				}

				virtual std::shared_ptr<CPPCodeGenLogging> logging() const override
				{
					return _logging;
				}

				virtual std::vector<Include> includes() const override
				{
					return _includes;
				}

				virtual Include coreIncludePath() const override 
				{
					return _coreIncludePath;
				}

				virtual std::string getName(const Language::TopLevel * t) const
				{
					if (dynamic_cast<const Language::Interface*>(t))
						return _interfaceSuffix.length() ? (t->name() + _interfaceSuffix) : t->name();

					if (dynamic_cast<const Language::Module*>(t))
						return _moduleSuffix.length() ? (t->name() + _moduleSuffix) : t->name();

					return t->name();
				}

				bool build(const rapidjson::Value & r, ErrorCollector & ec)
				{
					rapidjson::Value * v;
					if (JSONTools::getValue(r, "tab", v))
					{
						if (!v->IsObject())
						{
							ec << "value 'tab' is not object";
							return false;
						}
						std::string tab_str(_tabChar, 1);
						getStringOptional(*v, "char", tab_str, ec);

						if (tab_str.length() != 1)
						{
							ec << "invalid char definition: '" + tab_str + "'";
							return false;
						}
						_tabChar = tab_str[0];

						rapidjson::Value * v_;
						if (JSONTools::getValue(*v, "length", v_))
						{
							if (!v_->IsNumber())
							{
								ec << "value 'tab.length' is not number";
								return false;
							}
							_tabLength = (short)v_->GetInt();
						}
					}

					if (JSONTools::getValue(r, "logging", v))
					{
						ec << "logging settings is not yet supported";
						return false;
					}
					else
						_logging = std::make_shared<CPPVoidLogging>();

					if (JSONTools::getValue(r, "includes", v))
					{
						if (!v->IsArray())
						{
							ec << "value 'includes' is not array";
							return false;
						}

						_includes.resize(v->Size());
						for (rapidjson::SizeType i = 0, l = v->Size(); i < l; ++i)
						{
							if (!getInclude((*v)[i], _includes[i], ec))
								return false;
						}
					}

					if (JSONTools::getValue(r, "coreIncludePath", v))
					{
						if (!v->IsObject())
						{
							ec << "value 'coreIncludePath' is not object";
							return false;
						}
						if (!getInclude(*v, _coreIncludePath, ec))
							return false;
					}

					if (!getStringOptional(r, "moduleSuffix", _moduleSuffix, ec))
						return false;

					if (!getStringOptional(r, "interfaceSuffix", _interfaceSuffix, ec))
						return false;

					return true;
				}

			private:
				char _tabChar = '\t';
				short _tabLength = 1;

				std::shared_ptr<CPPCodeGenLogging> _logging;
				std::vector<Include> _includes;
				Include _coreIncludePath = { IncludeType::GLobal, "pidlCore" };

				std::string _moduleSuffix;
				std::string _interfaceSuffix;
			};

			struct CustomCSHelper : public CSCodeGenHelper
			{
				virtual short tabDefinition(char & ch) const override
				{
					ch = _tabChar;
					return _tabLength;
				}

				virtual std::shared_ptr<CSCodeGenLogging> logging() const override
				{
					return _logging;
				}

				virtual std::string getName(const Language::TopLevel * t) const
				{
					if (dynamic_cast<const Language::Interface*>(t))
						return _interfaceSuffix.length() ? (t->name() + _interfaceSuffix) : t->name();

					if (dynamic_cast<const Language::Module*>(t))
						return _moduleSuffix.length() ? (t->name() + _moduleSuffix) : t->name();

					return t->name();
				}

				bool build(const rapidjson::Value & r, ErrorCollector & ec)
				{
					rapidjson::Value * v;
					if (JSONTools::getValue(r, "tab", v))
					{
						if (!v->IsObject())
						{
							ec << "value 'tab' is not object";
							return false;
						}
						std::string tab_str(_tabChar, 1);
						getStringOptional(*v, "char", tab_str, ec);

						if (tab_str.length() != 1)
						{
							ec << "invalid char definition: '" + tab_str + "'";
							return false;
						}
						_tabChar = tab_str[0];

						rapidjson::Value * v_;
						if (JSONTools::getValue(*v, "length", v_))
						{
							if (!v_->IsNumber())
							{
								ec << "value 'tab.length' is not number";
								return false;
							}
							_tabLength = (short)v_->GetInt();
						}
					}

					if (JSONTools::getValue(r, "logging", v))
					{
						ec << "logging settings is not yet supported";
						return false;
					}
					else
						_logging = std::make_shared<CSVoidLogging>();

					if (!getStringOptional(r, "moduleSuffix", _moduleSuffix, ec))
						return false;

					if (!getStringOptional(r, "interfaceSuffix", _interfaceSuffix, ec))
						return false;

					return true;
				}

			private:
				char _tabChar = '\t';
				short _tabLength = 1;

				std::shared_ptr<CSCodeGenLogging> _logging;

				std::string _moduleSuffix;
				std::string _interfaceSuffix;
			};

			bool build(const rapidjson::Value & r, std::shared_ptr<CPPCodeGenHelper> & h, ErrorCollector & ec)
			{
				if (r.IsString())
				{
					std::string name;
					if (!getName(r, name, ec))
						return false;
					if (!cpp_helpers.count(name))
					{
						ec << "cpp codegen helper '" + name + "' is not defined";
						return false;
					}
					h = cpp_helpers[name];
					return true;
				}
				else if (r.IsObject())
				{
					std::string name;
					if (!getStringOptional(r, "name", name, ec))
						return false;

					std::string type_str = "basic";
					if (!getStringOptional(r, "type", type_str, ec))
						return false;

					if (type_str == "basic")
					{
						std::vector<CPPCodeGenHelper::Include> includes;
						rapidjson::Value * v;
						if (JSONTools::getValue(r, "includes", v))
						{
							if (!v->IsArray())
							{
								ec << "value 'includes' is not array";
								return false;
							}
							includes.resize(v->Size());
							for (rapidjson::SizeType i = 0, l = v->Size(); i < l; ++i)
								if (!getInclude((*v)[i], includes[i], ec))
									return false;
						}

						h = std::make_shared<CPPBasicCodegenHelper>(includes);
					}
					else if (type_str == "custom")
					{
						auto _h = std::make_shared<CustomCPPHelper>();
						if (!_h->build(r, ec))
							return false;
						h = _h;
					}
					else
					{
						ec << "type is not supported: '" + type_str + "'";
						return false;
					}

					if (name.length())
						cpp_helpers[name] = h;
				}
				else
				{
					ec << "value of helper is not string and not object";
					return false;
				}

				return true;
			}

			bool build(const rapidjson::Value & r, std::shared_ptr<CSCodeGenHelper> & h, ErrorCollector & ec)
			{
				if (r.IsString())
				{
					std::string name;
					if (!getName(r, name, ec))
						return false;
					if (!cs_helpers.count(name))
					{
						ec << "c# codegen helper '" + name + "' is not defined";
						return false;
					}
					h = cs_helpers[name];
					return true;
				}
				else if (r.IsObject())
				{
					std::string name;
					if (!getStringOptional(r, "name", name, ec))
						return false;

					std::string type_str = "basic";
					if (!getStringOptional(r, "type", type_str, ec))
						return false;

					if (type_str == "basic")
						h = std::make_shared<CSBasicCodegenHelper>();
					else if (type_str == "custom")
					{
						auto _h = std::make_shared<CustomCSHelper>();
						if (!_h->build(r, ec))
							return false;
						h = _h;
					}
					else
					{
						ec << "type is not supported: '" + type_str + "'";
						return false;
					}

					if (name.length())
						cs_helpers[name] = h;
				}
				else
				{
					ec << "value of helper is not string and not object";
					return false;
				}

				return true;
			}

			bool build(const rapidjson::Value & r, std::shared_ptr<OperationGroup> & op, ErrorCollector & ec)
			{
				rapidjson::Value * v;
				if (!JSONTools::getValue(r, "ops", v) || v->IsNull())
					return true;

				if (!v->IsArray())
				{
					ec << "value 'ops' is not array";
					return false;
				}

				std::vector<std::shared_ptr<Operation>> ops(v->Size());
				for (rapidjson::SizeType i = 0, l = v->Size(); i < l; ++i)
					if (!build((*v)[i], ops[i], ec))
						return false;

				op = std::make_shared<OperationGroup>(ops);

				return true;
			}

			bool build(const rapidjson::Value & v, std::shared_ptr<Read> & op, std::shared_ptr<Reader> & reader, ErrorCollector & ec)
			{
				if (v.IsString())
				{
					if (!readers.count(v.GetString()))
					{
						ec << std::string() + "the required reader '" + v.GetString() + "' is not yet defined";
						return false;
					}
					reader = readers[v.GetString()];
					return true;
				}
				else if (v.IsObject())
				{
					std::string name;
					if (!getStringOptional(v, "name", name, ec))
						return false;

					std::string type_str;
					if (!JSONTools::getValue(v, "type", type_str))
					{
						ec << "value 'type' is not found or invalid";
						return false;
					}

					auto get_data = [&] (std::string & str)-> bool {
						rapidjson::Value * v_;
						if (JSONTools::getValue(v, "data", v_))
						{
							if (!JSONTools::getValue(*v_, str))
							{
								ec << "value 'data' is invalid";
								return false;
							}
						}
						else if (JSONTools::getValue(v, "filename", v_))
						{
							std::string filename;
							if (!JSONTools::getValue(*v_, filename))
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
					};

					if (type_str == "json")
					{
						std::string str;
						if (!get_data(str))
							return false;
						reader = std::make_shared<JSONReader>(str);
					}
					else if (type_str == "xml")
					{
						std::string str;
						if (!get_data(str))
							return false;
						reader = std::make_shared<XMLReader>(str);
					}
					else
					{
						ec << std::string() + "unsupported reader type '" + type_str + "'";
						return false;
					}

					if (name.length())
					{
						if (readers.count(name))
						{
							ec << std::string() << "reader '" + name + "' has been already defined";
							return false;
						}

						readers[name] = reader;
					}

					op = std::make_shared<Read>(reader);
				}
				else
				{
					ec << "value with nature 'read' is invalid";
					return false;
				}

				return true;
			}

			bool build(const rapidjson::Value & r, std::shared_ptr<Write> &op, ErrorCollector & ec)
			{
				if (!r.IsObject())
				{
					ec << "value with nature 'write' is invalid";
					return false;
				}

				rapidjson::Value * v;
				if (!JSONTools::getValue(r, "read", v))
				{
					ec << "value 'read' is not found";
					return false;
				}

				std::shared_ptr<Read> read_op;
				std::shared_ptr<Reader> reader;
				if (!build(*v, read_op, reader, ec))
					return false;

				std::string type_str;
				if (!JSONTools::getValue(r, "type", type_str))
				{
					ec << "value 'type' is not found or invalid";
					return false;
				}

				auto get_out = [&](std::shared_ptr<std::ostream> & o, std::string & filename) -> bool {
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
				};

				std::shared_ptr<Writer> writer;
				if (type_str == "c++")
				{
					std::string codegen_str;
					if (!JSONTools::getValue(r, "codegen", codegen_str))
					{
						ec << "value 'codegen' is not found or invalid";
						return false;
					}

					rapidjson::Value * v;
					std::shared_ptr<CPPCodeGenHelper> helper;
					if (JSONTools::getValue(r, "helper", v) && !build(*v, helper, ec))
						return false;

					std::shared_ptr<CPPCodeGen> codegen;
					if (codegen_str == "json_stl")
						codegen = helper ? std::make_shared<JSON_STL_CodeGen>(helper) : std::make_shared<JSON_STL_CodeGen>();
					else
					{
						ec << std::string() << "unsupported  C++ code generator '" + codegen_str + "'";
						return false;
					}

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
					if (!get_out(o, filename))
						return false;

					std::string name;
					if (filename.length())
					{
						name = filename;
						if (!getStringOptional(r, "name", name, ec))
							return false;
					}
					else if (!getName(r, name, ec))
							return false;

					auto writer = std::make_shared<CPPWriter>(mode, role, codegen, o, name);

					op = std::make_shared<Write>(reader, read_op, writer);
				}
				else if (type_str == "c#")
				{
					std::string codegen_str;
					if (!JSONTools::getValue(r, "codegen", codegen_str))
					{
						ec << "value 'codegen' is not found or invalid";
						return false;
					}

					rapidjson::Value * v;
					std::shared_ptr<CSCodeGenHelper> helper;
					if (JSONTools::getValue(r, "helper", v) && !build(*v, helper, ec))
						return false;

					std::shared_ptr<CSCodeGen> codegen;
					if (codegen_str == "json")
						codegen = helper ? std::make_shared<JSON_CSCodeGen>(helper) : std::make_shared<JSON_CSCodeGen>();
					else
					{
						ec << std::string() << "unsupported  C# code generator '" + codegen_str + "'";
						return false;
					}

					std::string role_str;
					if (!JSONTools::getValue(r, "role", role_str))
					{
						ec << "value 'mode' is not found or invalid";
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
					if (!get_out(o, filename))
						return false;

					auto writer = std::make_shared<CSWriter>(role, codegen, o);

					op = std::make_shared<Write>(reader, read_op, writer);
				}
				else if (type_str == "json")
				{
					std::shared_ptr<std::ostream> o;
					std::string filename;
					if (!get_out(o, filename))
						return false;

					auto writer = std::make_shared<JSONWriter>(o);

					op = std::make_shared<Write>(reader, read_op, writer);
				}
				else
				{
					ec << std::string() << "unsupported writer type '" + type_str + "'";
					return false;
				}

				return true;
			}

			bool build(const rapidjson::Value & r, std::shared_ptr<Operation> &op, ErrorCollector & ec)
			{
				if (!r.IsObject())
				{
					ec << "toplevel value is invalid";
					return false;
				}

				std::string nat = "group";
				if (!getNatureOptional(r, nat, ec))
					return false;

				if (nat == "group")
				{
					std::shared_ptr<OperationGroup> _op;
					if (!build(r, _op, ec))
						return false;
					op = _op;
				}
				else if (nat == "read")
				{
					std::shared_ptr<Read> _op;
					std::shared_ptr<Reader> reader;
					if (!build(r, _op, reader, ec))
						return false;
					op = _op;
				}
				else if (nat == "write")
				{
					std::shared_ptr<Write> _op;
					if (!build(r, _op, ec))
						return false;
					op = _op;
				}
				else if (nat == "cpp_helper")
				{
					std::shared_ptr<CPPCodeGenHelper> h;
					if (!build(r, h, ec))
						return false;
					op = nullptr;
				}
				else
				{
					ec << "unknown nature '" + nat + "'";
					return false;
				}

				return true;
			}
		};

	};

	Job::Job(const std::shared_ptr<Operation> & op) : priv(new Priv(op))
	{ }

	Job::~Job()
	{
		delete priv;
	}

	bool Job::run(ErrorCollector & ec)
	{
		if (!priv->op)
		{
			ec << "no operation is specified";
			return false;
		}
		return priv->op->run(ec);
	}

	//static
	bool Job::build(const std::string & json_data, std::shared_ptr<Job> & ret, ErrorCollector & ec)
	{
		std::vector<char> buffer(json_data.length() + 1);
		memcpy(buffer.data(), json_data.c_str(), json_data.length());

		rapidjson::Document doc;

		if (doc.ParseInsitu(buffer.data()).HasParseError())
		{
			ec << ("JSON parse error (" + std::to_string(doc.GetParseError()) + ")");
			return false;
		}

		return build(doc, ret, ec);
	}

	//static
	bool Job::build(const rapidjson::Value & root, std::shared_ptr<Job> & ret, ErrorCollector & ec)
	{
		std::shared_ptr<Operation> op;
		Priv::Context ctx;
		if (!ctx.build(root, op, ec))
			return false;
		ret = std::make_shared<Job>(op);
	}

}
