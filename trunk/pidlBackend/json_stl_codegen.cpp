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

	JSON_STL_CodeGen::JSON_STL_CodeGen() :
		CPPCodeGen(),
		priv(new Priv(std::make_shared<CPPBasicCodegenHelper>()))
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
		switch (ctx->mode())
		{
		case Mode::AllInOne:
		case Mode::Implementatinon:
			if (!writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "map"), ec) ||
				!writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "functional"), ec))
				return false;
			break;
		case Mode::Declaration:
			break;
		}
		return
			writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "vector"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "string"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "time.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/deatetime.h" : "datetime.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/exception.h" : "exception.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/nullable.h" : "nullable.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/jsontools.h" : "jsontools.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/errorcollector.h" : "errorcollector.h"), ec);
	}

	bool JSON_STL_CodeGen::writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using nullable = PIDL::Nullable<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using nullable_const_ref = PIDL::NullableConstRef<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using array = std::vector<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using string = std::string;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using datetime = PIDL::DateTime;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using blob = std::vector<char>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using exception = PIDL::Exception;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using error_collector = PIDL::ErrorCollector;" << std::endl;

		return true;
	}

	bool JSON_STL_CodeGen::writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		priv->writeTabs(code_deepness, ctx) << "//private members" << std::endl << std::endl;

		switch (ctx->role())
		{
		case Role::Server:
			priv->writeTabs(code_deepness, ctx) << "typedef std::function<_InvokeStatus(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)> __Function;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "std::map<std::string, __Function> __functions;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_InvokeStatus __callFunction(const std::string & name, const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!__functions.count(name))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ec << \"function '\" + name + \"'is not found\";" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus::NotImplemented;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return __functions[name](root, ret, ec);" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			break;
		case Role::Client:
			priv->writeTabs(code_deepness, ctx) << "bool __invokeCall(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "auto status = that->_invoke(root, ret, ec);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "switch(status)" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus::Ok: break;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus::NotImplemented:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "ec.add((long)status, \"function is not implemented\"); return false;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus::Error:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "ec.add((long)status, \"error while executing server function\"); return false;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus::FatalError:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "ec.add((long)status, \"fatal error while executing server function\"); return false;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus::MarshallingError:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "ec.add((long)status, \"error while marshalling of function call\"); return false;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "}" << std::endl << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			break;
		}

		//__getValue
		priv->writeTabs(code_deepness, ctx) << "//marshallers" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "static bool __getValue(const rapidjson::Value & v, const char * name, rapidjson::Type type, rapidjson::Value *& ret, error_collector & ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(v, name, ret))" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (v.GetType() != type)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value '\" + name + \"' is invalid\"; return false; }" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static bool __getValue(const rapidjson::Value & v, T & ret, error_collector & ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(v, ret))" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ ec << \"value is invalid\"; return false; }" << std::endl << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static bool __getValue(const rapidjson::Value & r, const char * name, T & ret, error_collector & ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(r, name, v) || v->IsNull())" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value '\" + name + \"' is not found or null\"; return false; }" << std::endl << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return __getValue(*v, ret, ec);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static bool __getValue(const rapidjson::Value & r, const char * name, nullable<T> & ret, error_collector & ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (v->IsNull())" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ ret.setNull(); return true; }" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return __getValue(*v, ret.setNotNull(), ec);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			if (td)
			{
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << "static bool __getValue(const rapidjson::Value & v, " << td->name() << " & ret, error_collector & ec)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!v.IsObject())" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value of '" << td->name() << "' is not object\"; return false; }" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return" << std::endl;
					bool is_first = true;
					for (auto & m : s->members())
					{
						if (is_first)
						{
							is_first = false;
							priv->writeTabs(code_deepness + 1, ctx) << "  ";
						}
						else
							priv->writeTabs(code_deepness + 1, ctx) << "& ";
						*ctx << "__getValue(v, \"" << m->name() << "\", ret." << m->name() << ", ec)" << std::endl;
					}
					priv->writeTabs(code_deepness, ctx) << ";" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				}
			}
		}


		//__createValue
		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			if (td)
			{
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << "static rapidjson::Value __createValue(rapidjson::Document & doc, const " << td->name() << " & in)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "rapidjson::Value v(rapidjson::kObjectType);" << std::endl;

					for (auto & m : s->members())
						priv->writeTabs(code_deepness, ctx) << "__addValue(doc, v, \"" << m->name() << "\", in." << m->name() << ");" << std::endl;

					priv->writeTabs(code_deepness, ctx) << "return v;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				}
			}
		}

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static rapidjson::Value __createValue(rapidjson::Document & doc, const T & t)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ return PIDL::JSONTools::createValue(doc, t); }" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static rapidjson::Value __createValue(rapidjson::Document & doc, const std::vector<T> & values)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "rapidjson::Value v(rapidjson::kArrayType);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "for (auto & _v : values)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ auto tmp = __createValue(doc, _v); v.PushBack(tmp, doc.GetAllocator()); }" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return v;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static rapidjson::Value __createValue(rapidjson::Document & doc, const std::vector<char> & blob)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ return PIDL::JSONTools::createValue(doc, blob); }" << std::endl << std::endl;

		//__addValue
		priv->writeTabs(code_deepness, ctx) << "template<typename T> static void __addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const T & v)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ auto tmp = __createValue(doc, v); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static void __addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<T> & values)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ auto tmp = __createValue<T>(doc, values); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static void __addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<char> & blob)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ PIDL::JSONTools::addValue(doc, r, name, blob); }" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static void __addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable_const_ref<T> & v)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (v.isNull()) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "else __addValue(doc, r, name, *v);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "template<typename T> static void __addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable<T> & v)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (v.isNull()) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "else __addValue(doc, r, name, *v);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		return true;
	}

	bool JSON_STL_CodeGen::writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		if (ctx->role() == Role::Client && ctx->mode() == Mode::Implementatinon)
			return true;

		switch (ctx->mode())
		{
		case Mode::AllInOne:
		case Mode::Declaration:
			break;
		case Mode::Implementatinon:
			break;
		}

		switch (ctx->mode())
		{
		case Mode::AllInOne:
		case Mode::Declaration:
			priv->writeTabs(code_deepness, ctx) << "enum class _InvokeStatus {Ok, NotImplemented, Error, MarshallingError, FatalError};" << std::endl;
		case Mode::Implementatinon:
			break;
		}
		
		switch (ctx->role())
		{
		case Role::Server:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				priv->writeTabs(code_deepness, ctx) << "_InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
				break;
			case Mode::Implementatinon:
				priv->writeTabs(code_deepness, ctx) << intf->name() << "::_InvokeStatus " << intf->name() << "::_invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
				break;
			}

			switch (ctx->mode())
			{
			case Mode::Declaration:
				*ctx << ";" << std::endl;
				break;
			case Mode::AllInOne:
			case Mode::Implementatinon:
				ctx->stream() << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				switch (ctx->mode())
				{
				case Mode::Declaration:
					break;
				case Mode::AllInOne:
					priv->writeTabs(code_deepness, ctx) << "auto * p = this;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					priv->writeTabs(code_deepness, ctx) << "auto * p = priv;" << std::endl << std::endl;
					break;
				}

				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!p->__getValue(root, \"function\", rapidjson::kObjectType, v, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "std::string name;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!p->__getValue(*v, \"name\", name, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return p->__callFunction(name, *v, ret, ec);" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			}
			break;
		case Role::Client:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				priv->writeTabs(--code_deepness, ctx) << "virtual _InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec) = 0;" << std::endl;
				break;
			case Mode::Implementatinon:
				break;
			}
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeFunctionBody(Language::Function * function, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{

		switch (ctx->mode())
		{
		case Mode::AllInOne:
			priv->writeTabs(code_deepness, ctx) << "auto p = this;" << std::endl << std::endl;
			break;
		case Mode::Implementatinon:
			priv->writeTabs(code_deepness, ctx) << "auto p = priv;" << std::endl << std::endl;
			break;
		case Mode::Declaration:
			break;
		}

		switch (ctx->role())
		{
		case Role::Client:
			{
				priv->writeTabs(code_deepness, ctx) << "PIDL::ExceptionErrorCollector<error_collector> ec;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document __doc;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "__doc.SetObject();" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value __v(rapidjson::kObjectType);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "p->__addValue(__doc, __v, \"name\", \"" << function->name() << "\");" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value __aa(rapidjson::kObjectType);" << std::endl;

				for (auto & a : function->in_arguments())
					priv->writeTabs(code_deepness, ctx) << "p->__addValue(__doc, __aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;

				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(__doc, __v, \"arguments\", __aa);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(__doc, __doc, \"function\", __v);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document __ret;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!p->__invokeCall(__doc, __ret, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "ec.throwException();" << std::endl;

				auto ret_type = function->returnType().get();
				if (!dynamic_cast<Language::Void*>(ret_type))
				{
					priv->writeTabs(code_deepness, ctx);
					if (!writeType(ret_type, code_deepness, ctx, ec))
						return false;
					*ctx <<  " __retval;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!p->__getValue(__ret, \"retval\", __retval, ec))" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "ec.throwException();" << std::endl;
				}

				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * __out_v;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!p->__getValue(__ret, \"output\", rapidjson::kObjectType, __out_v, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "ec.throwException();" << std::endl;

				const auto & out_args = function->out_arguments();
				if (out_args.size())
				{
					auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;

					bool is_first = true;
					for (auto & a : out_args)
					{
						priv->writeTabs(code_deepness, ctx);
						if (!is_first)
							o << "| ";
						is_first = false;
						o << "!p->__getValue(*__out_v, \"" << a->name() << "\", " << a->name() << ", ec)" << std::endl;
					}
					priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "ec.throwException();" << std::endl;
				}

				if (!dynamic_cast<Language::Void*>(ret_type))
					priv->writeTabs(code_deepness, ctx) << "return __retval;" << std::endl;
			}
			break;
		case Role::Server:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeConstructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Client:
			break;
		case Role::Server:
			for (auto & d : intf->definitions())
			{
				auto function = dynamic_cast<Language::Function*>(d.get());
				if (function)
				{
					priv->writeTabs(code_deepness++, ctx) << "__functions[\"" << function->name() << "\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;

					for (auto & a : function->arguments())
					{
						auto & o = priv->writeTabs(code_deepness, ctx);
						if (!writeType(a->type().get(), code_deepness, ctx, ec))
							return false;
						o << " __arg__" << a->name() << ";" << std::endl;
					}

					auto in_args = function->in_arguments();
					if (in_args.size())
					{
						priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * aa;" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "if (!__getValue(r, \"arguments\", rapidjson::kObjectType, aa, ec))" << std::endl;
						priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;

						auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;
						bool is_first = true;
						for (auto & a : in_args)
						{
							priv->writeTabs(code_deepness, ctx);
							if (!is_first)
								o << "| ";
							is_first = false;
							o << "!__getValue(*aa, \"" << a->name() << "\", __arg__" << a->name() << ", ec)" << std::endl;
						}
						priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;
						priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
					}

					auto ret_type = function->returnType().get();
					if (dynamic_cast<Language::Void*>(function->returnType().get()))
						ret_type = nullptr;
					if (ret_type)
					{
						priv->writeTabs(code_deepness, ctx);
						if (!writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " retval;" << std::endl;
					}
					priv->writeTabs(code_deepness, ctx) << "try" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					auto & o = priv->writeTabs(code_deepness, ctx);
					if (ret_type)
						o << "retval = ";
					o << "that->" << function->name() << "(";
					bool is_first = true;
					for (auto & a : function->arguments())
					{
						if (!is_first)
							o << ", ";
						is_first = false;
						o << "__arg__" << a->name();
					}
					o << ");" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "catch (exception * e)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "e->get(ec); " << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus::Error;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "catch (std::exception * e)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.add((long)_InvokeStatus::FatalError, std::string() + \"unhandled exception: '\" + e->what() + \"'\");" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus::FatalError;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "catch (...)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.add((long)_InvokeStatus::FatalError, \"unknown unhandled exception\");" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus::FatalError;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

					priv->writeTabs(code_deepness, ctx) << "ret.SetObject();" << std::endl;

					if (ret_type)
						priv->writeTabs(code_deepness, ctx) << "__addValue(ret, ret, \"retval\", retval);" << std::endl;

					const auto & out_args = function->out_arguments();
					if (out_args.size())
					{
						priv->writeTabs(code_deepness, ctx) << "rapidjson::Value out_v(rapidjson::kObjectType);" << std::endl;
						for (auto & a : out_args)
							priv->writeTabs(code_deepness, ctx) << "__addValue(ret, out_v, \"" << a->name() << "\", __arg__" << a->name() << ");" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(ret, ret, \"output\", out_v);" << std::endl;
					}

					priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus::Ok;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
				}
			}
			break;
		}
		return true;
	}
	
}
