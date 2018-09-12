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
		Priv(JSON_STL_CodeGen * that_, const std::shared_ptr<CPPCodeGenHelper> & helper_) : that(that_), helper(helper_)
		{ }

		JSON_STL_CodeGen * that;
		std::shared_ptr<CPPCodeGenHelper> helper;

		std::string privLogger()
		{
			return "priv->logger";
		}

		template<class T>
		std::string getScope(const T * t)
		{
			std::string ret;
			for (auto & sc : t->scope())
				ret += sc + "::";
			return ret;
		}

		template<class Class_T>
		bool writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			ctx->writeTabs(code_deepness) << "//private members" << std::endl << std::endl;

			switch (ctx->role())
			{
			case Role::Server:
				//ctx->writeTabs(code_deepness) << "std::map<std::string, _Object::Ptr> _objects;" << std::endl;
				ctx->writeTabs(code_deepness) << "typedef std::function<_InvokeStatus(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)> _Function;" << std::endl;
				ctx->writeTabs(code_deepness) << "std::map<std::string, _Function> _functions;" << std::endl;
				ctx->writeTabs(code_deepness) << "_InvokeStatus _callFunction(const std::string & name, const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_functions.count(name))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ec << \"function '\" + name + \"'is not found\";" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus::NotImplemented;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "return _functions[name](root, ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "_InvokeStatus _callFunction(std::function<void(void)> func, error_collector & ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "try" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "func();" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "catch (exception * e)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "e->get(ec); " << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus::Error;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "catch (std::exception * e)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.add((long)_InvokeStatus::FatalError, std::string() + \"unhandled exception: '\" + e->what() + \"'\");" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus::FatalError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "catch (...)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.add((long)_InvokeStatus::FatalError, \"unknown unhandled exception\");" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus::FatalError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus::Ok;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				break;
			case Role::Client:
				ctx->writeTabs(code_deepness) << "bool _invokeCall(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (dynamic_cast<Language::Object*>(cl))
					ctx->writeTabs(code_deepness) << "auto status = _intf->_invoke(root, ret, ec);" << std::endl;
				else
					ctx->writeTabs(code_deepness) << "auto status = _that->_invoke(root, ret, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "switch(status)" << std::endl;
				ctx->writeTabs(code_deepness) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus::Ok: break;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus::NotImplemented:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"function is not implemented\"); return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus::Error:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"error while executing server function\"); return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus::FatalError:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"fatal error while executing server function\"); return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus::MarshallingError:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"error while marshalling of function call\"); return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "}" << std::endl << std::endl;
				ctx->writeTabs(code_deepness) << "return true;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			}

			//_getValue
			ctx->writeTabs(code_deepness) << "//marshallers" << std::endl;
			ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, const char * name, rapidjson::Type type, rapidjson::Value *& ret, error_collector & ec)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(v, name, ret))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl;
			ctx->writeTabs(code_deepness) << "if (v.GetType() != type)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is invalid\"; return false; }" << std::endl;
			ctx->writeTabs(code_deepness) << "return true;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "template<typename T> bool _getValue(const rapidjson::Value & v, T & ret, error_collector & ec)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(v, ret))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ec << \"value is invalid\"; return false; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "return true;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "template<typename T> bool _getValue(const rapidjson::Value & r, const char * name, T & ret, error_collector & ec)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(r, name, v) || v->IsNull())" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found or null\"; return false; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "return _getValue(*v, ret, ec);" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "template<typename T> bool _getValue(const rapidjson::Value & r, const char * name, nullable<T> & ret, error_collector & ec)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "if (v->IsNull())" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret.setNull(); return true; }" << std::endl;
			ctx->writeTabs(code_deepness) << "return _getValue(*v, ret.setNotNull(), ec);" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			for (auto & d : cl->definitions())
			{

				if (dynamic_cast<Language::TypeDefinition*>(d.get()))
				{
					auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
					auto s = dynamic_cast<Language::Structure*>(td->type().get());
					if (s)
					{
						ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, " << td->name() << " & ret, error_collector & ec)" << std::endl;
						ctx->writeTabs(code_deepness++) << "{" << std::endl;
						ctx->writeTabs(code_deepness) << "if (!v.IsObject())" << std::endl;
						ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value of '" << td->name() << "' is not object\"; return false; }" << std::endl;
						ctx->writeTabs(code_deepness) << "return" << std::endl;
						bool is_first = true;
						for (auto & m : s->members())
						{
							if (is_first)
							{
								is_first = false;
								ctx->writeTabs(code_deepness + 1) << "  ";
							}
							else
								ctx->writeTabs(code_deepness + 1) << "& ";
							*ctx << "_getValue(v, \"" << m->name() << "\", ret." << m->name() << ", ec)" << std::endl;
						}
						ctx->writeTabs(code_deepness) << ";" << std::endl;
						ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					}
				}
				else if (dynamic_cast<Language::Object*>(d.get()))
				{
					auto obj = dynamic_cast<Language::Object*>(d.get());
					switch (ctx->role())
					{
					case Role::Client:
						ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, " << obj->name() << "::Ptr & ret, error_collector & ec)" << std::endl;
						ctx->writeTabs(code_deepness++) << "{" << std::endl;
						ctx->writeTabs(code_deepness) << "std::string object_id;" << std::endl;
						ctx->writeTabs(code_deepness) << "if (!_getValue(v, object_id, ec))" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "return false;" << std::endl;
						ctx->writeTabs(code_deepness) << "ret = std::make_shared<" << obj->name() << ">(_that, object_id);" << std::endl;
						ctx->writeTabs(code_deepness) << "return true;" << std::endl;
						ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
						break;
					case Role::Server:
						ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, " << obj->name() << "::Ptr & ret, error_collector & ec)" << std::endl;
						ctx->writeTabs(code_deepness++) << "{" << std::endl;
						ctx->writeTabs(code_deepness) << "std::string id;" << std::endl;
						ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(v, id))" << std::endl;
						ctx->writeTabs(code_deepness) << "{ ec << \"value is invalid\"; return false; }" << std::endl;
						ctx->writeTabs(code_deepness) << "return (bool)(ret = _that->_get_object<" << obj->name() << ">(id, ec));" << std::endl;
						ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

						ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & r, const char * name, " << obj->name() << "::Ptr & ret, error_collector & ec)" << std::endl;
						ctx->writeTabs(code_deepness++) << "{" << std::endl;
						ctx->writeTabs(code_deepness) << "rapidjson::Value * v; " << std::endl;
						ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
						ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found or null\"; return false; }" << std::endl;
						ctx->writeTabs(code_deepness) << "if (v->IsNull())" << std::endl;
						ctx->writeTabs(code_deepness) << "{ ret = nullptr; return true; }" << std::endl;
						ctx->writeTabs(code_deepness) << "return _getValue(*v, ret, ec);" << std::endl;
						ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

						break;
					}
				}
			}

			//_createValue
			for (auto & d : cl->definitions())
			{
				if (dynamic_cast<Language::TypeDefinition*>(d.get()))
				{
					auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
					auto s = dynamic_cast<Language::Structure*>(td->type().get());
					if (s)
					{
						ctx->writeTabs(code_deepness) << "rapidjson::Value _createValue(rapidjson::Document & doc, const " << td->name() << " & in)" << std::endl;
						ctx->writeTabs(code_deepness++) << "{" << std::endl;
						ctx->writeTabs(code_deepness) << "rapidjson::Value v(rapidjson::kObjectType);" << std::endl;

						for (auto & m : s->members())
							ctx->writeTabs(code_deepness) << "_addValue(doc, v, \"" << m->name() << "\", in." << m->name() << ");" << std::endl;

						ctx->writeTabs(code_deepness) << "return v;" << std::endl;
						ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					}
				}
				else if (dynamic_cast<Language::Object*>(d.get()))
				{
					auto obj = dynamic_cast<Language::Object*>(d.get());
					ctx->writeTabs(code_deepness) << "rapidjson::Value _createValue(rapidjson::Document & doc, const " << obj->name() << "::Ptr & in)" << std::endl;
					ctx->writeTabs(code_deepness) << "{ return PIDL::JSONTools::createValue(doc, in->_id()); }" << std::endl << std::endl;
				}
			}

			ctx->writeTabs(code_deepness) << "template<typename T> rapidjson::Value _createValue(rapidjson::Document & doc, const T & t)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ return PIDL::JSONTools::createValue(doc, t); }" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "template<typename T> rapidjson::Value _createValue(rapidjson::Document & doc, const std::vector<T> & values)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "rapidjson::Value v(rapidjson::kArrayType);" << std::endl;
			ctx->writeTabs(code_deepness) << "for (auto & _v : values)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ auto tmp = _createValue(doc, _v); v.PushBack(tmp, doc.GetAllocator()); }" << std::endl;
			ctx->writeTabs(code_deepness) << "return v;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Value _createValue(rapidjson::Document & doc, const std::vector<char> & blob)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ return PIDL::JSONTools::createValue(doc, blob); }" << std::endl << std::endl;

			//_addValue
			ctx->writeTabs(code_deepness) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const T & v)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ auto tmp = _createValue(doc, v); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<T> & values)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ auto tmp = _createValue<T>(doc, values); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<char> & blob)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ PIDL::JSONTools::addValue(doc, r, name, blob); }" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable_const_ref<T> & v)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "if (v.isNull()) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
			ctx->writeTabs(code_deepness) << "else _addValue(doc, r, name, *v);" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable<T> & v)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "if (v.isNull()) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
			ctx->writeTabs(code_deepness) << "else _addValue(doc, r, name, *v);" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			return true;
		}

		template<class Class_T>
		bool writeConstructorBody(Class_T * cl, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
		{
			auto write_privs = [&](bool is_object) {
				if (is_object)
					switch (ctx->mode())
					{
						case Mode::AllInOne:
							ctx->writeTabs(code_deepness) << "auto _intf_p = _intf;" << std::endl;
							break;
						case Mode::Declaration:
							break;
						case Mode::Implementatinon:
							ctx->writeTabs(code_deepness) << "auto _intf_p = _intf->_priv;" << std::endl;
							break;
					}
				else
					ctx->writeTabs(code_deepness) << "auto _intf_p = this;" << std::endl;
			};

			switch (ctx->role())
			{
			case Role::Client:
				break;
			case Role::Server:
				for (auto & d : cl->definitions())
				{
					if (dynamic_cast<Language::Function*>(d.get()))
					{
						auto function = dynamic_cast<Language::Function*>(d.get());

						ctx->writeTabs(code_deepness++) << "_functions[\"" << function->hash() << "\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;
						write_privs(dynamic_cast<Language::Method*>(d.get()) != nullptr);
						for (auto & a : function->arguments())
						{
							auto & o = ctx->writeTabs(code_deepness);
							if (!that->writeType(a->type().get(), code_deepness, ctx, ec))
								return false;
							o << " _arg_" << a->name() << ";" << std::endl;
						}

						auto in_args = function->in_arguments();
						if (in_args.size())
						{
							ctx->writeTabs(code_deepness) << "rapidjson::Value * aa;" << std::endl;
							ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(r, \"arguments\", rapidjson::kObjectType, aa, ec))" << std::endl;
							ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;

							auto & o = ctx->writeTabs(code_deepness++) << "if (" << std::endl;
							bool is_first = true;
							for (auto & a : in_args)
							{
								ctx->writeTabs(code_deepness);
								if (!is_first)
									o << "| ";
								is_first = false;
								o << "!_intf_p->_getValue(*aa, \"" << a->name() << "\", _arg_" << a->name() << ", ec)" << std::endl;
							}
							ctx->writeTabs(--code_deepness) << ")" << std::endl;
							ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;
						}

						auto ret_type = function->returnType().get();
						if (dynamic_cast<Language::Void*>(function->returnType().get()))
							ret_type = nullptr;
						if (ret_type)
						{
							ctx->writeTabs(code_deepness);
							if (!that->writeType(ret_type, code_deepness, ctx, ec))
								return false;
							*ctx << " retval;" << std::endl;
						}

						auto & o = ctx->writeTabs(code_deepness);
						o << "auto stat = _callFunction([&](){";
						if (ret_type)
							o << "retval =";
						o << " _that->" << function->name() << "(";
						bool is_first = true;
						for (auto & a : function->arguments())
						{
							if (!is_first)
								o << ", ";
							is_first = false;
							o << "_arg_" << a->name();
						}
						o << "); }, ec);" << std::endl;
						ctx->writeTabs(code_deepness) << "if (stat != _InvokeStatus::Ok)" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "return stat;" << std::endl;

						ctx->writeTabs(code_deepness) << "ret.SetObject();" << std::endl;

						if (ret_type)
							ctx->writeTabs(code_deepness) << "_intf_p->_addValue(ret, ret, \"retval\", retval);" << std::endl;

						const auto & out_args = function->out_arguments();
						if (out_args.size())
						{
							ctx->writeTabs(code_deepness) << "rapidjson::Value out_v(rapidjson::kObjectType);" << std::endl;
							for (auto & a : out_args)
								ctx->writeTabs(code_deepness) << "_intf_p->_addValue(ret, out_v, \"" << a->name() << "\", _arg_" << a->name() << ");" << std::endl;
							ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(ret, ret, \"output\", out_v);" << std::endl;
						}

						ctx->writeTabs(code_deepness) << "return _InvokeStatus::Ok;" << std::endl;
						ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
					}
					else if (dynamic_cast<Language::Property*>(d.get()))
					{
						auto property = dynamic_cast<Language::Property*>(d.get());

					//getter
						ctx->writeTabs(code_deepness++) << "_functions[\"_get_" << property->name() << "\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;
						write_privs(true);
						auto ret_type = property->type().get();

						ctx->writeTabs(code_deepness);
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " retval;" << std::endl;

						ctx->writeTabs(code_deepness) << "auto stat = _callFunction([&](){ retval = _that->get_" << property->name() << "(); }, ec);" << std::endl;
						ctx->writeTabs(code_deepness) << "if (stat != _InvokeStatus::Ok)" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "return stat;" << std::endl;

						ctx->writeTabs(code_deepness) << "ret.SetObject();" << std::endl;

						if (ret_type)
							ctx->writeTabs(code_deepness) << "_intf_p->_addValue(ret, ret, \"retval\", retval);" << std::endl;

						ctx->writeTabs(code_deepness) << "return _InvokeStatus::Ok;" << std::endl;
						ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;

						//setter
						if (!property->readOnly())
						{
							ctx->writeTabs(code_deepness++) << "_functions[\"_set_" << property->name() << "\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;
							write_privs(true);
							auto type = property->type().get();
							ctx->writeTabs(code_deepness);
							if (!that->writeType(type, code_deepness, ctx, ec))
								return false;
							*ctx << " value;" << std::endl;
							ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(r, \"value\", value, ec))" << std::endl;
							ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;

							ctx->writeTabs(code_deepness) << "return _callFunction([&](){ _that->set_" << property->name() << "(value); }, ec);" << std::endl;
							ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
						}
					}
				}
				if (dynamic_cast<Language::Interface*>(cl))
				{
					ctx->writeTabs(code_deepness++) << "_functions[\"_dispose_object\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;
					write_privs(false);
					ctx->writeTabs(code_deepness) << "std::string _arg_object_id;" << std::endl;
					ctx->writeTabs(code_deepness) << "nullable<string> _arg_comment;" << std::endl;
					ctx->writeTabs(code_deepness) << "rapidjson::Value * aa;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(r, \"arguments\", rapidjson::kObjectType, aa, ec))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(*aa, \"object_id\", _arg_object_id, ec))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;
					ctx->writeTabs(code_deepness) << "return  _callFunction([&]() { _that->_dispose_object(_arg_object_id); }, ec);" << std::endl;
					ctx->writeTabs(--code_deepness) << "};" << std::endl;
				}

				break;
			}
			return true;
		}
	};

	JSON_STL_CodeGen::JSON_STL_CodeGen(const std::shared_ptr<CPPCodeGenHelper> & helper) :
		CPPCodeGen(),
		priv(new Priv(this, helper))
	{ }

	JSON_STL_CodeGen::JSON_STL_CodeGen() :
		CPPCodeGen(),
		priv(new Priv(this, std::make_shared<CPPBasicCodeGenHelper>()))
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
			writeInclude(code_deepness, ctx, std::make_pair(CPPCodeGenHelper::IncludeType::GLobal, "memory"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/datetime.h" : "datetime.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/exception.h" : "exception.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/nullable.h" : "nullable.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/jsontools.h" : "jsontools.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/errorcollector.h" : "errorcollector.h"), ec);
	}

	bool JSON_STL_CodeGen::writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		ctx->writeTabs(code_deepness) << "template<typename T> using nullable = PIDL::Nullable<T>;" << std::endl;
		ctx->writeTabs(code_deepness) << "template<typename T> using nullable_const_ref = PIDL::NullableConstRef<T>;" << std::endl;
		ctx->writeTabs(code_deepness) << "template<typename T> using array = std::vector<T>;" << std::endl;
		ctx->writeTabs(code_deepness) << "using string = std::string;" << std::endl;
		ctx->writeTabs(code_deepness) << "using datetime = PIDL::DateTime;" << std::endl;
		ctx->writeTabs(code_deepness) << "using blob = std::vector<char>;" << std::endl;
		ctx->writeTabs(code_deepness) << "using exception = PIDL::Exception;" << std::endl;
		ctx->writeTabs(code_deepness) << "using error_collector = PIDL::ErrorCollector;" << std::endl;
		ctx->writeTabs(code_deepness) << "enum class _InvokeStatus { Ok, NotImplemented, Error, MarshallingError, FatalError };" << std::endl << std::endl;

		return true;
	}

	bool JSON_STL_CodeGen::writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		return priv->writePrivateMembers(code_deepness, ctx, intf, ec);
	}

	bool JSON_STL_CodeGen::writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		if (ctx->role() == Role::Client && ctx->mode() == Mode::Implementatinon)
			return true;

		switch (ctx->role())
		{
		case Role::Server:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "_InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << priv->getScope(intf) << intf->name() << "::_InvokeStatus " << intf->name() << "::_invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
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
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				switch (ctx->mode())
				{
				case Mode::Declaration:
					break;
				case Mode::AllInOne:
					ctx->writeTabs(code_deepness) << "auto * _p = this;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					ctx->writeTabs(code_deepness) << "auto * _p = _priv;" << std::endl << std::endl;
					break;
				}

				ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (PIDL::JSONTools::getValue(root, \"function\", v) && v->IsObject())" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string name;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "return _p->_callFunction(name, *v, ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "else if (PIDL::JSONTools::getValue(root, \"object_call\", v) && v->IsObject())" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string object_id;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_p->_getValue(*v, \"object_id\", object_id, ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "auto obj = _get_object(object_id, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!obj)" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::Error;" << std::endl;
				ctx->writeTabs(code_deepness) << "return obj->_invoke(*v, ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus::MarshallingError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			}
			break;
		case Role::Client:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "virtual _InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec) = 0;" << std::endl;
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
		switch (ctx->role())
		{
		case Role::Client:
			if (dynamic_cast<Language::Method*>(function))
			{
				switch (ctx->mode())
				{
				case Mode::AllInOne:
					ctx->writeTabs(code_deepness) << "auto _p = this;" << std::endl;
					ctx->writeTabs(code_deepness) << "auto _intf_p = _p->_intf;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					ctx->writeTabs(code_deepness) << "auto _p = _priv;" << std::endl;
					ctx->writeTabs(code_deepness) << "auto _intf_p = _p->_intf->_priv;" << std::endl << std::endl;
					break;
				case Mode::Declaration:
					break;
				}

				ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
				ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _r, \"object_id\", _p->__id);" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << function->hash() << "\");" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Value _aa(rapidjson::kObjectType);" << std::endl;

				for (auto & a : function->in_arguments())
					ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;

				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _v, \"arguments\", _aa);" << std::endl;
				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _r, \"method\", _v);" << std::endl;
				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _doc, \"object_call\", _r);" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Document _ret;" << std::endl;
			}
			else
			{
				switch (ctx->mode())
				{
				case Mode::AllInOne:
					ctx->writeTabs(code_deepness) << "auto _p = this;" << std::endl;
					ctx->writeTabs(code_deepness) << "auto _intf_p = this;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					ctx->writeTabs(code_deepness) << "auto _p = _priv;" << std::endl;
					ctx->writeTabs(code_deepness) << "auto _intf_p = _p;" << std::endl << std::endl;
					break;
				case Mode::Declaration:
					break;
				}

				ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
				ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << function->hash() << "\");" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Value _aa(rapidjson::kObjectType);" << std::endl;

				for (auto & a : function->in_arguments())
					ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;

				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _v, \"arguments\", _aa);" << std::endl;
				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _doc, \"function\", _v);" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Document _ret;" << std::endl;
			}


			{
				ctx->writeTabs(code_deepness) << "if (!_intf_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "_ec.throwException();" << std::endl;

				auto ret_type = function->returnType().get();
				if (!dynamic_cast<Language::Void*>(ret_type))
				{
					ctx->writeTabs(code_deepness);
					if (!writeType(ret_type, code_deepness, ctx, ec))
						return false;
					*ctx << " _retval;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(_ret, \"retval\", _retval, _ec))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "_ec.throwException();" << std::endl;
				}

				const auto & out_args = function->out_arguments();
				if (out_args.size())
				{
					ctx->writeTabs(code_deepness) << "rapidjson::Value * _out_v;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(_ret, \"output\", rapidjson::kObjectType, _out_v, _ec))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "_ec.throwException();" << std::endl;

					auto & o = ctx->writeTabs(code_deepness++) << "if (" << std::endl;

					bool is_first = true;
					for (auto & a : out_args)
					{
						ctx->writeTabs(code_deepness);
						if (!is_first)
							o << "| ";
						is_first = false;
						o << "!_intf_p->_getValue(*_out_v, \"" << a->name() << "\", " << a->name() << ", _ec)" << std::endl;
					}
					ctx->writeTabs(--code_deepness) << ")" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "_ec.throwException();" << std::endl;
				}

				if (!dynamic_cast<Language::Void*>(ret_type))
					ctx->writeTabs(code_deepness) << "return _retval;" << std::endl;
			}
			break;
		case Role::Server:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeConstructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writeConstructorBody(intf, code_deepness, ctx, ec);
	}

	bool JSON_STL_CodeGen::writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec)
	{
		if (ctx->role() == Role::Client && ctx->mode() == Mode::Implementatinon)
			return true;

		switch (ctx->role())
		{
		case Role::Server:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "_InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness)
					<< priv->getScope(object)
					<< "_InvokeStatus " << priv->getScope(object) << object->name()
					<< "::_invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
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
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				switch (ctx->mode())
				{
				case Mode::Declaration:
					break;
				case Mode::AllInOne:
					ctx->writeTabs(code_deepness) << "auto * _p = this;" << std::endl;
					ctx->writeTabs(code_deepness) << "auto * _intf_p = _p->_intf;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					ctx->writeTabs(code_deepness) << "auto * _p = _priv;" << std::endl;
					ctx->writeTabs(code_deepness) << "auto * _intf_p = _p->_intf->_priv;" << std::endl << std::endl;
					break;
				}

				ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (_intf_p->_getValue(root, \"method\", rapidjson::kObjectType, v, ec))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string name;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				ctx->writeTabs(code_deepness+1) << "return _InvokeStatus::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "return _p->_callFunction(name, *v, ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "else if (_intf_p->_getValue(root, \"property_get\", rapidjson::kObjectType, v, ec))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string name;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "return _p->_callFunction(\"_get_\" + name, *v, ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "else if (_intf_p->_getValue(root, \"property_set\", rapidjson::kObjectType, v, ec))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string name;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return _InvokeStatus::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "return _p->_callFunction(\"_set_\" + name, *v, ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus::MarshallingError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				break;
			}

			break;
		case Role::Client:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec)
	{
		return priv->writePrivateMembers(code_deepness, ctx, object, ec);
		return true;
	}

	bool JSON_STL_CodeGen::writePropertyGetterBody(Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		switch (ctx->mode())
		{
		case Mode::AllInOne:
			ctx->writeTabs(code_deepness) << "auto _p = this;" << std::endl;
			ctx->writeTabs(code_deepness) << "auto * _intf_p = _p->_intf;" << std::endl << std::endl;
			break;
		case Mode::Implementatinon:
			ctx->writeTabs(code_deepness) << "auto _p = _priv;" << std::endl;
			ctx->writeTabs(code_deepness) << "auto * _intf_p = _p->_intf->_priv;" << std::endl << std::endl;
			break;
		case Mode::Declaration:
			break;
		}

		switch (ctx->role())
		{
		case Role::Client:
		{
			ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
			ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
			ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _r, \"object_id\", _p->__id);" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << property->name() << "\");" << std::endl;
			ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _r, \"property_get\", _v);" << std::endl;
			ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _doc, \"object_call\", _r);" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Document _ret;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "_ec.throwException();" << std::endl;

			auto ret_type = property->type().get();
			ctx->writeTabs(code_deepness);
			if (!writeType(ret_type, code_deepness, ctx, ec))
				return false;
			*ctx << " _retval;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(_ret, \"retval\", _retval, _ec))" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "_ec.throwException();" << std::endl;

			if (!dynamic_cast<Language::Void*>(ret_type))
				ctx->writeTabs(code_deepness) << "return _retval;" << std::endl;
		}
		break;
		case Role::Server:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writePropertySetterBody(Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		switch (ctx->mode())
		{
		case Mode::AllInOne:
			ctx->writeTabs(code_deepness) << "auto _p = this;" << std::endl;
			ctx->writeTabs(code_deepness) << "auto * _intf_p = _p->_intf;" << std::endl << std::endl;
			break;
		case Mode::Implementatinon:
			ctx->writeTabs(code_deepness) << "auto _p = _priv;" << std::endl;
			ctx->writeTabs(code_deepness) << "auto * _intf_p = _p->_intf->_priv;" << std::endl << std::endl;
			break;
		case Mode::Declaration:
			break;
		}

		switch (ctx->role())
		{
		case Role::Client:
		{

			ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
			ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
			ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _r, \"object_id\", _p->__id);" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << property->name() << "\");" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"value\", value);" << std::endl;
			ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _r, \"property_set\", _v);" << std::endl;
			ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _doc, \"object_call\", _r);" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Document _ret;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "_ec.throwException();" << std::endl;
		}
		break;
		case Role::Server:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeConstructorBody(Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writeConstructorBody(object, code_deepness, ctx, ec);
	}

	bool JSON_STL_CodeGen::writeObjectBase(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		ctx->writeTabs(code_deepness) << "class _Object" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness - 1) << "public:" << std::endl;
		ctx->writeTabs(code_deepness) << "typedef std::shared_ptr<_Object> Ptr;" << std::endl;
		ctx->writeTabs(code_deepness) << "virtual ~_Object() = default;" << std::endl;
		switch (ctx->role())
		{
		case Role::Client:
			break;
		case Role::Server:
			ctx->writeTabs(code_deepness) << "virtual _InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec) = 0;" << std::endl;
			break;
		}
		ctx->writeTabs(code_deepness) << "virtual std::string _id() const = 0;" << std::endl;
		ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
		return true;
	}

	bool JSON_STL_CodeGen::writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		if (!CPPCodeGen::writeProtectedSection(code_deepness, ctx, intf, ec))
			return false;
		switch (ctx->mode())
		{
		case Mode::AllInOne:
		case Mode::Declaration:
			switch (ctx->role())
			{
			case Role::Client:
				break;
			case Role::Server:
				ctx->writeTabs(code_deepness) << "virtual _Object::Ptr _get_object(const std::string & object_id, error_collector & ec) = 0;" << std::endl;
				ctx->writeTabs(code_deepness) << "template<class Object_T> typename Object_T::Ptr _get_object(const std::string & object_id, error_collector & ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "auto o = _get_object(object_id, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!o) return nullptr;" << std::endl;
				ctx->writeTabs(code_deepness) << "auto ret = std::dynamic_pointer_cast<Object_T, _Object>(o);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!ret) ec.add(-1, \"unexpected: invalid object type for id '\" + object_id + \"'\");" << std::endl;
				ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "virtual void _dispose_object(const std::string & object_id) = 0;" << std::endl;
				break;
			}
			break;
		case Mode::Implementatinon:
			break;
		}
		return true;
	}

	bool JSON_STL_CodeGen::writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		if (!CPPCodeGen::writePublicSection(code_deepness, ctx, intf, ec))
			return false;
		switch (ctx->role())
		{
		case Role::Client:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "void _dispose_object(const std::string & object_id)";
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << "void " << priv->getScope(intf) << intf->name() << "::_dispose_object(const std::string & object_id)" << std::endl;
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
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				switch (ctx->mode())
				{
				case Mode::Declaration:
				case Mode::AllInOne:
					ctx->writeTabs(code_deepness) << "auto _p = this;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					ctx->writeTabs(code_deepness) << "auto _p = _priv;" << std::endl << std::endl;
					break;
				}

				ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
				ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
				ctx->writeTabs(code_deepness) << "_p->_addValue(_doc, _v, \"name\", \"_dispose_object\");" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Value _aa(rapidjson::kObjectType);" << std::endl;
				ctx->writeTabs(code_deepness) << "_p->_addValue(_doc, _aa, \"object_id\", object_id);" << std::endl;
				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _v, \"arguments\", _aa);" << std::endl;
				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _doc, \"function\", _v);" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Document _ret;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
				ctx->writeTabs(code_deepness) << "_ec.throwException();" << std::endl;
				ctx->writeTabs(code_deepness) << "}" << std::endl << std::endl;
				break;
			}
			break;
		case Role::Server:
			break;
		}
		return true;
	}

	bool JSON_STL_CodeGen::writeDestructorBody(Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Client:
			ctx->writeTabs(code_deepness) << "try { _intf->_dispose_object(__id); } catch(...) { }" << std::endl;
			break;
		case Role::Server:
			break;
		}
		return CPPCodeGen::writeDestructorBody(object, code_deepness, ctx, ec);
	}

}
