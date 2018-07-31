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

		std::ostream & writeTabs(short code_deepness, CPPCodeGenContext * ctx) const
		{
			char ch;
			for (short i = 0, l = code_deepness * helper->tabDefinition(ch); i < l; ++i)
				*ctx << ch;
			return ctx->stream();
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
			writeTabs(code_deepness, ctx) << "//private members" << std::endl << std::endl;

			switch (ctx->role())
			{
			case Role::Server:
				//				writeTabs(code_deepness, ctx) << "std::map<std::string, _Object::Ptr> _objects;" << std::endl;
				writeTabs(code_deepness, ctx) << "typedef std::function<_InvokeStatus(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)> _Function;" << std::endl;
				writeTabs(code_deepness, ctx) << "std::map<std::string, _Function> _functions;" << std::endl;
				writeTabs(code_deepness, ctx) << "_InvokeStatus _callFunction(const std::string & name, const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "if (!_functions.count(name))" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "ec << \"function '\" + name + \"'is not found\";" << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus::NotImplemented;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "return _functions[name](root, ret, ec);" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

				writeTabs(code_deepness, ctx) << "_InvokeStatus _callFunction(std::function<void(void)> func, error_collector & ec)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "try" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "func();" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "catch (exception * e)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "e->get(ec); " << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus::Error;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "catch (std::exception * e)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "ec.add((long)_InvokeStatus::FatalError, std::string() + \"unhandled exception: '\" + e->what() + \"'\");" << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus::FatalError;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "catch (...)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "ec.add((long)_InvokeStatus::FatalError, \"unknown unhandled exception\");" << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus::FatalError;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus::Ok;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

				break;
			case Role::Client:
				writeTabs(code_deepness, ctx) << "bool _invokeCall(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (dynamic_cast<Language::Object*>(cl))
					writeTabs(code_deepness, ctx) << "auto status = _intf->_invoke(root, ret, ec);" << std::endl;
				else
					writeTabs(code_deepness, ctx) << "auto status = _that->_invoke(root, ret, ec);" << std::endl;
				writeTabs(code_deepness, ctx) << "switch(status)" << std::endl;
				writeTabs(code_deepness, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus::Ok: break;" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus::NotImplemented:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "ec.add((long)status, \"function is not implemented\"); return false;" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus::Error:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "ec.add((long)status, \"error while executing server function\"); return false;" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus::FatalError:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "ec.add((long)status, \"fatal error while executing server function\"); return false;" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus::MarshallingError:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "ec.add((long)status, \"error while marshalling of function call\"); return false;" << std::endl;
				writeTabs(code_deepness, ctx) << "}" << std::endl << std::endl;
				writeTabs(code_deepness, ctx) << "return true;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			}

			//_getValue
			writeTabs(code_deepness, ctx) << "//marshallers" << std::endl;
			writeTabs(code_deepness, ctx) << "bool _getValue(const rapidjson::Value & v, const char * name, rapidjson::Type type, rapidjson::Value *& ret, error_collector & ec)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(v, name, ret))" << std::endl;
			writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl;
			writeTabs(code_deepness, ctx) << "if (v.GetType() != type)" << std::endl;
			writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value '\" + name + \"' is invalid\"; return false; }" << std::endl;
			writeTabs(code_deepness, ctx) << "return true;" << std::endl;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "template<typename T> bool _getValue(const rapidjson::Value & v, T & ret, error_collector & ec)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(v, ret))" << std::endl;
			writeTabs(code_deepness, ctx) << "{ ec << \"value is invalid\"; return false; }" << std::endl << std::endl;
			writeTabs(code_deepness, ctx) << "return true;" << std::endl;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "template<typename T> bool _getValue(const rapidjson::Value & r, const char * name, T & ret, error_collector & ec)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
			writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(r, name, v) || v->IsNull())" << std::endl;
			writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value '\" + name + \"' is not found or null\"; return false; }" << std::endl << std::endl;
			writeTabs(code_deepness, ctx) << "return _getValue(*v, ret, ec);" << std::endl;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "template<typename T> bool _getValue(const rapidjson::Value & r, const char * name, nullable<T> & ret, error_collector & ec)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
			writeTabs(code_deepness, ctx) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
			writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl << std::endl;
			writeTabs(code_deepness, ctx) << "if (v->IsNull())" << std::endl;
			writeTabs(code_deepness, ctx) << "{ ret.setNull(); return true; }" << std::endl;
			writeTabs(code_deepness, ctx) << "return _getValue(*v, ret.setNotNull(), ec);" << std::endl;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			for (auto & d : cl->definitions())
			{

				if (dynamic_cast<Language::TypeDefinition*>(d.get()))
				{
					auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
					auto s = dynamic_cast<Language::Structure*>(td->type().get());
					if (s)
					{
						writeTabs(code_deepness, ctx) << "bool _getValue(const rapidjson::Value & v, " << td->name() << " & ret, error_collector & ec)" << std::endl;
						writeTabs(code_deepness++, ctx) << "{" << std::endl;
						writeTabs(code_deepness, ctx) << "if (!v.IsObject())" << std::endl;
						writeTabs(code_deepness, ctx) << "{ ec << std::string() + \"value of '" << td->name() << "' is not object\"; return false; }" << std::endl;
						writeTabs(code_deepness, ctx) << "return" << std::endl;
						bool is_first = true;
						for (auto & m : s->members())
						{
							if (is_first)
							{
								is_first = false;
								writeTabs(code_deepness + 1, ctx) << "  ";
							}
							else
								writeTabs(code_deepness + 1, ctx) << "& ";
							*ctx << "_getValue(v, \"" << m->name() << "\", ret." << m->name() << ", ec)" << std::endl;
						}
						writeTabs(code_deepness, ctx) << ";" << std::endl;
						writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
					}
				}
				else if (dynamic_cast<Language::Object*>(d.get()))
				{
					auto obj = dynamic_cast<Language::Object*>(d.get());
					switch (ctx->role())
					{
					case Role::Client:
						writeTabs(code_deepness, ctx) << "bool _getValue(const rapidjson::Value & v, " << obj->name() << "::Ptr & ret, error_collector & ec)" << std::endl;
						writeTabs(code_deepness++, ctx) << "{" << std::endl;
						writeTabs(code_deepness, ctx) << "std::string object_id;" << std::endl;
						writeTabs(code_deepness, ctx) << "if (!_getValue(v, object_id, ec))" << std::endl;
						writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
						writeTabs(code_deepness, ctx) << "ret = std::make_shared<" << obj->name() << ">(_that, object_id);" << std::endl;
						writeTabs(code_deepness, ctx) << "return true;" << std::endl;
						writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
						break;
					case Role::Server:
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
						writeTabs(code_deepness, ctx) << "rapidjson::Value _createValue(rapidjson::Document & doc, const " << td->name() << " & in)" << std::endl;
						writeTabs(code_deepness++, ctx) << "{" << std::endl;
						writeTabs(code_deepness, ctx) << "rapidjson::Value v(rapidjson::kObjectType);" << std::endl;

						for (auto & m : s->members())
							writeTabs(code_deepness, ctx) << "_addValue(doc, v, \"" << m->name() << "\", in." << m->name() << ");" << std::endl;

						writeTabs(code_deepness, ctx) << "return v;" << std::endl;
						writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
					}
				}
				else if (dynamic_cast<Language::Object*>(d.get()))
				{
					auto obj = dynamic_cast<Language::Object*>(d.get());
					writeTabs(code_deepness, ctx) << "rapidjson::Value _createValue(rapidjson::Document & doc, const " << obj->name() << "::Ptr & in)" << std::endl;
					writeTabs(code_deepness, ctx) << "{ return PIDL::JSONTools::createValue(doc, in->_id()); }" << std::endl << std::endl;
				}
			}

			writeTabs(code_deepness, ctx) << "template<typename T> rapidjson::Value _createValue(rapidjson::Document & doc, const T & t)" << std::endl;
			writeTabs(code_deepness, ctx) << "{ return PIDL::JSONTools::createValue(doc, t); }" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "template<typename T> rapidjson::Value _createValue(rapidjson::Document & doc, const std::vector<T> & values)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			writeTabs(code_deepness, ctx) << "rapidjson::Value v(rapidjson::kArrayType);" << std::endl;
			writeTabs(code_deepness, ctx) << "for (auto & _v : values)" << std::endl;
			writeTabs(code_deepness, ctx) << "{ auto tmp = _createValue(doc, _v); v.PushBack(tmp, doc.GetAllocator()); }" << std::endl;
			writeTabs(code_deepness, ctx) << "return v;" << std::endl;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "rapidjson::Value _createValue(rapidjson::Document & doc, const std::vector<char> & blob)" << std::endl;
			writeTabs(code_deepness, ctx) << "{ return PIDL::JSONTools::createValue(doc, blob); }" << std::endl << std::endl;

			//_addValue
			writeTabs(code_deepness, ctx) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const T & v)" << std::endl;
			writeTabs(code_deepness, ctx) << "{ auto tmp = _createValue(doc, v); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<T> & values)" << std::endl;
			writeTabs(code_deepness, ctx) << "{ auto tmp = _createValue<T>(doc, values); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const std::vector<char> & blob)" << std::endl;
			writeTabs(code_deepness, ctx) << "{ PIDL::JSONTools::addValue(doc, r, name, blob); }" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable_const_ref<T> & v)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			writeTabs(code_deepness, ctx) << "if (v.isNull()) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
			writeTabs(code_deepness, ctx) << "else _addValue(doc, r, name, *v);" << std::endl;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable<T> & v)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			writeTabs(code_deepness, ctx) << "if (v.isNull()) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
			writeTabs(code_deepness, ctx) << "else _addValue(doc, r, name, *v);" << std::endl;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

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
							writeTabs(code_deepness, ctx) << "auto _intf_p = _intf;" << std::endl;
							break;
						case Mode::Declaration:
							break;
						case Mode::Implementatinon:
							writeTabs(code_deepness, ctx) << "auto _intf_p = _intf->_priv;" << std::endl;
							break;
					}
				else
					writeTabs(code_deepness, ctx) << "auto _intf_p = this;" << std::endl;
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

						writeTabs(code_deepness++, ctx) << "_functions[\"" << function->name() << "\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;
						write_privs(dynamic_cast<Language::Method*>(d.get()) != nullptr);
						for (auto & a : function->arguments())
						{
							auto & o = writeTabs(code_deepness, ctx);
							if (!that->writeType(a->type().get(), code_deepness, ctx, ec))
								return false;
							o << " _arg_" << a->name() << ";" << std::endl;
						}

						auto in_args = function->in_arguments();
						if (in_args.size())
						{
							writeTabs(code_deepness, ctx) << "rapidjson::Value * aa;" << std::endl;
							writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(r, \"arguments\", rapidjson::kObjectType, aa, ec))" << std::endl;
							writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;

							auto & o = writeTabs(code_deepness++, ctx) << "if (" << std::endl;
							bool is_first = true;
							for (auto & a : in_args)
							{
								writeTabs(code_deepness, ctx);
								if (!is_first)
									o << "| ";
								is_first = false;
								o << "!_intf_p->_getValue(*aa, \"" << a->name() << "\", _arg_" << a->name() << ", ec)" << std::endl;
							}
							writeTabs(--code_deepness, ctx) << ")" << std::endl;
							writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
						}

						auto ret_type = function->returnType().get();
						if (dynamic_cast<Language::Void*>(function->returnType().get()))
							ret_type = nullptr;
						if (ret_type)
						{
							writeTabs(code_deepness, ctx);
							if (!that->writeType(ret_type, code_deepness, ctx, ec))
								return false;
							*ctx << " retval;" << std::endl;
						}

						auto & o = writeTabs(code_deepness, ctx);
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
						writeTabs(code_deepness, ctx) << "if (stat != _InvokeStatus::Ok)" << std::endl;
						writeTabs(code_deepness + 1, ctx) << "return stat;" << std::endl;

						writeTabs(code_deepness, ctx) << "ret.SetObject();" << std::endl;

						if (ret_type)
							writeTabs(code_deepness, ctx) << "_intf_p->_addValue(ret, ret, \"retval\", retval);" << std::endl;

						const auto & out_args = function->out_arguments();
						if (out_args.size())
						{
							writeTabs(code_deepness, ctx) << "rapidjson::Value out_v(rapidjson::kObjectType);" << std::endl;
							for (auto & a : out_args)
								writeTabs(code_deepness, ctx) << "_intf_p->_addValue(ret, out_v, \"" << a->name() << "\", _arg_" << a->name() << ");" << std::endl;
							writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(ret, ret, \"output\", out_v);" << std::endl;
						}

						writeTabs(code_deepness, ctx) << "return _InvokeStatus::Ok;" << std::endl;
						writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
					}
					else if (dynamic_cast<Language::Property*>(d.get()))
					{
						auto property = dynamic_cast<Language::Property*>(d.get());

					//getter
						writeTabs(code_deepness++, ctx) << "_functions[\"_get_" << property->name() << "\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;
						write_privs(true);
						auto ret_type = property->type().get();

						writeTabs(code_deepness, ctx);
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " retval;" << std::endl;

						writeTabs(code_deepness, ctx) << "auto stat = _callFunction([&](){ retval = _that->get_" << property->name() << "(); }, ec);" << std::endl;
						writeTabs(code_deepness, ctx) << "if (stat != _InvokeStatus::Ok)" << std::endl;
						writeTabs(code_deepness + 1, ctx) << "return stat;" << std::endl;

						writeTabs(code_deepness, ctx) << "ret.SetObject();" << std::endl;

						if (ret_type)
							writeTabs(code_deepness, ctx) << "_intf_p->_addValue(ret, ret, \"retval\", retval);" << std::endl;

						writeTabs(code_deepness, ctx) << "return _InvokeStatus::Ok;" << std::endl;
						writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;

						//setter
						if (!property->readOnly())
						{
							writeTabs(code_deepness++, ctx) << "_functions[\"_set_" << property->name() << "\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;
							write_privs(true);
							auto type = property->type().get();
							writeTabs(code_deepness, ctx);
							if (!that->writeType(type, code_deepness, ctx, ec))
								return false;
							*ctx << " value;" << std::endl;
							writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(r, \"value\", value, ec))" << std::endl;
							writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;

							writeTabs(code_deepness, ctx) << "return _callFunction([&](){ _that->set_" << property->name() << "(value); }, ec);" << std::endl;
							writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
						}
					}
				}
				if (dynamic_cast<Language::Interface*>(cl))
				{
					writeTabs(code_deepness++, ctx) << "_functions[\"_dispose_object\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, error_collector & ec)->_InvokeStatus {" << std::endl;
					write_privs(false);
					writeTabs(code_deepness, ctx) << "std::string _arg_object_id;" << std::endl;
					writeTabs(code_deepness, ctx) << "nullable<string> _arg_comment;" << std::endl;
					writeTabs(code_deepness, ctx) << "rapidjson::Value * aa;" << std::endl;
					writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(r, \"arguments\", rapidjson::kObjectType, aa, ec))" << std::endl;
					writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
					writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(*aa, \"object_id\", _arg_object_id, ec))" << std::endl;
					writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
					writeTabs(code_deepness, ctx) << "return  _callFunction([&]() { _that->_dispose_object(_arg_object_id); }, ec);" << std::endl;
					writeTabs(--code_deepness, ctx) << "};" << std::endl;
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
		priv(new Priv(this, std::make_shared<CPPBasicCodegenHelper>()))
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
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using nullable = PIDL::Nullable<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using nullable_const_ref = PIDL::NullableConstRef<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "template<typename T> using array = std::vector<T>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using string = std::string;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using datetime = PIDL::DateTime;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using blob = std::vector<char>;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using exception = PIDL::Exception;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using error_collector = PIDL::ErrorCollector;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "enum class _InvokeStatus { Ok, NotImplemented, Error, MarshallingError, FatalError };" << std::endl << std::endl;

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
				priv->writeTabs(code_deepness, ctx) << "_InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
				break;
			case Mode::Implementatinon:
				priv->writeTabs(code_deepness, ctx) << priv->getScope(intf) << intf->name() << "::_InvokeStatus " << intf->name() << "::_invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
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
					priv->writeTabs(code_deepness, ctx) << "auto * _p = this;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					priv->writeTabs(code_deepness, ctx) << "auto * _p = _priv;" << std::endl << std::endl;
					break;
				}

				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (PIDL::JSONTools::getValue(root, \"function\", v) && v->IsObject())" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "std::string name;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return _p->_callFunction(name, *v, ret, ec);" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "else if (PIDL::JSONTools::getValue(root, \"object_call\", v) && v->IsObject())" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "std::string object_id;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!_p->_getValue(*v, \"object_id\", object_id, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "auto obj = _get_object(object_id, ec);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!obj)" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::Error;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return obj->_invoke(*v, ret, ec);" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			}
			break;
		case Role::Client:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				priv->writeTabs(code_deepness, ctx) << "virtual _InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec) = 0;" << std::endl;
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
					priv->writeTabs(code_deepness, ctx) << "auto _p = this;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "auto _intf_p = _p->_intf;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					priv->writeTabs(code_deepness, ctx) << "auto _p = _priv;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "auto _intf_p = _p->_intf->_priv;" << std::endl << std::endl;
					break;
				case Mode::Declaration:
					break;
				}

				priv->writeTabs(code_deepness, ctx) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _doc;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_doc.SetObject();" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _r, \"object_id\", _p->__id);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << function->name() << "\");" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _aa(rapidjson::kObjectType);" << std::endl;

				for (auto & a : function->in_arguments())
					priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;

				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _v, \"arguments\", _aa);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _r, \"method\", _v);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _doc, \"object_call\", _r);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _ret;" << std::endl;
			}
			else
			{
				switch (ctx->mode())
				{
				case Mode::AllInOne:
					priv->writeTabs(code_deepness, ctx) << "auto _p = this;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "auto _intf_p = this;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					priv->writeTabs(code_deepness, ctx) << "auto _p = _priv;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "auto _intf_p = _p;" << std::endl << std::endl;
					break;
				case Mode::Declaration:
					break;
				}

				priv->writeTabs(code_deepness, ctx) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _doc;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_doc.SetObject();" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << function->name() << "\");" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _aa(rapidjson::kObjectType);" << std::endl;

				for (auto & a : function->in_arguments())
					priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;

				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _v, \"arguments\", _aa);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _doc, \"function\", _v);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _ret;" << std::endl;
			}


			{
				priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "_ec.throwException();" << std::endl;

				auto ret_type = function->returnType().get();
				if (!dynamic_cast<Language::Void*>(ret_type))
				{
					priv->writeTabs(code_deepness, ctx);
					if (!writeType(ret_type, code_deepness, ctx, ec))
						return false;
					*ctx << " _retval;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(_ret, \"retval\", _retval, _ec))" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "_ec.throwException();" << std::endl;
				}

				const auto & out_args = function->out_arguments();
				if (out_args.size())
				{
					priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * _out_v;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(_ret, \"output\", rapidjson::kObjectType, _out_v, _ec))" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "_ec.throwException();" << std::endl;

					auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;

					bool is_first = true;
					for (auto & a : out_args)
					{
						priv->writeTabs(code_deepness, ctx);
						if (!is_first)
							o << "| ";
						is_first = false;
						o << "!_intf_p->_getValue(*_out_v, \"" << a->name() << "\", " << a->name() << ", _ec)" << std::endl;
					}
					priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "_ec.throwException();" << std::endl;
				}

				if (!dynamic_cast<Language::Void*>(ret_type))
					priv->writeTabs(code_deepness, ctx) << "return _retval;" << std::endl;
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
				priv->writeTabs(code_deepness, ctx) << "_InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec)";
				break;
			case Mode::Implementatinon:
				priv->writeTabs(code_deepness, ctx)
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
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				switch (ctx->mode())
				{
				case Mode::Declaration:
					break;
				case Mode::AllInOne:
					priv->writeTabs(code_deepness, ctx) << "auto * _p = this;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "auto * _intf_p = _p->_intf;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					priv->writeTabs(code_deepness, ctx) << "auto * _p = _priv;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "auto * _intf_p = _p->_intf->_priv;" << std::endl << std::endl;
					break;
				}

				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value * v;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (_intf_p->_getValue(root, \"method\", rapidjson::kObjectType, v, ec))" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "std::string name;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				priv->writeTabs(code_deepness+1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return _p->_callFunction(name, *v, ret, ec);" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "else if (_intf_p->_getValue(root, \"property_get\", rapidjson::kObjectType, v, ec))" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "std::string name;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return _p->_callFunction(\"_get_\" + name, *v, ret, ec);" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "else if (_intf_p->_getValue(root, \"property_set\", rapidjson::kObjectType, v, ec))" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "std::string name;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return _p->_callFunction(\"_set_\" + name, *v, ret, ec);" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus::MarshallingError;" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

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
			priv->writeTabs(code_deepness, ctx) << "auto _p = this;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "auto * _intf_p = _p->_intf;" << std::endl << std::endl;
			break;
		case Mode::Implementatinon:
			priv->writeTabs(code_deepness, ctx) << "auto _p = _priv;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "auto * _intf_p = _p->_intf->_priv;" << std::endl << std::endl;
			break;
		case Mode::Declaration:
			break;
		}

		switch (ctx->role())
		{
		case Role::Client:
		{
			priv->writeTabs(code_deepness, ctx) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _doc;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_doc.SetObject();" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _r, \"object_id\", _p->__id);" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << property->name() << "\");" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _r, \"property_get\", _v);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _doc, \"object_call\", _r);" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _ret;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "_ec.throwException();" << std::endl;

			auto ret_type = property->type().get();
			priv->writeTabs(code_deepness, ctx);
			if (!writeType(ret_type, code_deepness, ctx, ec))
				return false;
			*ctx << " _retval;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_getValue(_ret, \"retval\", _retval, _ec))" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "_ec.throwException();" << std::endl;

			if (!dynamic_cast<Language::Void*>(ret_type))
				priv->writeTabs(code_deepness, ctx) << "return _retval;" << std::endl;
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
			priv->writeTabs(code_deepness, ctx) << "auto _p = this;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "auto * _intf_p = _p->_intf;" << std::endl << std::endl;
			break;
		case Mode::Implementatinon:
			priv->writeTabs(code_deepness, ctx) << "auto _p = _priv;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "auto * _intf_p = _p->_intf->_priv;" << std::endl << std::endl;
			break;
		case Mode::Declaration:
			break;
		}

		switch (ctx->role())
		{
		case Role::Client:
		{

			priv->writeTabs(code_deepness, ctx) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _doc;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_doc.SetObject();" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _r, \"object_id\", _p->__id);" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << property->name() << "\");" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_intf_p->_addValue(_doc, _v, \"value\", value);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _r, \"property_set\", _v);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _doc, \"object_call\", _r);" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _ret;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_intf_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "_ec.throwException();" << std::endl;
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
		priv->writeTabs(code_deepness, ctx) << "class _Object" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness - 1, ctx) << "public:" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "typedef std::shared_ptr<_Object> Ptr;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "virtual ~_Object() = default;" << std::endl;
		switch (ctx->role())
		{
		case Role::Client:
			break;
		case Role::Server:
			priv->writeTabs(code_deepness, ctx) << "virtual _InvokeStatus _invoke(const rapidjson::Value & root, rapidjson::Document & ret, error_collector & ec) = 0;" << std::endl;
			break;
		}
		priv->writeTabs(code_deepness, ctx) << "virtual std::string _id() const = 0;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
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
				priv->writeTabs(code_deepness, ctx) << "virtual _Object::Ptr _get_object(const std::string & object_id, error_collector & ec) = 0;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "virtual void _dispose_object(const std::string & object_id) = 0;" << std::endl;
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
				priv->writeTabs(code_deepness, ctx) << "void _dispose_object(const std::string & object_id)";
				break;
			case Mode::Implementatinon:
				priv->writeTabs(code_deepness, ctx) << "void " << priv->getScope(intf) << intf->name() << "::_dispose_object(const std::string & object_id)" << std::endl;
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
				case Mode::AllInOne:
					priv->writeTabs(code_deepness, ctx) << "auto _p = this;" << std::endl << std::endl;
					break;
				case Mode::Implementatinon:
					priv->writeTabs(code_deepness, ctx) << "auto _p = _priv;" << std::endl << std::endl;
					break;
				}

				priv->writeTabs(code_deepness, ctx) << "PIDL::ExceptionErrorCollector<error_collector> _ec;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _doc;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_doc.SetObject();" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_p->_addValue(_doc, _v, \"name\", \"_dispose_object\");" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Value _aa(rapidjson::kObjectType);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_p->_addValue(_doc, _aa, \"object_id\", object_id);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _v, \"arguments\", _aa);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "PIDL::JSONTools::addValue(_doc, _doc, \"function\", _v);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "rapidjson::Document _ret;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_ec.throwException();" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "}" << std::endl << std::endl;
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
			priv->writeTabs(code_deepness, ctx) << "try { _intf->_dispose_object(__id); } catch(...) { }" << std::endl;
			break;
		case Role::Server:
			break;
		}
		return CPPCodeGen::writeDestructorBody(object, code_deepness, ctx, ec);
	}

}
