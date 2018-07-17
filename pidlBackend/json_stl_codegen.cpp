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

#include "include/pidlBackend/json_stl_codegen.h"
#include "include/pidlBackend/language.h"

#include <assert.h>

namespace PIDL
{

	struct JSON_STL_CodeGen::Priv 
	{
		Priv(const std::shared_ptr<CPPCodeGenHelper> & helper_) : helper(helper_)
		{ }

		std::shared_ptr<CPPCodeGenHelper> helper;

		std::string privLogger()
		{
			return "priv->logger";
		}

		std::ostream & writeTabs(short code_deepness, CPPCodeGenContext * ctx) const
		{
			char ch;
			for (short i = 0, l = code_deepness * helper->tabDefinition(ch); i < l; ++i)
				*ctx << ch;
			return ctx->stream();
		}

	};

	JSON_STL_CodeGen::JSON_STL_CodeGen(const std::shared_ptr<CPPCodeGenHelper> & helper) :
		CPPCodeGen(),
		priv(new Priv(helper))
	{ }

	JSON_STL_CodeGen::~JSON_STL_CodeGen()
	{
		delete priv;
	}

	CPPCodeGenHelper * JSON_STL_CodeGen::helper() const
	{
		return priv->helper.get();
	}

	bool JSON_STL_CodeGen::writeIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		auto core_path = helper()->coreIncludePath();
		switch (ctx->nature())
		{
		case CPPCodeGenContext::Nature::AllInOne:
		case CPPCodeGenContext::Nature::Implementatinon:
			if (!writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "map"), ec))
				return false;
			break;
		case CPPCodeGenContext::Nature::Declaration:
			break;
		}
		return
			writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "vector"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "string"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "time.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/exception.h" : "exception.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/nullable.h" : "nullable.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/jsontools.h" : "jsontools.h"), ec);
	}

	bool JSON_STL_CodeGen::writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using nullable = PIDL::Nullable<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using nullable_const_ref = PIDL::NullableConstRef<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using array = std::vector<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using string = std::string;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using datetime = tm;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using blob = std::vector<char>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using exception = PIDL::Exception;" << std::endl;

		return true;
	}

	bool JSON_STL_CodeGen::writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		priv->writeTabs(code_deepness, ctx) << "//private members" << std::endl << std::endl;

		switch (ctx->role())
		{
		case CPPCodeGenContext::Role::Server:
			priv->writeTabs(code_deepness, ctx) << "std::map<std::string, std::function<bool(rapidjson::Value & r, rapidjson::Document & ret, " << helper()->errorCollector() << " & ec)>> _functions;" << std::endl;
			break;
		case CPPCodeGenContext::Role::Client:
			break;
		}

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static bool getValue(rapidjson::Value & v, T & ret)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ return PIDL::JSONTools::getValue(v, ret); }" << std::endl << std::endl;
		
		priv->writeTabs(code_deepness, ctx) << "template<typename T> static bool getValue(rapidjson::Value & r, const char * name, T & ret)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(r, name, v) || v->IsNull())" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return getValue(*v, ret);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static bool getValue(rapidjson::Value & r, const char * name, nullable<T> & ret)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (v->IsNull())" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret.setNull();" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return getValue(*v, ret.setNotNull());" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			if (td)
			{
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << "static bool getValue(rapidjson::Value & v, " << td->name() << " & ret)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return !v.IsNull() && v.IsObject()" << std::endl;
					++code_deepness;
					for (auto & m : s->members())
						priv->writeTabs(code_deepness, ctx) << "&& getValue(v, \"" << m->name() << "\", ret." << m->name() << ")" << std::endl;
					--code_deepness;
					priv->writeTabs(code_deepness, ctx) << ";" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				}
			}
		}


		priv->writeTabs(code_deepness, ctx) << "template<typename T> static void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const T & v)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ PIDL::JSONTools::addValue(doc, r, name, v); }" << std::endl << std::endl;
		
		priv->writeTabs(code_deepness, ctx) << "template<typename T> static void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable_const_ref<T> & v)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (v.isNull()) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "else addValue(doc, r, name, *v);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
		
		priv->writeTabs(code_deepness, ctx) << "template<typename T> static void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable<T> & v)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (v.isNull()) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "else addValue(doc, r, name, *v);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			if (td)
			{
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << "static void addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const " << td->name() << " & in)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "rapidjson::Value v(rapidjson::kObjectType);" << std::endl;

					for (auto & m : s->members())
						priv->writeTabs(code_deepness, ctx) << "addValue(doc, v, \"" << m->name() << "\", in." << m->name() << ");" << std::endl;

					priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(doc, r, name, v);" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				}
			}
		}


		switch (ctx->role())
		{
		case CPPCodeGenContext::Role::Client:
			priv->writeTabs(code_deepness, ctx) << "static void addName(rapidjson::Document & doc, rapidjson::Value & r, const char * str)" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ PIDL::JSONTools::addValue(doc, r, \"name\", str); }" << std::endl;
			break;
		case CPPCodeGenContext::Role::Server:
			priv->writeTabs(code_deepness, ctx) << "static bool getName(rapidjson::Value & r, std::string & ret)" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ return PIDL::JSONTools::getValue(r, \"name\", ret); }" << std::endl << std::endl;
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		if (ctx->role() == CPPCodeGenContext::Role::Client && ctx->nature() == CPPCodeGenContext::Nature::Implementatinon)
			return true;

		switch (ctx->nature())
		{
		case CPPCodeGenContext::Nature::AllInOne:
		case CPPCodeGenContext::Nature::Declaration:
			break;
		case CPPCodeGenContext::Nature::Implementatinon:
			break;
		}

		auto & o = priv->writeTabs(code_deepness, ctx);

		switch (ctx->role())
		{
		case CPPCodeGenContext::Role::Server:
			switch (ctx->nature())
			{
			case CPPCodeGenContext::Nature::AllInOne:
			case CPPCodeGenContext::Nature::Declaration:
				o << "bool _invoke(const rapidjson::Value & root, rapidjson::Document & ret, " << helper()->errorCollector() << " & ec)";
				break;
			case CPPCodeGenContext::Nature::Implementatinon:
				o << "bool " << intf->name() << "::_invoke(const rapidjson::Value & root, rapidjson::Document & ret, " << helper()->errorCollector() << " & ec)";
				break;
			}

			switch (ctx->nature())
			{
			case CPPCodeGenContext::Nature::Declaration:
				o << ";" << std::endl;
				break;
			case CPPCodeGenContext::Nature::AllInOne:
			case CPPCodeGenContext::Nature::Implementatinon:
				o << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "//server implementation" << std::endl << std::endl;
				switch (ctx->nature())
				{
				case CPPCodeGenContext::Nature::Declaration:
					break;
				case CPPCodeGenContext::Nature::AllInOne:
					priv->writeTabs(code_deepness, ctx) << "auto * p = this;" << std::endl << std::endl;
					break;
				case CPPCodeGenContext::Nature::Implementatinon:
					priv->writeTabs(code_deepness, ctx) << "auto * p = priv;" << std::endl << std::endl;
					break;
				}

				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(root, \"function\", v) || !v->IsObject())" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "auto err = ec.add(-1, \"'function' is not found or invalid\");" << std::endl;
				//priv->writeTabs(code_deepness, ctx) << "SAS_LOG_ERROR(p->logger, err);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "std::string name;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!Priv::getName(*v, name))" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "auto err = ec.add(-1, \"name of function is not found or invalid\");" << std::endl;
				//priv->writeTabs(code_deepness, ctx) << "SAS_LOG_ERROR(p->logger, err);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!p->_functions.count(name))" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "auto err = ec.add(-1, \"function '\" + name + \"' is not found\");" << std::endl;
				//priv->writeTabs(code_deepness, ctx) << "SAS_LOG_ERROR(p->logger, err);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return p->_functions[name](*v, ret, ec);" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			}

			break;

		case CPPCodeGenContext::Role::Client:
			switch (ctx->nature())
			{
			case CPPCodeGenContext::Nature::AllInOne:
			case CPPCodeGenContext::Nature::Declaration:
				o << "virtual bool _invoke(const rapidjson::Value & root, rapidjson::Document & ret, " << helper()->errorCollector() << " & ec) = 0;" << std::endl;
				break;
			case CPPCodeGenContext::Nature::Implementatinon:
				break;
			}
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeFunctionBody(Language::Function * function, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{

		//switch (ctx->nature())
		//{
		//case CPPCodeGenContext::Nature::AllInOne:
		//	priv->writeTabs(code_deepness, ctx) << "auto p = this;" << std::endl << std::endl;
		//	break;
		//case CPPCodeGenContext::Nature::Implementatinon:
		//	priv->writeTabs(code_deepness, ctx) << "auto p = priv;" << std::endl << std::endl;
		//	break;
		//case CPPCodeGenContext::Nature::Declaration:
		//	break;
		//}

		switch (ctx->role())
		{
		case CPPCodeGenContext::Role::Client:
			{
				priv->writeTabs(code_deepness, ctx) << "//client implementation" << std::endl << std::endl;

				priv->writeTabs(code_deepness, ctx) << "PIDL::ExceptionErrorCollector<" << helper()->errorCollector() << "> ec;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document doc;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "doc.SetObject();" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value v(rapidjson::kObjectType);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "Priv::addName(doc, v, \"" << function->name() << "\");" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value aa(rapidjson::kObjectType);" << std::endl;

				auto & args = function->arguments();

				for (auto & a : args)
					priv->writeTabs(code_deepness, ctx) << "Priv::addValue(doc, aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;

				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(doc, v, \"arguments\", aa);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(doc, doc, \"function\", v);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document ret;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!_invoke(doc, ret, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "ec.throwException();" << std::endl;

				auto ret_type = function->returnType().get();
				if (!dynamic_cast<Language::Void*>(ret_type))
				{
					priv->writeTabs(code_deepness, ctx) << "bool __retval;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!Priv::getValue(ret, \"retval\", __retval))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "auto err = ec.add(-1, \"'retval' is not found or invalid\");" << std::endl;
					//priv->writeTabs(code_deepness, ctx) << "SAS_LOG_ERROR(p->logger, err);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.throwException();" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				}

				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * out_v;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(ret, \"output\", out_v) || !out_v->IsObject())" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "auto err = ec.add(-1, \"'output' is not found or invalid\");" << std::endl;
				//priv->writeTabs(code_deepness, ctx) << "SAS_LOG_ERROR(p->logger, err);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "ec.throwException();" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

				if (args.size())
				{
					auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;

					bool is_first = true;
					for (auto & a : args)
					{
						switch (a->direction())
						{
						case Language::Function::Argument::Direction::In:
							break;
						case Language::Function::Argument::Direction::Out:
						case Language::Function::Argument::Direction::InOut:
							priv->writeTabs(code_deepness, ctx);
							if (!is_first)
								o << "|| ";
							is_first = false;
							o << "!Priv::getValue(*out_v, \"" << a->name() << "\", " << a->name() << ")" << std::endl;
							break;
						}
					}
					priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "auto err = ec.add(-1, \"could not get one or more arguments\");" << std::endl;
					//priv->writeTabs(code_deepness, ctx) << "SAS_LOG_ERROR(p->logger, err); " << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.throwException();" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				}
				if (!dynamic_cast<Language::Void*>(ret_type))
					priv->writeTabs(code_deepness, ctx) << "return __retval;" << std::endl;
			}
			break;
		case CPPCodeGenContext::Role::Server:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeConstructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case CPPCodeGenContext::Role::Client:
			break;
		case CPPCodeGenContext::Role::Server:
			for (auto & d : intf->definitions())
			{
				auto function = dynamic_cast<Language::Function*>(d.get());
				if (function)
				{
					priv->writeTabs(code_deepness++, ctx) << "_functions[\"" << function->name() << "\"] = [&](rapidjson::Value & r, rapidjson::Document & ret, " << helper()->errorCollector() << " & ec)->bool {" << std::endl;

					auto args = function->arguments();
					if (args.size())
					{
						priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * aa;" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(r, \"arguments\", aa))" << std::endl;
						priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "auto err = ec.add(-1, \"arguments are not found\");" << std::endl;
						//priv->writeTabs(code_deepness, ctx) << "SAS_LOG_ERROR(logger, err);"
						priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
						priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

						for (auto & a : args)
						{
							auto & o = priv->writeTabs(code_deepness, ctx);
							if (!writeType(a->type().get(), code_deepness, ctx, ec))
								return false;
							o << " arg__" << a->name() << ";" << std::endl;
						}

						auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;
						bool is_first = true;
						for (auto & a : args)
						{
							switch (a->direction())
							{
							case Language::Function::Argument::Direction::In:
							case Language::Function::Argument::Direction::InOut:
								priv->writeTabs(code_deepness, ctx);
								if (!is_first)
									o << "|| ";
								is_first = false;
								o << "!getValue(*aa, \"" << a->name() << "\", arg__" << a->name() << ")" << std::endl;
								break;
							case Language::Function::Argument::Direction::Out:
								break;
							}
						}
						priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;
						priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "auto err = ec.add(-1, \"one or more arguments are not found\");" << std::endl;
						//priv->writeTabs(code_deepness, ctx) << "SAS_LOG_ERROR(logger, err);" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
						priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

					}

					auto ret_type = function->returnType().get();
					if (dynamic_cast<Language::Void*>(function->returnType().get()))
						ret_type = nullptr;
					if (ret_type)
					{
						priv->writeTabs(code_deepness, ctx);
						if (!writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " __retval;" << std::endl;
					}
					priv->writeTabs(code_deepness, ctx) << "try" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					auto & o = priv->writeTabs(code_deepness, ctx);
					if (ret_type)
						o << "__retval = ";
					o << "that->" << function->name() << "(";
					bool is_first = true;
					for (auto & a : args)
					{
						if (!is_first)
							o << ", ";
						is_first = false;
						o << "arg__" << a->name();
					}
					o << ");" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "catch (exception * e)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "e->get(ec); " << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

					priv->writeTabs(code_deepness, ctx) << "ret.SetObject();" << std::endl;

					if (ret_type)
						priv->writeTabs(code_deepness, ctx) << "addValue(ret, ret, \"retval\", __retval);" << std::endl;

					priv->writeTabs(code_deepness, ctx) << "rapidjson::Value out_v(rapidjson::kObjectType);" << std::endl;

					for (auto & a : args)
					{
						switch (a->direction())
						{
						case Language::Function::Argument::Direction::In:
							break;
						case Language::Function::Argument::Direction::InOut:
						case Language::Function::Argument::Direction::Out:
							priv->writeTabs(code_deepness, ctx) << "addValue(ret, out_v, \"" << a->name() << "\", arg__" << a->name() << ");" << std::endl;
							break;
						}
					}

					priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(ret, ret, \"output\", out_v);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
				}
			}
			break;
		}
		return true;
	}



	//bool JSON_STL_CodeGen_server::generate(const std::shared_ptr<Language::TopLevel> & top_level, short code_deepness, std::ostream & o, ErrorCollector & ec)
	//{
	//	return priv->addTopLevel(code_deepness, ctx, top_level.get(), ec);

	//	//priv->helper->writeTabs(code_deepness, ctx) << priv->helper->logging()->loggingStart(priv->privLogger()) << std::endl;
	//	//priv->helper->writeTabs(code_deepness, ctx) << priv->helper->logging()->loggingAssert(priv->privLogger(), "arguments.IsObject()", "\"'arguments' must be object\"") << std::endl;

	//	//TODO

	//	//priv->helper->writeTabs(code_deepness, ctx) << "return true" << std::endl;

	//	//return true;
	//}
	
}
