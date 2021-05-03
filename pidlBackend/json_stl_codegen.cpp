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
#include <functional>

namespace PIDL
{

	struct JSON_STL_CodeGen::Priv
	{
        Priv(JSON_STL_CodeGen * that, const std::shared_ptr<CPPCodeGenHelper> & helper, const std::set<Flag> & flags) :
            that(that),
            helper(helper),
            flags(flags)
		{ }

		JSON_STL_CodeGen * that;
		std::shared_ptr<CPPCodeGenHelper> helper;
        std::set<Flag> flags;

        bool useOptional() const
        {
            return flags.count(Flag::UseOptional);
        }

		std::string privLogger()
		{
			return "priv->logger";
		}

		bool hasObjects(Language::Interface * intf)
		{
			for (auto & d : intf->definitions())
			{
				if (std::dynamic_pointer_cast<Language::Object>(d))
					return true;
			}
			return false;
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
            (void)ec;
            ctx->writeTabs(code_deepness) << "//private members" << std::endl << std::endl;

			switch (ctx->role())
			{
			case Role::Server:
                //ctx->writeTabs(code_deepness) << "std::map<std::string, ptr<_Object> _objects;" << std::endl;
				ctx->writeTabs(code_deepness) << "typedef std::function<_invoke_status(const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec)> _Function;" << std::endl;
				ctx->writeTabs(code_deepness) << "struct _Variants { std::map<std::string, _Function> data; };" << std::endl;
				ctx->writeTabs(code_deepness) << "std::map<std::string, _Variants> _functions;" << std::endl;
				ctx->writeTabs(code_deepness) << "_invoke_status _callFunction(const std::string & name, const std::string & variant, const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_functions.count(name))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec << \"function '\" + name + \"'is not found\"; return _invoke_status::NotImplemented; }" << std::endl;
				ctx->writeTabs(code_deepness) << "auto & vars = _functions[name].data;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!variant.length() && vars.size() == 1)" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return vars.begin()->second(root, ret, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!vars.count(variant))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec << \"variant '\" + variant + \"' of function '\" + name + \"'is not found\"; return _invoke_status::NotImplemented; }" << std::endl;
				ctx->writeTabs(code_deepness) << "return vars[variant](root, ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "_invoke_status _callFunction(std::function<void(void)> func, _error_collector & ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "try" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "func();" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
                ctx->writeTabs(code_deepness) << "catch (exception & e)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "e.get(ec); " << std::endl;
				ctx->writeTabs(code_deepness) << "return _invoke_status::Error;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
                ctx->writeTabs(code_deepness) << "catch (std::exception & e)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "ec.add((long)_invoke_status::FatalError, std::string() + \"unhandled exception: '\" + e.what() + \"'\");" << std::endl;
				ctx->writeTabs(code_deepness) << "return _invoke_status::FatalError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "catch (...)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.add((long)_invoke_status::FatalError, \"unknown unhandled exception\");" << std::endl;
				ctx->writeTabs(code_deepness) << "return _invoke_status::FatalError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "return _invoke_status::Ok;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				break;
			case Role::Client:
				ctx->writeTabs(code_deepness) << "bool _invokeCall(const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (dynamic_cast<Language::Object*>(cl))
					ctx->writeTabs(code_deepness) << "auto status = _intf->_invoke(root, ret, ec);" << std::endl;
				else
					ctx->writeTabs(code_deepness) << "auto status = _that->_invoke(root, ret, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "switch(status)" << std::endl;
				ctx->writeTabs(code_deepness) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "case _invoke_status::Ok: break;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _invoke_status::NotImplemented:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"function is not implemented\"); return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _invoke_status::Error:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"error while executing server function\"); return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _invoke_status::FatalError:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"fatal error while executing server function\"); return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _invoke_status::MarshallingError:" << std::endl;
                ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"error while marshalling of function call\"); return false;" << std::endl;
                ctx->writeTabs(code_deepness) << "case _invoke_status::NotSupportedMarshallingVersion:" << std::endl;
                ctx->writeTabs(code_deepness + 1) << "ec.add((long)status, \"not supported marshalling version\"); return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "}" << std::endl << std::endl;
				ctx->writeTabs(code_deepness) << "return true;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			}

            if(auto intf = dynamic_cast<Language::Interface*>(cl))
            {
                //_getValue
                ctx->writeTabs(code_deepness) << "//marshalers" << std::endl;
                ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, const char * name, rapidjson::Type type, rapidjson::Value *& ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(v, name, ret))" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "if (v.GetType() != type)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is invalid\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "return true;" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> bool _getValue(const rapidjson::Value & v, T & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(v, ret))" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << \"value is invalid\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "return true;" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> bool _getValue(const rapidjson::Value & r, const char * name, T & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(r, name, v) || v->IsNull())" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found or null\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "return _getValue(*v, ret, ec);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> bool _getValue(const rapidjson::Value & r, const char * name, ptr<T> & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found or null\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "if(v->IsNull())" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ret.reset(); return true; }" << std::endl;
                ctx->writeTabs(code_deepness) << "return _getValue(*v, ret, ec);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> bool _getValue(const rapidjson::Value & v, nullable<T> & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "if (v.IsNull())" << std::endl;
                if(useOptional())
                {
                    ctx->writeTabs(code_deepness) << "{ ret.reset(); return true; }" << std::endl;
                    ctx->writeTabs(code_deepness) << "return _getValue(v, ret.emplace(), ec);" << std::endl;
                }
                else
                {
                    ctx->writeTabs(code_deepness) << "{ ret.setNull(); return true; }" << std::endl;
                    ctx->writeTabs(code_deepness) << "return _getValue(v, ret.setNotNull(), ec);" << std::endl;
                }
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> bool _getValue(const rapidjson::Value & r, const char * name, nullable<T> & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "return _getValue(*v, ret, ec);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template <typename T> bool _getValue(const rapidjson::Value & v, array<T> & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!v.IsArray())" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << \"value is not array\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "ret.resize(v.Size());" << std::endl;
                ctx->writeTabs(code_deepness) << "size_t i(0);" << std::endl;
                ctx->writeTabs(code_deepness) << "bool has_error = false;" << std::endl;
                ctx->writeTabs(code_deepness) << "for (auto it = v.Begin(); it != v.End(); ++it)" << std::endl;
                ctx->writeTabs(code_deepness + 1) << "if (!_getValue(*it, ret[i++], ec)) has_error = true;" << std::endl;
                ctx->writeTabs(code_deepness) << "return !has_error;" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template <typename T> bool _getValue(const rapidjson::Value & r, const char * name, array<T> & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "return _getValue(*v, ret, ec);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, blob & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "return _getValue<blob>(v, ret, ec);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & r, const char * name, blob & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "return _getValue<blob>(r, name, ret, ec);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                //tuple
                ctx->writeTabs(code_deepness) << "struct _tuple_getValue_functor" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "_tuple_getValue_functor(_Priv * priv_, const rapidjson::Value & r_, bool & has_error_, _error_collector & ec_) : priv(priv_), r(r_), has_error(has_error_), ec(ec_) { }" << std::endl;
                ctx->writeTabs(code_deepness) << "_Priv * priv;" << std::endl;
                ctx->writeTabs(code_deepness) << "const rapidjson::Value & r;" << std::endl;
                ctx->writeTabs(code_deepness) << "bool & has_error;" << std::endl;
                ctx->writeTabs(code_deepness) << "_error_collector & ec;" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::SizeType idx = 0;" << std::endl;
                ctx->writeTabs(code_deepness) << "template<typename T> void operator () (T && v)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ if (!priv->_getValue(r[idx++], v, ec)) has_error = true; }" << std::endl;
                ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename ...T> bool _getValue(const rapidjson::Value & v, tuple<T...> & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "bool has_error = false;" << std::endl;
                ctx->writeTabs(code_deepness) << "PIDL::JSONTools::for_each_in_tuple(ret, _tuple_getValue_functor(this, v, has_error, ec));" << std::endl;
                ctx->writeTabs(code_deepness) << "if (has_error) ec << \"invalid marshalling when tuple value\";" << std::endl;
                ctx->writeTabs(code_deepness) << "return !has_error;" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename ...T> bool _getValue(const rapidjson::Value & r, const char * name, tuple<T...> & ret, _error_collector & ec)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(r, name, v))" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value '\" + name + \"' is not found\"; return false; }" << std::endl;
                ctx->writeTabs(code_deepness) << "return _getValue(*v, ret, ec);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                std::function<void(Language::DefinitionProvider * cl)> add_getValue = [&](Language::DefinitionProvider * cl) {

                    for (auto & d : cl->definitions())
                    {
                        if (auto td = dynamic_cast<Language::TypeDefinition*>(d.get()))
                        {
                            if (auto s = dynamic_cast<Language::Structure*>(td->type().get()))
                            {
                                ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, " << getScope(td) << td->name() << " & ret, _error_collector & ec)" << std::endl;
                                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                                ctx->writeTabs(code_deepness) << "if (!v.IsObject())" << std::endl;
                                ctx->writeTabs(code_deepness) << "{ ec << std::string() + \"value of '" << td->name() << "' is not object\"; return false; }" << std::endl;
                                auto & members = s->members();
                                if(members.size())
                                {
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
                                }
                                else
                                    ctx->writeTabs(code_deepness) << "return true;" << std::endl;

                                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
                            }
                        }
                        else if (auto obj = dynamic_cast<Language::Object*>(d.get()))
                        {
                            switch (ctx->role())
                            {
                            case Role::Client:
                                ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, ptr<" << getScope(obj) << obj->name() << "> & ret, _error_collector & ec)" << std::endl;
                                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                                ctx->writeTabs(code_deepness) << "nullable<std::string> object_data;" << std::endl;
                                ctx->writeTabs(code_deepness) << "if (!_getValue(v, object_data, ec))" << std::endl;
                                ctx->writeTabs(code_deepness + 1) << "return false;" << std::endl;
                                ctx->writeTabs(code_deepness) << "if (!object_data)" << std::endl;
                                ctx->writeTabs(code_deepness) << "{ ret.reset(); return true; }" << std::endl;
                                ctx->writeTabs(code_deepness) << "ret = std::make_shared<" << getScope(obj) << obj->name() << ">(_that, *object_data);" << std::endl;
                                ctx->writeTabs(code_deepness) << "return true;" << std::endl;
                                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
                                break;
                            case Role::Server:
                                ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & v, ptr<" << getScope(obj) << obj->name() << "> & ret, _error_collector & ec)" << std::endl;
                                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                                ctx->writeTabs(code_deepness) << "nullable<std::string> object_data;" << std::endl;
                                ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(v, object_data))" << std::endl;
                                ctx->writeTabs(code_deepness) << "{ ec << \"value is invalid\"; return false; }" << std::endl;
                                ctx->writeTabs(code_deepness) << "if (!object_data)" << std::endl;
                                ctx->writeTabs(code_deepness) << "{ ret.reset(); return true; }" << std::endl;
                                ctx->writeTabs(code_deepness) << "return (bool)(ret = _intf->_get_object<" << getScope(obj) << obj->name() << ">(*object_data, ec));" << std::endl;
                                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                                ctx->writeTabs(code_deepness) << "bool _getValue(const rapidjson::Value & r, const char * name, ptr<" << getScope(obj) << obj->name() << "> & ret, _error_collector & ec)" << std::endl;
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

                            add_getValue(obj);
                        }
                    }
                };

                std::function<void(Language::DefinitionProvider * cl)> add_createValue = [&](Language::DefinitionProvider * cl) {
                    //_createValue
                    for (auto & d : cl->definitions())
                    {
                        if (auto td = dynamic_cast<Language::TypeDefinition*>(d.get()))
                        {
                            if (auto s = dynamic_cast<Language::Structure*>(td->type().get()))
                            {
                                ctx->writeTabs(code_deepness) << "rapidjson::Value _createValue(rapidjson::Document & doc, const " << getScope(td) << td->name() << " & in)" << std::endl;
                                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                                ctx->writeTabs(code_deepness) << "rapidjson::Value v(rapidjson::kObjectType);" << std::endl;

                                for (auto & m : s->members())
                                    ctx->writeTabs(code_deepness) << "_addValue(doc, v, \"" << m->name() << "\", in." << m->name() << ");" << std::endl;

                                ctx->writeTabs(code_deepness) << "return v;" << std::endl;
                                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
                            }
                        }
                        else if (auto obj = dynamic_cast<Language::Object*>(d.get()))
                        {
                            ctx->writeTabs(code_deepness) << "rapidjson::Value _createValue(rapidjson::Document & doc, const ptr<" << getScope(obj) << obj->name() << "> & in)" << std::endl;
                            ctx->writeTabs(code_deepness) << "{ return in ? PIDL::JSONTools::createValue(doc, in->_data()) : rapidjson::Value(rapidjson::kNullType); }" << std::endl << std::endl;

                            add_createValue(obj);
                        }
                    }
                };

                add_getValue(cl);
                add_createValue(cl);

                ctx->writeTabs(code_deepness) << "template<typename T> rapidjson::Value _createValue(rapidjson::Document & doc, const T & t)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ return PIDL::JSONTools::createValue(doc, t); }" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> rapidjson::Value _createValue(rapidjson::Document & doc, const array<T> & values)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Value v(rapidjson::kArrayType);" << std::endl;
                ctx->writeTabs(code_deepness) << "for (auto & _v : values)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ auto tmp = _createValue(doc, _v); v.PushBack(tmp, doc.GetAllocator()); }" << std::endl;
                ctx->writeTabs(code_deepness) << "return v;" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> rapidjson::Value _createValue(rapidjson::Document & doc, const nullable<T> & value)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!value) return rapidjson::Value(rapidjson::kNullType);" << std::endl;
                ctx->writeTabs(code_deepness) << "return _createValue(doc, *value);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "rapidjson::Value _createValue(rapidjson::Document & doc, const blob & data)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ return PIDL::JSONTools::createValue(doc, data); }" << std::endl << std::endl;

                //_addValue
                ctx->writeTabs(code_deepness) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const T & v)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ auto tmp = _createValue(doc, v); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const array<T> & values)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ auto tmp = _createValue<T>(doc, values); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const blob & data)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ PIDL::JSONTools::addValue(doc, r, name, data); }" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable_const_ref<T> & v)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!v) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
                ctx->writeTabs(code_deepness) << "else _addValue(doc, r, name, *v);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const nullable<T> & v)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "if (!v) PIDL::JSONTools::addNull(doc, r, name);" << std::endl;
                ctx->writeTabs(code_deepness) << "else _addValue(doc, r, name, *v);" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                //tuple
                ctx->writeTabs(code_deepness) << "struct _tuple_createValue_functor" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "_tuple_createValue_functor(_Priv * priv_, rapidjson::Document & doc_, rapidjson::Value & r_) : priv(priv_), doc(doc_), r(r_) { }" << std::endl;
                ctx->writeTabs(code_deepness) << "_Priv * priv;" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Document & doc;" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Value & r;" << std::endl;
                ctx->writeTabs(code_deepness) << "template<typename T> void operator () (T && v)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ r.PushBack(priv->_createValue(doc, v), doc.GetAllocator()); }" << std::endl;
                ctx->writeTabs(--code_deepness) << "};" << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename ...T> rapidjson::Value _createValue(rapidjson::Document & doc, const tuple<T...> & values)" << std::endl;
                ctx->writeTabs(code_deepness++) << "{" << std::endl;
                ctx->writeTabs(code_deepness) << "rapidjson::Value v(rapidjson::kArrayType);" << std::endl;
                ctx->writeTabs(code_deepness) << "PIDL::JSONTools::for_each_in_tuple(values, _tuple_createValue_functor(this, doc, v));" << std::endl;
                ctx->writeTabs(code_deepness) << "return v;" << std::endl;
                ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

                ctx->writeTabs(code_deepness) << "template<typename ...T> void _addValue(rapidjson::Document & doc, rapidjson::Value & r, const char * name, const tuple<T...> & values)" << std::endl;
                ctx->writeTabs(code_deepness) << "{ auto tmp = _createValue(doc, values); PIDL::JSONTools::addValue(doc, r, name, tmp); }" << std::endl << std::endl;
            }

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
					if (dynamic_cast<Language::FunctionVariant*>(d.get()))
					{
						auto function = dynamic_cast<Language::FunctionVariant*>(d.get());
						ctx->writeTabs(code_deepness++) << "_functions[\"" << function->name() << "\"].data[\"" << function->variantId() << "\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, _error_collector & ec)->_invoke_status {" << std::endl;
                        write_privs(dynamic_cast<Language::MethodVariant*>(d.get()) != nullptr);

                        if(helper->logging())
                        {
                            auto starter = helper->logging()->loggingStart("_logger");
                            if(starter.length())
                                ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                            auto debug = helper->logging()->loggingDebug("_logger", std::string() + "\"" + (dynamic_cast<Language::MethodVariant*>(d.get()) ? "method: " : "function: '") +
                                                                         std::string(function->name()) + "' variant: '" + function->variantId() + "'\"");
                            if(debug.length())
                                ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
                        }

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
							ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;

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
							ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;
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
						ctx->writeTabs(code_deepness) << "if (stat != _invoke_status::Ok)" << std::endl;
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

						ctx->writeTabs(code_deepness) << "return _invoke_status::Ok;" << std::endl;
						ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
					}
					else if (dynamic_cast<Language::Property*>(d.get()))
					{
						auto property = dynamic_cast<Language::Property*>(d.get());

					//getter
						ctx->writeTabs(code_deepness++) << "_functions[\"" << property->name() << "\"].data[\"get\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, _error_collector & ec)->_invoke_status {" << std::endl;
                        write_privs(true);

                        if(helper->logging())
                        {
                            auto starter = helper->logging()->loggingStart("_logger");
                            if(starter.length())
                                ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                            auto debug = helper->logging()->loggingDebug("_logger", "\"property getter: '" + std::string(property->name()) + "'\"");
                            if(debug.length())
                                ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
                        }

						auto ret_type = property->type().get();

						ctx->writeTabs(code_deepness);
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " retval;" << std::endl;

						ctx->writeTabs(code_deepness) << "auto stat = _callFunction([&](){ retval = _that->get_" << property->name() << "(); }, ec);" << std::endl;
						ctx->writeTabs(code_deepness) << "if (stat != _invoke_status::Ok)" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "return stat;" << std::endl;

						ctx->writeTabs(code_deepness) << "ret.SetObject();" << std::endl;

						if (ret_type)
							ctx->writeTabs(code_deepness) << "_intf_p->_addValue(ret, ret, \"retval\", retval);" << std::endl;

						ctx->writeTabs(code_deepness) << "return _invoke_status::Ok;" << std::endl;
						ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;

                    //setter
						if (!property->readOnly())
						{
							ctx->writeTabs(code_deepness++) << "_functions[\"" << property->name() << "\"].data[\"set\"] = [&](const rapidjson::Value & r, rapidjson::Document & ret, _error_collector & ec)->_invoke_status {" << std::endl;
                            write_privs(true);

                            if(helper->logging())
                            {
                                auto starter = helper->logging()->loggingStart("_logger");
                                if(starter.length())
                                    ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                                auto debug = helper->logging()->loggingDebug("_logger", "\"property setter: '" + std::string(property->name()) + "'\"");
                                if(debug.length())
                                    ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
                            }

							auto type = property->type().get();
							ctx->writeTabs(code_deepness);
							if (!that->writeType(type, code_deepness, ctx, ec))
								return false;
							*ctx << " value;" << std::endl;
							ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(r, \"value\", value, ec))" << std::endl;
							ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;

							ctx->writeTabs(code_deepness) << "return _callFunction([&](){ _that->set_" << property->name() << "(value); }, ec);" << std::endl;
							ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
						}
					}
				}
				if (dynamic_cast<Language::Interface*>(cl) && hasObjects(dynamic_cast<Language::Interface*>(cl)))
				{
					ctx->writeTabs(code_deepness++) << "_functions[\"_dispose_object\"].data[std::string()] = [&](const rapidjson::Value & r, rapidjson::Document & ret, _error_collector & ec)->_invoke_status {" << std::endl;
                    write_privs(false);

                    if(helper->logging())
                    {
                        auto starter = helper->logging()->loggingStart("_logger");
                        if(starter.length())
                            ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                        auto debug = helper->logging()->loggingDebug("_logger", "\"embedded function: '_dispose_object'\"");
                        if(debug.length())
                            ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
                    }

					ctx->writeTabs(code_deepness) << "std::string _arg_object_data;" << std::endl;
					ctx->writeTabs(code_deepness) << "nullable<string> _arg_comment;" << std::endl;
					ctx->writeTabs(code_deepness) << "rapidjson::Value * aa;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(r, \"arguments\", rapidjson::kObjectType, aa, ec))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(*aa, \"object_data\", _arg_object_data, ec))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;
					ctx->writeTabs(code_deepness) << "return  _callFunction([&]() { _that->_dispose_object(_arg_object_data); }, ec);" << std::endl;
					ctx->writeTabs(--code_deepness) << "};" << std::endl;
				}

				break;
			}
			return true;
		}
	};

    JSON_STL_CodeGen::JSON_STL_CodeGen(const std::shared_ptr<CPPCodeGenHelper> & helper, const std::set<Flag> & flags) :
		CPPCodeGen(),
        priv(new Priv(this, helper, flags))
	{ }

	JSON_STL_CodeGen::JSON_STL_CodeGen() :
		CPPCodeGen(),
        priv(new Priv(this, std::make_shared<CPPBasicCodeGenHelper>(), std::set<Flag>()))
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
            if (!writeInclude(code_deepness, ctx, std::make_pair(IncludeType::GLobal, "map"), ec) ||
                !writeInclude(code_deepness, ctx, std::make_pair(IncludeType::GLobal, "functional"), ec))
				return false;
			break;
		case Mode::Declaration:
			break;
		}
		return
            writeInclude(code_deepness, ctx, std::make_pair(IncludeType::GLobal, "vector"), ec) &&
            writeInclude(code_deepness, ctx, std::make_pair(IncludeType::GLobal, "tuple"), ec) &&
            writeInclude(code_deepness, ctx, std::make_pair(IncludeType::GLobal, "string"), ec) &&
            writeInclude(code_deepness, ctx, std::make_pair(IncludeType::GLobal, "memory"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/datetime.h" : "datetime.h"), ec) &&
			writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/exception.h" : "exception.h"), ec) &&
            writeInclude(code_deepness, ctx, priv->useOptional() ?
                             std::make_pair(IncludeType::GLobal, "optional") :
                             std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/nullable.h" : "nullable.h"), ec) &&
            writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/jsontools.h" : "jsontools.h"), ec) &&
            writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/basictypes.h" : "basictypes.h"), ec) &&
            writeInclude(code_deepness, ctx, std::make_pair(core_path.first, core_path.second.length() ? core_path.second + "/errorcollector.h" : "errorcollector.h"), ec);
	}

	bool JSON_STL_CodeGen::writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
        (void)ec;
        if(priv->useOptional())
        {
            ctx->writeTabs(code_deepness) << "template<typename T> using nullable = std::optional<T>;" << std::endl;
            ctx->writeTabs(code_deepness) << "template<typename T> using nullable_const_ref = std::optional<const T &>;" << std::endl;
        }
        else
        {
            ctx->writeTabs(code_deepness) << "template<typename T> using nullable = PIDL::Nullable<T>;" << std::endl;
            ctx->writeTabs(code_deepness) << "template<typename T> using nullable_const_ref = PIDL::NullableConstRef<T>;" << std::endl;
        }
		ctx->writeTabs(code_deepness) << "template<typename T> using array = std::vector<T>;" << std::endl;
		ctx->writeTabs(code_deepness) << "template<typename ...T> using tuple = std::tuple<T...>;" << std::endl;
        ctx->writeTabs(code_deepness) << "template<typename T> using ptr = std::shared_ptr<T>;" << std::endl;
        ctx->writeTabs(code_deepness) << "using string = std::string;" << std::endl;
		ctx->writeTabs(code_deepness) << "using datetime = PIDL::DateTime;" << std::endl;
		ctx->writeTabs(code_deepness) << "using blob = std::vector<char>;" << std::endl;
		ctx->writeTabs(code_deepness) << "using exception = PIDL::Exception;" << std::endl;
		ctx->writeTabs(code_deepness) << "using _error_collector = PIDL::ErrorCollector;" << std::endl;
        ctx->writeTabs(code_deepness) << "using _invoke_status = PIDL::InvokeStatus;" << std::endl << std::endl;

		return true;
	}

	bool JSON_STL_CodeGen::writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		return priv->writePrivateMembers(code_deepness, ctx, intf, ec);
	}

	bool JSON_STL_CodeGen::writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
        (void)ec;
        if (ctx->role() == Role::Client && ctx->mode() == Mode::Implementatinon)
			return true;

		switch (ctx->role())
		{
		case Role::Server:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "_invoke_status _invoke(const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec)";
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << "_invoke_status " << intf->name() << "::_invoke(const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec)";
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

				ctx->writeTabs(code_deepness) << "int version;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!PIDL::JSONTools::getValue(root, \"version\", version))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec << \"could not detect mashalling version\"; return _invoke_status::MarshallingError; }" << std::endl << std::endl;
				ctx->writeTabs(code_deepness) << "if (version != " << PIDL_JSON_MARSHALLING_VERSION << ")" << std::endl;
                ctx->writeTabs(code_deepness) << "{ ec << \"unsupported mashalling version detected\"; return _invoke_status::NotSupportedMarshallingVersion; }" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "rapidjson::Value * v;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (PIDL::JSONTools::getValue(root, \"function\", v) && v->IsObject())" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string name, variant;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::getValue(*v, \"variant\", variant);" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.clear();" << std::endl;
				ctx->writeTabs(code_deepness) << "return _p->_callFunction(name, variant, *v, ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;

				if (priv->hasObjects(intf))
				{
					ctx->writeTabs(code_deepness) << "else if (PIDL::JSONTools::getValue(root, \"object_call\", v) && v->IsObject())" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "std::string object_data;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_p->_getValue(*v, \"object_data\", object_data, ec))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;
					ctx->writeTabs(code_deepness) << "auto obj = _get_object(object_data, ec);" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!obj)" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "return _invoke_status::Error;" << std::endl;
					ctx->writeTabs(code_deepness) << "return obj->_invoke(*v, ret, ec);" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl;
				}

				ctx->writeTabs(code_deepness) << "return _invoke_status::MarshallingError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			}
			break;
		case Role::Client:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "virtual _invoke_status _invoke(const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec) = 0;" << std::endl;
				break;
			case Mode::Implementatinon:
				break;
			}
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeFunctionBody(Language::Interface * intf, Language::FunctionVariant * function, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
        (void)intf;
        switch (ctx->role())
		{
		case Role::Client:
			if (dynamic_cast<Language::MethodVariant*>(function))
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

                if(helper()->logging())
                {
                    auto starter = helper()->logging()->loggingStart("_p->_logger");
                    if(starter.length())
                        ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                    auto debug = helper()->logging()->loggingDebug("_p->_logger", "\"method: '"+std::string(function->name())+"' variant: '"+function->variantId()+"'\"");
                    if(debug.length())
                        ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
                }

				ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<_error_collector> _ec;" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
				ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;

				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _doc, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

				ctx->writeTabs(code_deepness) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _r, \"object_data\", _p->__data);" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << function->name() << "\");" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"variant\", \"" << function->variantId() << "\");" << std::endl;
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

                if(helper()->logging())
                {
                    auto starter = helper()->logging()->loggingStart("_p->_logger");
                    if(starter.length())
                        ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                    auto debug = helper()->logging()->loggingDebug("_p->_logger", "\"function: '"+std::string(function->name())+"' variant: '"+function->variantId()+"'\"");
                    if(debug.length())
                        ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
                }

				ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<_error_collector> _ec;" << std::endl;
				ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
				ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;

				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _doc, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

				ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << function->name() << "\");" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"variant\", \"" << function->variantId() << "\");" << std::endl;
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

				if (dynamic_cast<Language::MethodVariant*>(function))
					ctx->writeTabs(code_deepness) << "PIDL::JSONTools::getValue(_ret, \"object_data\", _p->__data);" << std::endl;

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

	bool JSON_STL_CodeGen::writeInvoke(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec)
	{
        (void)intf;
        (void)ec;
        if (ctx->role() == Role::Client && ctx->mode() == Mode::Implementatinon)
			return true;

		switch (ctx->role())
		{
		case Role::Server:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "_invoke_status _invoke(const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec)";
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness)
					<< "_invoke_status " << priv->getScope(object) << object->name()
					<< "::_invoke(const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec)";
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
				ctx->writeTabs(code_deepness) << "std::string name, variant;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				ctx->writeTabs(code_deepness+1) << "return _invoke_status::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "PIDL::JSONTools::getValue(*v, \"variant\", variant);" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.clear();" << std::endl;
				ctx->writeTabs(code_deepness) << "auto stat = _p->_callFunction(name, variant, *v, ret, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(ret, ret, \"object_data\", _data());" << std::endl;
				ctx->writeTabs(code_deepness) << "return stat;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;

				ctx->writeTabs(code_deepness) << "else if (_intf_p->_getValue(root, \"property_get\", rapidjson::kObjectType, v, ec))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string name;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.clear();" << std::endl;
				ctx->writeTabs(code_deepness) << "auto stat = _p->_callFunction(name, \"get\", *v, ret, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(ret, ret, \"object_data\", _data());" << std::endl;
				ctx->writeTabs(code_deepness) << "return stat;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;

				ctx->writeTabs(code_deepness) << "else if (_intf_p->_getValue(root, \"property_set\", rapidjson::kObjectType, v, ec))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string name;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf_p->_getValue(*v, \"name\", name, ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return _invoke_status::MarshallingError;" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.clear();" << std::endl;
				ctx->writeTabs(code_deepness) << "auto stat = _p->_callFunction(name, \"set\", *v, ret, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!ret.IsObject()) ret.SetObject();" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf_p->_addValue(ret, ret, \"object_data\", _data());" << std::endl;
				ctx->writeTabs(code_deepness) << "return stat;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "return _invoke_status::MarshallingError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				break;
			}

			break;
		case Role::Client:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writePrivateMembers(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec)
	{
        (void)intf;
        return priv->writePrivateMembers(code_deepness, ctx, object, ec);
	}

	bool JSON_STL_CodeGen::writePropertyGetterBody(Language::Interface * intf, Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
        (void)intf;
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
            if(helper()->logging())
            {
                auto starter = helper()->logging()->loggingStart("_p->_logger");
                if(starter.length())
                    ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                auto debug = helper()->logging()->loggingDebug("_p->_logger", "\"property getter: '"+std::string(property->name())+"'\"");
                if(debug.length())
                    ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
            }

            ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<_error_collector> _ec;" << std::endl;
			ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
			ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;

			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _doc, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _r, \"object_data\", _p->__data);" << std::endl;

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

			ctx->writeTabs(code_deepness) << "PIDL::JSONTools::getValue(_ret, \"object_data\", _p->__data);" << std::endl;

			if (!dynamic_cast<Language::Void*>(ret_type))
				ctx->writeTabs(code_deepness) << "return _retval;" << std::endl;
		}
		break;
		case Role::Server:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writePropertySetterBody(Language::Interface * intf, Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
        (void)intf;
        (void)ec;
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
            if(helper()->logging())
            {
                auto starter = helper()->logging()->loggingStart("_p->_logger");
                if(starter.length())
                    ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                auto debug = helper()->logging()->loggingDebug("_p->_logger", "\"property setter: '"+std::string(property->name())+"'\"");
                if(debug.length())
                    ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
            }

			ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<_error_collector> _ec;" << std::endl;
			ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
			ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;

			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _doc, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Value _r(rapidjson::kObjectType);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _r, \"object_data\", _p->_that->_data());" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"name\", \"" << property->name() << "\");" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf_p->_addValue(_doc, _v, \"value\", value);" << std::endl;
			ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _r, \"property_set\", _v);" << std::endl;
			ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _doc, \"object_call\", _r);" << std::endl;

			ctx->writeTabs(code_deepness) << "rapidjson::Document _ret;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf_p->_invokeCall(_doc, _ret, _ec))" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "_ec.throwException();" << std::endl;

			ctx->writeTabs(code_deepness) << "PIDL::JSONTools::getValue(_ret, \"object_data\", _p->__data);" << std::endl;
		}
		break;
		case Role::Server:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeConstructorBody(Language::Interface * intf, Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
        (void)intf;
        return priv->writeConstructorBody(object, code_deepness, ctx, ec);
	}

	bool JSON_STL_CodeGen::writeObjectBase(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
        (void)ec;
        if (priv->hasObjects(intf))
		{
			ctx->writeTabs(code_deepness) << "class _Object" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness - 1) << "public:" << std::endl;
            ctx->writeTabs(code_deepness) << "typedef ptr<_Object> Ptr;" << std::endl;
            ctx->writeTabs(code_deepness) << "virtual ~_Object() = default;" << std::endl;
			switch (ctx->role())
			{
			case Role::Client:
				break;
			case Role::Server:
				ctx->writeTabs(code_deepness) << "virtual _invoke_status _invoke(const rapidjson::Value & root, rapidjson::Document & ret, _error_collector & ec) = 0;" << std::endl;
				break;
			}
			ctx->writeTabs(code_deepness) << "virtual std::string _data() = 0;" << std::endl;
			ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
		}
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
				if (priv->hasObjects(intf))
				{
                    ctx->writeTabs(code_deepness) << "virtual ptr<_Object> _get_object(const std::string & object_data, _error_collector & ec) = 0;" << std::endl;
                    ctx->writeTabs(code_deepness) << "template<class Object_T> ptr<Object_T> _get_object(const std::string & object_data, _error_collector & ec)" << std::endl; //**
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "auto o = _get_object(object_data, ec);" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!o) return nullptr;" << std::endl;
					ctx->writeTabs(code_deepness) << "auto ret = std::dynamic_pointer_cast<Object_T, _Object>(o);" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!ret) ec.add(-1, \"unexpected: invalid object type for data '\" + object_data + \"'\");" << std::endl;
					ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl;
					ctx->writeTabs(code_deepness) << "virtual void _dispose_object(const std::string & object_data) = 0;" << std::endl;
				}
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
			if (priv->hasObjects(intf))
			{
				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					ctx->writeTabs(code_deepness) << "void _dispose_object(const std::string & object_data)";
					break;
				case Mode::Implementatinon:
					ctx->writeTabs(code_deepness) << "void " << priv->getScope(intf) << intf->name() << "::_dispose_object(const std::string & object_data)";
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

                    if(helper()->logging())
                    {
                        auto starter = helper()->logging()->loggingStart("_p->_logger");
                        if(starter.length())
                            ctx->writeTabs(code_deepness) << starter << ";" << std::endl;

                        auto debug = helper()->logging()->loggingDebug("_p->_logger", "\"embedded function: '_dispose_object'\"");
                        if(starter.length())
                            ctx->writeTabs(code_deepness) << debug << ";" << std::endl;
                    }

					ctx->writeTabs(code_deepness) << "PIDL::ExceptionErrorCollector<_error_collector> _ec;" << std::endl;
					ctx->writeTabs(code_deepness) << "rapidjson::Document _doc;" << std::endl;
					ctx->writeTabs(code_deepness) << "_doc.SetObject();" << std::endl;

					ctx->writeTabs(code_deepness) << "_p->_addValue(_doc, _doc, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

					ctx->writeTabs(code_deepness) << "rapidjson::Value _v(rapidjson::kObjectType);" << std::endl;
					ctx->writeTabs(code_deepness) << "_p->_addValue(_doc, _v, \"name\", \"_dispose_object\");" << std::endl;
					ctx->writeTabs(code_deepness) << "rapidjson::Value _aa(rapidjson::kObjectType);" << std::endl;
					ctx->writeTabs(code_deepness) << "_p->_addValue(_doc, _aa, \"object_data\", object_data);" << std::endl;
					ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _v, \"arguments\", _aa);" << std::endl;
					ctx->writeTabs(code_deepness) << "PIDL::JSONTools::addValue(_doc, _doc, \"function\", _v);" << std::endl;
					ctx->writeTabs(code_deepness) << "rapidjson::Document _ret;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_p->_invokeCall(_doc, _ret, _ec)) _ec.throwException();" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					break;
				}
			}
			break;
		case Role::Server:
			break;
		}
		return true;
	}

	bool JSON_STL_CodeGen::writePublicSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		if (!CPPCodeGen::writePublicSection(intf, code_deepness, ctx, obj, ec))
			return false;

		switch (ctx->role())
		{
		case Role::Client:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
				ctx->writeTabs(code_deepness) << "virtual std::string _data() override" << std::endl;
				ctx->writeTabs(code_deepness) << "{ return __data; }" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "virtual std::string _data() override;" << std::endl;
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << "std::string " << priv->getScope(obj) << obj->name() << "::_data()" << std::endl;
				ctx->writeTabs(code_deepness) << "{ return _priv->__data; }" << std::endl << std::endl;
				break;
			}
			break;
		case Role::Server:
			break;
		}

		return true;
	}

	bool JSON_STL_CodeGen::writeDestructorBody(Language::Interface * intf, Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Client:
			switch (ctx->mode())
			{
			case Mode::AllInOne:
				ctx->writeTabs(code_deepness) << "try { _intf->_dispose_object(_data()); } catch(...) { }" << std::endl;
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << "try { _intf->_dispose_object(_that->_data()); } catch(...) { }" << std::endl;
				break;
			case Mode::Declaration:
				break;
			}
			break;
		case Role::Server:
			break;
		}
		return CPPCodeGen::writeDestructorBody(intf, object, code_deepness, ctx, ec);
	}

}
