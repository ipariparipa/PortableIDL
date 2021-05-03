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

#include "include/pidlBackend/json_cscodegen.h"
#include "include/pidlBackend/language.h"
#include <pidlCore/errorcollector.h>

#include <functional>
#include <assert.h>

#define MAX_DOTNET_TUPLE_ARG_NUM 7

namespace PIDL
{

	struct JSON_CSCodeGen::Priv
	{
		struct Context : public CSCodeGenContext
		{
			Context(short tab_length, char tab_char, std::ostream & o, Role role) : CSCodeGenContext(tab_length, tab_char, o, role) { }
			virtual ~Context() = default;

			std::map<std::string /*interface_path*/, std::map<size_t /*hash*/, Language::Type*>> _prebuilt_types;

			static std::string to_interface_path(Language::Interface * intf)
			{
				std::string ret;
				for (auto & s : intf->scope())
					ret += s + ".";
				ret += intf->name();
				return ret;
			}

			const std::map<size_t /*hash*/, Language::Type*> & prebuilt_types(Language::Interface * intf) const
			{
				static const std::map<size_t /*hash*/, Language::Type*> empty;
				const auto & it = _prebuilt_types.find(to_interface_path(intf));
				return it != _prebuilt_types.end() ? it->second : empty;
			}

			static size_t /*hash*/ calculate_hash(Language::Type * t)
			{
				std::function<void(Language::Type * t, std::string & ret)> _calculateHash_str;
				_calculateHash_str = [&](Language::Type * t, std::string & ret)
				{
					if (ret.length())
						ret += ",";

					auto ft = t->finalType().get();
					if (dynamic_cast<Language::Generic*>(ft))
					{
						std::string _str;
						for (auto & t : dynamic_cast<Language::Generic*>(ft)->types())
							_calculateHash_str(t.get(), _str);
						ret += std::string(ft->name()) + "<" + _str + ">";
					}
					else if (dynamic_cast<Language::TypeDefinition*>(t) && dynamic_cast<Language::Structure*>(ft))
					{
						ret += t->name();
					}
					else if (dynamic_cast<Language::Structure*>(ft))
					{
						std::string _str;
						for (auto & m : dynamic_cast<Language::Structure*>(ft)->members())
							_calculateHash_str(m->type().get(), _str);
						ret += std::string(ft->name()) + "{" + _str + "}";
					}
					else
						ret += ft->name();
				};

				std::string hash_str;
				_calculateHash_str(t, hash_str);
				return std::hash<std::string>()(hash_str);
			}
				 
			void prebuild(Language::Interface * intf)
			{
				auto & _prebuilt_types = this->_prebuilt_types[to_interface_path(intf)];

				auto _addPrebuiltType = [&](Language::Type * t)
				{
					auto h = calculate_hash(t);
					if (!_prebuilt_types.count(h))
						_prebuilt_types[h] = t;
				};

				std::function<void(Language::Type * t)> _handleType;
				_handleType = [&](Language::Type * t)
				{
					auto ft = t->finalType().get();

					if (dynamic_cast<Language::Structure*>(ft))
					{
						_addPrebuiltType(ft);
						for (auto & m : dynamic_cast<Language::Structure*>(ft)->members())
							_handleType(m->type().get());
					}
					if (dynamic_cast<Language::TypeDefinition*>(t) && dynamic_cast<Language::Structure*>(ft))
					{
						_addPrebuiltType(t);
					}
					else if (dynamic_cast<Language::Generic*>(ft))
					{
						_addPrebuiltType(ft);
						for (auto & t : dynamic_cast<Language::Generic*>(ft)->types())
							_handleType(t.get());
					}
				};

				std::function<void(Language::DefinitionProvider * dp)> _handleDefinitions;
				_handleDefinitions = [&](Language::DefinitionProvider* dp)
				{
					for (auto & d : dp->definitions())
					{
						if (std::dynamic_pointer_cast<Language::TypeDefinition>(d))
						{
							_handleType(std::dynamic_pointer_cast<Language::TypeDefinition>(d)->type().get());
						}
						else if (std::dynamic_pointer_cast<Language::FunctionVariant>(d))
						{
							for (auto & a : std::dynamic_pointer_cast<Language::FunctionVariant>(d)->arguments())
								_handleType(a->type().get());
							_handleType(std::dynamic_pointer_cast<Language::FunctionVariant>(d)->returnType().get());
						}
						else if (std::dynamic_pointer_cast<Language::DefinitionProvider>(d))
						{
							_handleDefinitions(std::dynamic_pointer_cast<Language::DefinitionProvider>(d).get());
						}
					}
				};

				_handleDefinitions(intf);
			}

			virtual bool prebuild(Language::TopLevel *tl, ErrorCollector & ec) final override
			{
				bool has_error = false;
				if (dynamic_cast<Language::Module*>(tl))
				{
					auto m = dynamic_cast<Language::Module*>(tl);
					for (auto e : m->elements())
						if (!prebuild(dynamic_cast<Language::TopLevel*>(e.get()), ec)) 
							has_error = true;
				}
				else if (dynamic_cast<Language::Interface*>(tl))
					prebuild(dynamic_cast<Language::Interface*>(tl));
				return !has_error;
			}


			std::string mashaller_function_str(Language::Interface * intf, Language::Type * t, const std::string & func_name)
			{
				auto _t_hash = calculate_hash(t);
				return prebuilt_types(intf).count(_t_hash) ? func_name + "_" + std::to_string(_t_hash) : func_name;
			}

			std::string getValue_str(Language::Interface * intf, Language::Type * t)
			{
				return mashaller_function_str(intf, t, "_getValue");
			}

			std::string addValue_str(Language::Interface * intf, Language::Type * t)
			{
				return mashaller_function_str(intf, t, "_addValue");
			}

		};

		Priv(JSON_CSCodeGen * that_, const std::shared_ptr<CSCodeGenHelper> & helper_) : that(that_), helper(helper_)
		{ }

		JSON_CSCodeGen * that;
		std::shared_ptr<CSCodeGenHelper> helper;

		bool hasObjects(Language::Interface * intf)
		{
			for (auto & d : intf->definitions())
			{
				if (std::dynamic_pointer_cast<Language::Object>(d))
					return true;
			}
			return false;
		}

		template<class Class_T>
		bool writeMembers(Language::Interface * intf, short code_deepness, Context * ctx, Class_T * cl, ErrorCollector & ec)
		{
            (void)intf;
            (void)ec;
            switch (ctx->role())
			{
			case Role::Server:
				ctx->writeTabs(code_deepness) << "struct _FunctionRet { public _InvokeStatus status; public XElement ret; };" << std::endl;
				ctx->writeTabs(code_deepness) << "Dictionary<string, Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>>> _functions = new Dictionary<string, Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>>>();" << std::endl;
				ctx->writeTabs(code_deepness) << "_InvokeStatus _callFunction(string name, string variant, XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_functions.ContainsKey(name))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"function '\" + name + \"' is not found\"); ret = null; return _InvokeStatus.NotImplemented; }" << std::endl;
				ctx->writeTabs(code_deepness) << "var func = _functions[name];" << std::endl;
				ctx->writeTabs(code_deepness) << "if (string.IsNullOrEmpty(variant) && func.Count == 1)" << std::endl;
				ctx->writeTabs(code_deepness) << "{ var retval = func.First().Value(root, ec); ret = retval.ret; return retval.status; }" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!func.ContainsKey(variant))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"variant '\" +variant + \"' of function '\" + name + \"' is not found\"); ret = null; return _InvokeStatus.NotImplemented; }" << std::endl;
				ctx->writeTabs(code_deepness) << "var retval_ = func[variant](root, ec); ret = retval_.ret; return retval_.status;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "_InvokeStatus _callFunction(Func<object> func, PIDL.IPIDLErrorCollector ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "try" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "func();" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "catch (PIDL.PIDLException e)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "e.Get(ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus.Error;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "catch (Exception e)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.Add(-1, \"unhandled exception : '\" + e.ToString() + \"'\");" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus.FatalError;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus.Ok;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				break;
			case Role::Client:
				ctx->writeTabs(code_deepness) << "bool _invokeCall(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (dynamic_cast<Language::Interface*>(cl))
					ctx->writeTabs(code_deepness) << "var _intf = this;" << std::endl;
				ctx->writeTabs(code_deepness) << "var status = _intf._invoke(root, out ret, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "switch(status)" << std::endl;
				ctx->writeTabs(code_deepness) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus.Ok:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "break;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus.NotImplemented:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.Add((int)status, \"function is not implemented\");" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus.Error:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.Add((int)status, \"error while executing server function\");" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus.FatalError:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.Add((int)status, \"fatal error while executing server function\");" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "case _InvokeStatus.MarshallingError:" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ec.Add((int)status, \"error while marshalling of function call\");" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return false;" << std::endl;
				ctx->writeTabs(code_deepness) << "}" << std::endl << std::endl;
				ctx->writeTabs(code_deepness) << "return true;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			}

			return true;
		}

		template<class Class_T>
		bool writeConstructorBody(Language::Interface * intf, Class_T * cl, short code_deepness, Context * ctx, ErrorCollector & ec)
		{
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
						ctx->writeTabs(code_deepness) << "if (!_functions.ContainsKey(\"" << function->name() << "\"))" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "_functions.Add(\"" << function->name() << "\", new Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>>());" << std::endl;
						ctx->writeTabs(code_deepness++) << "_functions[\"" << function->name() << "\"][\"" << function->variantId() << "\"] = (root, ec) => {" << std::endl;
						if (!dynamic_cast<Language::MethodVariant* > (function))
							ctx->writeTabs(code_deepness) << "var _intf = this;" << std::endl;
						ctx->writeTabs(code_deepness) << "var ret = new _FunctionRet();" << std::endl;
						for (auto & a : function->arguments())
						{
							auto & o = ctx->writeTabs(code_deepness);
							if (!that->writeType(a->type().get(), code_deepness, ctx, ec))
								return false;
							o << " _arg_" << a->name() << " = default(";
							if (!that->writeType(a->type().get(), code_deepness, ctx, ec))
								return false;
							o << ");" << std::endl;
						}

						const auto & in_args = function->in_arguments();
						if (in_args.size())
						{
							ctx->writeTabs(code_deepness) << "XElement aa;" << std::endl;
							ctx->writeTabs(code_deepness) << "if (!_intf._getValue(root, \"arguments\", PIDL.JSONTools.Type.Object, out aa, ec))" << std::endl;
							ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;

							auto & o = ctx->writeTabs(code_deepness++) << "if (" << std::endl;
							bool is_first = true;
							for (auto & a : in_args)
							{
								ctx->writeTabs(code_deepness);
								if (!is_first)
									o << "| ";
								is_first = false;
								o << "!_intf." << ctx->getValue_str(intf, a->type().get()) << "(aa, \"" << a->name() << "\", out _arg_" << a->name() << ", ec)" << std::endl;
							}
							ctx->writeTabs(--code_deepness) << ")" << std::endl;
							ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;
						}

						auto ret_type = function->returnType().get();
						if (dynamic_cast<Language::Void*>(function->returnType().get()))
							ret_type = nullptr;
						if (ret_type)
						{
							ctx->writeTabs(code_deepness);
							if (!that->writeType(ret_type, code_deepness, ctx, ec))
								return false;
							*ctx << " retval = default(";
							if (!that->writeType(ret_type, code_deepness, ctx, ec))
								return false;
							*ctx << ");" << std::endl;
						}

						ctx->writeTabs(code_deepness) << "ret.status = _callFunction(() => { ";
						if (ret_type)
							*ctx << "retval = ";
						*ctx << function->name() << "(";
						bool is_first = true;
						for (auto & a : function->arguments())
						{
							if (!is_first)
								*ctx << ", ";
							is_first = false;
							switch (a->direction())
							{
							case Language::FunctionVariant::Argument::Direction::In:
								*ctx << "_arg_" << a->name();
								break;
							case Language::FunctionVariant::Argument::Direction::InOut:
								*ctx << "ref _arg_" << a->name();
								break;
							case Language::FunctionVariant::Argument::Direction::Out:
								*ctx << "out _arg_" << a->name();
								break;
							}
						}
						*ctx << "); return null; }, ec);" << std::endl;
						ctx->writeTabs(code_deepness) << "if (ret.status != _InvokeStatus.Ok)" << std::endl;
						ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.Error; return ret; }" << std::endl;

						ctx->writeTabs(code_deepness) << "ret.ret = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

						if (ret_type)
							ctx->writeTabs(code_deepness) << "_intf._addValue(ret.ret, \"retval\", retval);" << std::endl;

						const auto & out_args = function->out_arguments();
						if (out_args.size())
						{
							ctx->writeTabs(code_deepness) << "var out_v = PIDL.JSONTools.addValue(ret.ret, \"output\", PIDL.JSONTools.Type.Object);" << std::endl;

							for (auto & a : out_args)
								ctx->writeTabs(code_deepness) << "_intf." << ctx->addValue_str(intf, a->type().get()) << "(out_v, \"" << a->name() << "\", _arg_" << a->name() << ");" << std::endl;
						}

						ctx->writeTabs(code_deepness) << "ret.status = _InvokeStatus.Ok;" << std::endl;
						ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
						ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
					}
					else if (dynamic_cast<Language::Property*>(d.get()))
					{
					//getter
						auto property = dynamic_cast<Language::Property*>(d.get());
						ctx->writeTabs(code_deepness) << "_functions.Add(\"" << property->name() << "\", new Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>>());" << std::endl;
						ctx->writeTabs(code_deepness++) << "_functions[\"" << property->name() << "\"][\"get\"] = (root, ec) => {" << std::endl;
						ctx->writeTabs(code_deepness) << "var ret = new _FunctionRet();" << std::endl;

						auto ret_type = property->type().get();
						ctx->writeTabs(code_deepness);
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " retval = default(";
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << ");" << std::endl;

						ctx->writeTabs(code_deepness) << "ret.status = _callFunction(() => { ";
						*ctx << "retval = " << property->name() << "; return null; }, ec);" << std::endl;
						ctx->writeTabs(code_deepness) << "if (ret.status != _InvokeStatus.Ok)" << std::endl;
						ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.Error; return ret; }" << std::endl;

						ctx->writeTabs(code_deepness) << "ret.ret = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

						ctx->writeTabs(code_deepness) << "_intf." << ctx->addValue_str(intf, property->type().get()) << "(ret.ret, \"retval\", retval);" << std::endl;

						ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
						ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;

					//setter
						if (!property->readOnly())
						{
							ctx->writeTabs(code_deepness++) << "_functions[\"" << property->name() << "\"][\"set\"] = (root, ec) => {" << std::endl;
							ctx->writeTabs(code_deepness) << "var ret = new _FunctionRet();" << std::endl;
							auto & o = ctx->writeTabs(code_deepness);
							if (!that->writeType(property->type().get(), code_deepness, ctx, ec))
								return false;
							o << " value = default(";
							if (!that->writeType(property->type().get(), code_deepness, ctx, ec))
								return false;
							o << ");" << std::endl;
							ctx->writeTabs(code_deepness) << "if (!_intf." << ctx->getValue_str(intf, property->type().get()) << "(root, \"value\", out value, ec))" << std::endl;
							ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;

							ctx->writeTabs(code_deepness) << "ec.Clear();" << std::endl;
							ctx->writeTabs(code_deepness) << "ret.status = _callFunction(() => { " << property->name() << " = value; return null; }, ec);" << std::endl;
							ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
							ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
						}
					}
				}

				break;
			}
			return true;
		}

		bool writeFunctionBody(Language::Interface * intf, Language::FunctionVariant * function, short code_deepness, Context * ctx, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
				{
					ctx->writeTabs(code_deepness) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
					ctx->writeTabs(code_deepness) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

					ctx->writeTabs(code_deepness) << "PIDL.JSONTools.addValue(_root, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

					if (dynamic_cast<Language::MethodVariant*>(function))
					{
						ctx->writeTabs(code_deepness) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
						ctx->writeTabs(code_deepness) << "_intf._addValue(_r, \"object_data\", _data);" << std::endl;
						ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_r, \"method\", PIDL.JSONTools.Type.Object);" << std::endl;
					}
					else
					{
						ctx->writeTabs(code_deepness) << "var _intf = this;" << std::endl;
						ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_root, \"function\", PIDL.JSONTools.Type.Object);" << std::endl;
					}
					ctx->writeTabs(code_deepness) << "_intf._addValue(_v, \"name\", \"" << function->name() << "\");" << std::endl;
					ctx->writeTabs(code_deepness) << "_intf._addValue(_v, \"variant\", \"" << function->variantId() << "\");" << std::endl;

					auto in_args = function->in_arguments();
					if (in_args.size())
					{
						ctx->writeTabs(code_deepness) << "var _aa = PIDL.JSONTools.addValue(_v, \"arguments\", PIDL.JSONTools.Type.Object);" << std::endl;
						for (const auto & a : in_args)
							ctx->writeTabs(code_deepness) << "_intf." << ctx->addValue_str(intf, a->type().get()) << "(_aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;
					}

					ctx->writeTabs(code_deepness) << "XElement _ret;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_intf._invokeCall(_root, out _ret, _ec))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;

					auto ret_type = function->returnType().get();
					if (!dynamic_cast<Language::Void*>(ret_type))
					{
						ctx->writeTabs(code_deepness);
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " _retval;" << std::endl;
						ctx->writeTabs(code_deepness) << "if (!_intf._getValue(_ret, \"retval\", out _retval, _ec))" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;
					}

					const auto & out_args = function->out_arguments();
					if (out_args.size())
					{
						ctx->writeTabs(code_deepness) << "XElement _out_v;" << std::endl;
						ctx->writeTabs(code_deepness) << "if (!_intf._getValue(_ret, \"output\", PIDL.JSONTools.Type.Object, out _out_v, _ec))" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;

						bool has_error = false;
						for (auto & a : out_args)
						{
							auto & o = ctx->writeTabs(code_deepness) << a->name() << " = default(";
							if (!that->writeType(a->type().get(), code_deepness, ctx, ec))
								has_error = true;
							o << ");" << std::endl;
						}
						if (has_error)
							return false;

						ctx->writeTabs(code_deepness++) << "if (" << std::endl;

						bool is_first = true;
						for (auto & a : out_args)
						{
							auto & o = ctx->writeTabs(code_deepness);
							if (is_first)
							{
								o << "  ";
								is_first = false;
							}
							else
								o << "| ";
							o << "!_intf." << ctx->getValue_str(intf, a->type().get()) << "(_out_v, \"" << a->name() << "\", out " << a->name() << ", _ec)" << std::endl;
						}
						ctx->writeTabs(--code_deepness) << ") _ec.ThrowException();" << std::endl;
					}

					if (dynamic_cast<Language::MethodVariant*>(function))
					{
						ctx->writeTabs(code_deepness) << "string _tmp_object_data;" << std::endl;
						ctx->writeTabs(code_deepness) << "if (PIDL.JSONTools.getValue(_ret, \"object_data\", out _tmp_object_data)) _data = _tmp_object_data;" << std::endl;
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

		bool writePropertyGetterBody(Language::Interface * intf, Language::Property * prop, short code_deepness, Context * ctx, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
			{
				ctx->writeTabs(code_deepness) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
				ctx->writeTabs(code_deepness) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

				ctx->writeTabs(code_deepness) << "PIDL.JSONTools.addValue(_root, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

				ctx->writeTabs(code_deepness) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf._addValue(_r, \"object_data\", _data);" << std::endl;
				ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_r, \"property_get\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf._addValue(_v, \"name\", \"" << prop->name() << "\");" << std::endl;

				ctx->writeTabs(code_deepness) << "XElement _ret;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf._invokeCall(_root, out _ret, _ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;

				auto ret_type = prop->type().get();
				ctx->writeTabs(code_deepness);
				if (!that->writeType(ret_type, code_deepness, ctx, ec))
					return false;
				*ctx << " _retval;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf." << ctx->getValue_str(intf, ret_type) << "(_ret, \"retval\", out _retval, _ec)) _ec.ThrowException();" << std::endl;

				ctx->writeTabs(code_deepness) << "string _tmp_object_data;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (PIDL.JSONTools.getValue(_ret, \"object_data\", out _tmp_object_data)) _data = _tmp_object_data;" << std::endl;

				ctx->writeTabs(code_deepness) << "return _retval;" << std::endl;
			}
			break;
			case Role::Server:
				break;
			}

			return true;
		}

		bool writePropertySetterBody(Language::Interface * intf, Language::Property * prop, short code_deepness, Context * ctx, ErrorCollector & ec)
		{
            (void)ec;
            switch (ctx->role())
			{
			case Role::Client:
			{
				ctx->writeTabs(code_deepness) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
				ctx->writeTabs(code_deepness) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

				ctx->writeTabs(code_deepness) << "PIDL.JSONTools.addValue(_root, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

				ctx->writeTabs(code_deepness) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf._addValue(_r, \"object_data\", _data);" << std::endl;
				ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_r, \"property_set\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf._addValue(_v, \"name\", \"" << prop->name() << "\");" << std::endl;

				ctx->writeTabs(code_deepness) << "_intf." << ctx->addValue_str(intf, prop->type().get()) << "(_v, \"value\", value);" << std::endl;

				ctx->writeTabs(code_deepness) << "XElement _ret;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf._invokeCall(_root, out _ret, _ec)) _ec.ThrowException();" << std::endl;

				ctx->writeTabs(code_deepness) << "string _tmp_object_data;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (PIDL.JSONTools.getValue(_ret, \"object_data\", out _tmp_object_data)) _data = _tmp_object_data;" << std::endl;
			}
			break;
			case Role::Server:
				break;
			}

			return true;
		}

	};

	JSON_CSCodeGen::JSON_CSCodeGen(const std::shared_ptr<CSCodeGenHelper> & helper) :
		CSCodeGen(),
		priv(new Priv(this, helper))
	{ }

	JSON_CSCodeGen::JSON_CSCodeGen() :
		CSCodeGen(),
		priv(new Priv(this, std::make_shared<CSBasicCodeGenHelper>()))
	{ }

	JSON_CSCodeGen::~JSON_CSCodeGen()
	{
		delete priv;
	}

	CSCodeGenHelper * JSON_CSCodeGen::helper() const
	{
		return priv->helper.get();
	}

	CSCodeGenContext * JSON_CSCodeGen::createContext(short tab_length, char tab_char, std::ostream & o, Role role) const //final override
	{
		return new Priv::Context(tab_length, tab_char, o, role);
	}

	bool JSON_CSCodeGen::writeUsings(short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
        (void)ec;
        ctx->writeTabs(code_deepness) << "using System;" << std::endl;
		ctx->writeTabs(code_deepness) << "using System.Collections.Generic;" << std::endl;
		ctx->writeTabs(code_deepness) << "using System.Linq;" << std::endl;
		ctx->writeTabs(code_deepness) << "using System.Text;" << std::endl;
		ctx->writeTabs(code_deepness) << "using System.Xml.Linq;" << std::endl;

		return true;
	}

	bool JSON_CSCodeGen::writeMembers(short code_deepness, CSCodeGenContext * _ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		auto ctx = dynamic_cast<Priv::Context*>(_ctx);
		assert(ctx);

		if (!priv->writeMembers(intf, code_deepness, ctx, intf, ec))
			return false;

		ctx->stream() << std::endl;

		ctx->writeTabs(code_deepness) << "#region marshallers" << std::endl;

		for (auto & d : intf->definitions())
		{
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
			} 
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());
				switch (ctx->role())
				{
				case Role::Client:
					ctx->writeTabs(code_deepness) << object->name() << " _getValue_" << object->name() << "(XElement v, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "string object_data;" << std::endl;
					ctx->writeTabs(code_deepness) << "if(!_getValue(v, out object_data, ec))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "isOk = true;" << std::endl;
					ctx->writeTabs(code_deepness) << "return new " << object->name() << "(this, object_data);" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

					ctx->writeTabs(code_deepness) << object->name() << " _getValue_" << object->name() << "(XElement r, string name, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_getValue(r, name, PIDL.JSONTools.Type.String, out v, ec))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "return _getValue_" << object->name() << "(v, out isOk, ec);" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					break;
				case Role::Server:
					ctx->writeTabs(code_deepness) << object->name() << " _getValue_" << object->name() << "(XElement v, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "string object_data;" << std::endl;
					ctx->writeTabs(code_deepness) << "if(!_getValue(v, out object_data, ec))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "var o = _get_object(object_data, ec);" << std::endl;
					ctx->writeTabs(code_deepness) << "if (o == null)" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!(o is " << object->name() << "))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"unexpected: invalid object type for id '\" + object_data + \" '\"); isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "isOk = true; return o as " << object->name() << ";" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

					ctx->writeTabs(code_deepness) << object->name() << " _getValue_" << object->name() << "(XElement r, string name, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_getValue(r, name, out v, ec))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "if (PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Null))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = true; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "return _getValue_" << object->name() << "(v, out isOk, ec);" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					break;
				}
			}
		}

		ctx->writeTabs(code_deepness) << "bool _getValue(XElement r, string name, PIDL.JSONTools.Type type, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "if(!PIDL.JSONTools.getValue(r, name, type, out ret))" << std::endl;
		ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value '\" + name + \"' is not found or invalid\"); return false; }" << std::endl;
		ctx->writeTabs(code_deepness) << "return true;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;


		ctx->writeTabs(code_deepness) << "bool _getValue<T>(XElement r, string name, out T ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "bool isOk = false;" << std::endl;
		bool is_first = true;
		for (auto & d : intf->definitions())
		{
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
			}
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());
				if (is_first)
				{
					is_first = false;
					ctx->writeTabs(code_deepness);
				}
				else
					ctx->writeTabs(code_deepness) << "else ";
				*ctx << "if (typeof(T) == typeof(" << object->name() << "))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "ret = (T)(object)_getValue_" << object->name() << "(r, name, out isOk, ec);" << std::endl;
			}
		}
		if (!is_first)
		{
			ctx->writeTabs(code_deepness) << "else" << std::endl;
			ctx->writeTabs(code_deepness + 1);
		}
		else
			ctx->writeTabs(code_deepness);
		*ctx << "isOk = PIDL.JSONTools.getValue(r, name, out ret);" << std::endl;
		ctx->writeTabs(code_deepness) << "if (!isOk)" << std::endl;
		ctx->writeTabs(code_deepness + 1) << "ec.Add(-1, \"value '\" + name + \"' is not found or invalid\");" << std::endl;
		ctx->writeTabs(code_deepness) << "return isOk;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		ctx->writeTabs(code_deepness) << "bool _getValue<T>(XElement v, out T ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		for (auto & d : intf->definitions())
		{
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
			}
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());
				ctx->writeTabs(code_deepness) << "if (typeof(T) == typeof(" << object->name() << "))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "bool isOk;" << std::endl;
				ctx->writeTabs(code_deepness) << "ret = (T)(object)_getValue_" << object->name() << "(v, out isOk, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "return isOk;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
			}
		}

		ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue<T>(v, out ret))" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "ec.Add(-1, \"value is invalid\");" << std::endl;
		ctx->writeTabs(code_deepness) << "return false;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "return true;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		//getter for blob
		ctx->writeTabs(code_deepness) << "bool _getValue(XElement r, string name, out byte[] ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
		ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue(r, name, out v))" << std::endl;
		ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value '\" + name + \"' is not found\"); ret = null; return false; }" << std::endl;
		ctx->writeTabs(code_deepness) << "return _getValue(v, out ret, ec);" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		ctx->writeTabs(code_deepness) << "bool _getValue(XElement v, out byte[] ret, PIDL.IPIDLErrorCollector ec)";
		ctx->stream() << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "ret = null;" << std::endl;
		ctx->writeTabs(code_deepness) << "var t = PIDL.JSONTools.getType(v);" << std::endl;
		ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
		ctx->writeTabs(code_deepness) << "return true;" << std::endl;
		ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.None)" << std::endl;
		ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value is invalid\"); return false; }" << std::endl;
		ctx->writeTabs(code_deepness) << "string tmp;" << std::endl;
		ctx->writeTabs(code_deepness) << "if (!_getValue(v, out tmp, ec))" << std::endl;
		ctx->writeTabs(code_deepness) << "{ ret = null; return false; }" << std::endl;
		ctx->writeTabs(code_deepness) << "ret = Convert.FromBase64String(tmp);" << std::endl;
		ctx->writeTabs(code_deepness) << "return true;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		//getter for prebuilt types
		for (auto & ct : ctx->prebuilt_types(intf))
		{
			if (dynamic_cast<Language::Tuple*>(ct.second))
			{
				auto tt = dynamic_cast<Language::Tuple*>(ct.second);
				ctx->writeTabs(code_deepness) << "bool _getValue_" << ct.first << "(XElement r, string name, out ";
				if (!writeType(tt, code_deepness, ctx, ec))
					return false;
				*ctx << " ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue(r, name, out v))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value '\" + name + \"' is not found\"); ret = null; return false; }" << std::endl;
				ctx->writeTabs(code_deepness) << "return _getValue_" << ct.first << "(v, out ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "bool _getValue_" << ct.first << "(XElement v, out ";
				if (!writeType(tt, code_deepness, ctx, ec))
					return false;
				*ctx << " ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ret = null;" << std::endl;
				ctx->writeTabs(code_deepness) << "var t = PIDL.JSONTools.getType(v);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return true;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.None || t != PIDL.JSONTools.Type.Array)" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value is invalid\"); return false; }" << std::endl;
				size_t i = 0;
				for (auto & t : tt->types())
				{
					if (i > MAX_DOTNET_TUPLE_ARG_NUM)
					{
						ec << "more then 7 types of tuple cannot ba handled by .net";
						return false;
					}

					ctx->writeTabs(code_deepness);
					if (!writeType(t.get(), code_deepness, ctx, ec))
						return false;
					*ctx << " _t" << ++i << " = default(";
					if (!writeType(t.get(), code_deepness, ctx, ec))
						return false;
					*ctx << ");" << std::endl;
				}
				ctx->writeTabs(code_deepness) << "bool has_error = false;" << std::endl;
				ctx->writeTabs(code_deepness) << "int i = 0;" << std::endl;
				ctx->writeTabs(code_deepness) << "foreach(var e in v.Elements(\"item\"))" << std::endl;
				ctx->writeTabs(++code_deepness) << "switch (++i)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				i = 0;
				for (auto & t : tt->types())
				{
					ctx->writeTabs(code_deepness - 1) << "case " << ++i << ":" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!" << ctx->getValue_str(intf, t.get()) << "(e, out _t" << i << ", ec)) has_error = true;" << std::endl;
					ctx->writeTabs(code_deepness) << "break;" << std::endl;
				}
				ctx->writeTabs((--code_deepness)--) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "if (has_error) return false;" << std::endl;

				ctx->writeTabs(code_deepness) << "ret = new ";
				if (!writeType(tt, code_deepness, ctx, ec))
					return false;
				*ctx << "(";
				for (size_t i = 0, l = tt->types().size(); i < l; ++i)
				{
					if (i) *ctx << ", ";
					*ctx << "_t" << i + 1;
				}
				*ctx << ");" << std::endl;
				ctx->writeTabs(code_deepness) << "return true;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			}
			else if (dynamic_cast<Language::Nullable*>(ct.second))
			{
				auto nt = dynamic_cast<Language::Nullable*>(ct.second);
				ctx->writeTabs(code_deepness) << "bool _getValue_" << ct.first << "(XElement r, string name, out ";
				if (!writeType(nt, code_deepness, ctx, ec))
					return false;
				*ctx << " ret, PIDL.IPIDLErrorCollector ec)";
				ctx->stream() << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue(r, name, out v))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value '\" + name + \"' is not found\"); ret = null; return false; }" << std::endl;
				ctx->writeTabs(code_deepness) << "return _getValue_" << ct.first << "(v, out ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "bool _getValue_" << ct.first << "(XElement v, out ";
				if (!writeType(nt, code_deepness, ctx, ec))
					return false;
				*ctx << " ret, PIDL.IPIDLErrorCollector ec)";
				ctx->stream() << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ret = null;" << std::endl;
				ctx->writeTabs(code_deepness) << "var t = PIDL.JSONTools.getType(v);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.None)" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value is invalid\"); return false; }" << std::endl;
				ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return true;" << std::endl;
				ctx->writeTabs(code_deepness);
				auto t = nt->types().front().get();
				auto fn_type = t->finalType().get();
				if (dynamic_cast<Language::NativeType*>(fn_type) ||
					dynamic_cast<Language::DateTime*>(fn_type) ||
					dynamic_cast<Language::Structure*>(fn_type))
				{
					if (!writeType(nt->types().front().get(), code_deepness, ctx, ec))
						return false;
					*ctx << " tmp;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!" << ctx->getValue_str(intf, nt->types().front().get()) << "(v, out tmp, ec)) return false;" << std::endl;
					ctx->writeTabs(code_deepness) << "ret = new ";
					if (!writeType(nt, code_deepness, ctx, ec))
						return false;
					*ctx << "( tmp);" << std::endl;
					ctx->writeTabs(code_deepness) << "return true;" << std::endl;
				}
				else
					ctx->writeTabs(code_deepness) << "return " << ctx->getValue_str(intf, nt->types().front().get()) << "(v, out ret, ec);" << std::endl;

				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			}
			else if (dynamic_cast<Language::Array*>(ct.second))
			{
				auto at = dynamic_cast<Language::Array*>(ct.second);
				ctx->writeTabs(code_deepness) << "bool _getValue_" << ct.first << "(XElement r, string name, out ";
				if (!writeType(at, code_deepness, ctx, ec))
					return false;
				*ctx << " ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue(r, name, out v))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value '\" + name + \"' is not found\"); ret = null; return false; }" << std::endl;
				ctx->writeTabs(code_deepness) << "return _getValue_" << ct.first << "(v, out ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "bool _getValue_" << ct.first << "(XElement v, out ";
				if (!writeType(at, code_deepness, ctx, ec))
					return false;
				*ctx << " ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ret = null;" << std::endl;
				ctx->writeTabs(code_deepness) << "var t = PIDL.JSONTools.getType(v);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "return true;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.None || t != PIDL.JSONTools.Type.Array)" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value is invalid\"); return false; }" << std::endl;

				ctx->writeTabs(code_deepness) << "bool has_error = false;" << std::endl;
				ctx->writeTabs(code_deepness) << "int i = 0;" << std::endl;
				ctx->writeTabs(code_deepness) << "var elems = v.Elements(\"item\");" << std::endl;
				ctx->writeTabs(code_deepness) << "foreach(var e in elems) ++i;" << std::endl;
				ctx->writeTabs(code_deepness) << "ret = new ";
				if (!writeType(at->types().front().get(), code_deepness, ctx, ec))
					return false;
				*ctx << "[i];" << std::endl;
				ctx->writeTabs(code_deepness) << "i = 0;" << std::endl;
				ctx->writeTabs(code_deepness) << "foreach (var e in elems)" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "if (!" << ctx->getValue_str(intf, at->types().front().get()) << "(e, out ret[i++], ec)) has_error = true;" << std::endl;

				ctx->writeTabs(code_deepness) << "return !has_error;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			}
			else if (dynamic_cast<Language::TypeDefinition*>(ct.second))
			{
				auto td = dynamic_cast<Language::TypeDefinition*>(ct.second);
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					ctx->writeTabs(code_deepness) << "bool _getValue_" << ct.first << "(XElement r, string name, out ";
					if (!writeType(td, code_deepness, ctx, ec))
						return false;
					*ctx << " ret, PIDL.IPIDLErrorCollector ec)";
					ctx->stream() << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "ret = default(";
					if (!writeType(td, code_deepness, ctx, ec))
						return false;
					*ctx << ");" << std::endl;
					ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue(r, name, out v))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value '\" + name + \"' is not found\"); return false; }" << std::endl;
					ctx->writeTabs(code_deepness) << "return _getValue_" << ct.first << "(v, out ret, ec);" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

					ctx->writeTabs(code_deepness) << "bool _getValue_" << ct.first << "(XElement v, out ";
					if (!writeType(td, code_deepness, ctx, ec))
						return false;
					*ctx << " ret, PIDL.IPIDLErrorCollector ec)";
					ctx->stream() << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "ret = default(";
					if (!writeType(td, code_deepness, ctx, ec))
						return false;
					*ctx << ");" << std::endl;
					ctx->writeTabs(code_deepness) << "var t = PIDL.JSONTools.getType(v);" << std::endl;
					ctx->writeTabs(code_deepness) << "if (t != PIDL.JSONTools.Type.Object)" << std::endl;
					ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"value is invalid\"); return false; }" << std::endl;

					ctx->writeTabs(code_deepness) << "ret = new ";
					if (!writeType(td, code_deepness, ctx, ec))
						return false;
					*ctx << "();" << std::endl;
					bool is_first = true;
					for (auto & m : s->members())
					{
						if (is_first)
						{
							ctx->writeTabs(code_deepness) << "if (";
							is_first = false;
						}
						else
						{
							*ctx << " |" << std::endl;
							ctx->writeTabs(code_deepness) << "    ";
						}
						*ctx << "!" << ctx->getValue_str(intf, m->type().get()) << "(v, \"" << m->name() << "\", out ret." << m->name() << ", ec)";
					}
					if (!is_first)
					{
						*ctx << ")" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "return false;" << std::endl;
					}
					ctx->writeTabs(code_deepness) << "return true;" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				}
			}
		}

		ctx->writeTabs(code_deepness) << "void _addValue<T>(XElement r, string name, T val)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		is_first = true;
		for (auto & d : intf->definitions())
		{
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
			}
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());

				if (is_first)
				{
					ctx->writeTabs(code_deepness) << "if (typeof(T) == typeof(" << object->name() << "))" << std::endl;
					is_first = false;
				}
				else
					ctx->writeTabs(code_deepness) << "else if (typeof(T) == typeof(" << object->name() << "))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "_addValue(r, name, ((" << object->name() << ")(object)val)._data);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
			}
		}

		if (!is_first)
		{
			ctx->writeTabs(code_deepness) << "else" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "PIDL.JSONTools.addValue<T>(r, name, val);" << std::endl;
		}
		else
			ctx->writeTabs(code_deepness) << "PIDL.JSONTools.addValue<T>(r, name, val);" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		//setter for nullable
		ctx->writeTabs(code_deepness) << "void _addValue<T>(XElement r, string name, Nullable<T> val) where T : struct" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "if (val == null)" << std::endl;
		ctx->writeTabs(code_deepness + 1) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		ctx->writeTabs(code_deepness) << "else" << std::endl;
		ctx->writeTabs(code_deepness + 1) << "_addValue(r, name, val.Value);" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		//setter for array
		ctx->writeTabs(code_deepness) << "void _addValue<T>(XElement r, string name, T[] val)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "if (val == null)" << std::endl;
		ctx->writeTabs(code_deepness) << "{ PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null); return; }" << std::endl;
		ctx->writeTabs(code_deepness) << "if (typeof(T) == typeof(byte))" << std::endl;
		ctx->writeTabs(code_deepness) << "{ _addValue(r, name, Convert.ToBase64String((byte[])(object)val)); return; }" << std::endl;
		ctx->writeTabs(code_deepness) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Array);" << std::endl;
		ctx->writeTabs(code_deepness) << "foreach(var it in val)" << std::endl;
		ctx->writeTabs(code_deepness + 1) << "_addValue(v, \"item\", it);" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		//setters for prebuilt types
		for (auto & ct : ctx->prebuilt_types(intf))
		{
			if (dynamic_cast<Language::Tuple*>(ct.second))
			{
				auto tt = dynamic_cast<Language::Tuple*>(ct.second);
				ctx->writeTabs(code_deepness) << "void _addValue_" << ct.first << "(XElement r, string name, ";
				if (!writeType(tt, code_deepness, ctx, ec))
					return false;
				*ctx << " val)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "if (val == null)" << std::endl;
				ctx->writeTabs(code_deepness) << "{ PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null); return; }" << std::endl;
				ctx->writeTabs(code_deepness) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Array);" << std::endl;
				size_t i = 0;
				for (auto & t : tt->types())
					ctx->writeTabs(code_deepness) << ctx->addValue_str(intf, t.get()) << "(v, \"item\", val.Item" << ++i << ");" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			}
			else if (dynamic_cast<Language::Nullable*>(ct.second))
			{
				auto nt = dynamic_cast<Language::Nullable*>(ct.second);
				ctx->writeTabs(code_deepness) << "void _addValue_" << ct.first << "(XElement r, string name, ";
				if (!writeType(nt, code_deepness, ctx, ec))
					return false;
				*ctx << " val)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "if (val == null)" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
				ctx->writeTabs(code_deepness) << "else" << std::endl;
				ctx->writeTabs(code_deepness + 1) << ctx->addValue_str(intf, nt->types().front().get()) << "(r, name, val);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			}
			else if (dynamic_cast<Language::Array*>(ct.second))
			{
				auto at = dynamic_cast<Language::Array*>(ct.second);
				ctx->writeTabs(code_deepness) << "void _addValue_" << ct.first << "(XElement r, string name, ";
				if (!writeType(at, code_deepness, ctx, ec))
					return false;
				*ctx << " val)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "if (val == null)" << std::endl;
				ctx->writeTabs(code_deepness) << "{ PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null); return; }" << std::endl;
				ctx->writeTabs(code_deepness) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Array);" << std::endl;
				ctx->writeTabs(code_deepness) << "foreach(var it in val)" << std::endl;
				ctx->writeTabs(code_deepness + 1) << ctx->addValue_str(intf, at->types().front().get()) << "(v, \"item\", it);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			}
			else if (dynamic_cast<Language::TypeDefinition*>(ct.second))
			{
				auto td = dynamic_cast<Language::TypeDefinition*>(ct.second);
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					ctx->writeTabs(code_deepness) << "void _addValue_" << ct.first << "(XElement r, string name, ";
					if (!writeType(td, code_deepness, ctx, ec))
						return false;
					*ctx << " val)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Object);" << std::endl;
					for (auto & m : s->members())
						ctx->writeTabs(code_deepness) << ctx->addValue_str(intf, m->type().get()) << "(v, \"" << m->name() << "\", val." << m->name() << ");" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				}
			}
		}

		ctx->writeTabs(code_deepness) << "#endregion marshallers" << std::endl << std::endl;

		if (priv->hasObjects(intf))
		{
			switch (ctx->role())
			{
			case Role::Server:
				ctx->writeTabs(code_deepness) << "protected abstract _IObject _get_object(string object_data, PIDL.IPIDLErrorCollector ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "protected abstract void _dispose_object(string object_data);" << std::endl;
				break;
			case Role::Client:
				ctx->writeTabs(code_deepness) << "void _dispose_object(string object_data)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "" << std::endl;
				ctx->writeTabs(code_deepness) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
				ctx->writeTabs(code_deepness) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

				ctx->writeTabs(code_deepness) << "PIDL.JSONTools.addValue(_root, \"version\", " << PIDL_JSON_MARSHALLING_VERSION << ");" << std::endl;

				ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_root, \"function\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_addValue(_v, \"name\", \"_dispose_object\");" << std::endl;
				ctx->writeTabs(code_deepness) << "var _aa = PIDL.JSONTools.addValue(_v, \"arguments\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_addValue(_aa, \"object_data\", object_data);" << std::endl;
				ctx->writeTabs(code_deepness) << "XElement _ret;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_invokeCall(_root, out _ret, _ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			}
		}

		return true;
	}

	bool JSON_CSCodeGen::writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
        (void)ec;
        ctx->writeTabs(code_deepness) << "public enum _InvokeStatus {Ok, NotImplemented, Error, MarshallingError, NotSupportedMarshallingVersion, FatalError};" << std::endl;
		switch (ctx->role())
		{
		case Role::Server:
			ctx->writeTabs(code_deepness) << "public _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;

			ctx->writeTabs(code_deepness) << "int version;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue(root, \"version\", out version))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"could not detect mashalling version\"); ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "if (version != " << PIDL_JSON_MARSHALLING_VERSION << ")" << std::endl;
            ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"unsupported mashalling version detected\"); ret = null; return _InvokeStatus.NotSupportedMarshallingVersion; }" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "if (PIDL.JSONTools.getValue(root, \"function\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string name, variant;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_getValue(v, \"name\", out name, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl;
			ctx->writeTabs(code_deepness) << "PIDL.JSONTools.getValue(v, \"variant\", out variant);" << std::endl;
			ctx->writeTabs(code_deepness) << "ec.Clear();" << std::endl;
			ctx->writeTabs(code_deepness) << "return _callFunction(name, variant, v, out ret, ec); " << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			if (priv->hasObjects(intf))
			{
				ctx->writeTabs(code_deepness) << "else if (PIDL.JSONTools.getValue(root, \"object_call\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "string object_data;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_getValue(v, \"object_data\", out object_data, ec))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl;
				ctx->writeTabs(code_deepness) << "var obj = _get_object(object_data, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "if(obj == null)" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.Error; }" << std::endl;
				ctx->writeTabs(code_deepness) << "return obj._invoke(v, out ret, ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			}

			ctx->writeTabs(code_deepness) << "ret = null;" << std::endl;
			ctx->writeTabs(code_deepness) << "return _InvokeStatus.MarshallingError;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			
			break;
		case Role::Client:
			ctx->writeTabs(code_deepness) << "protected abstract _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec);" << std::endl << std::endl;

			break;
		}

		return true;
	}

	bool JSON_CSCodeGen::writeMembers(Language::Interface * intf, short code_deepness, CSCodeGenContext * _ctx, Language::Object * obj, ErrorCollector & ec)
	{
		auto ctx = dynamic_cast<Priv::Context*>(_ctx);
		assert(ctx);
		if (!priv->writeMembers(intf, code_deepness, ctx, obj, ec))
			return false;

		return true;
	}

	bool JSON_CSCodeGen::writeFunctionBody(Language::Interface * intf, Language::FunctionVariant * function, short code_deepness, CSCodeGenContext * _ctx, ErrorCollector & ec)
	{
		auto ctx = dynamic_cast<Priv::Context*>(_ctx);
		assert(ctx);
		return priv->writeFunctionBody(intf, function, code_deepness, ctx, ec);
	}

	bool JSON_CSCodeGen::writeConstructorBody(Language::Interface * intf, short code_deepness, CSCodeGenContext * _ctx, ErrorCollector & ec)
	{
		auto ctx = dynamic_cast<Priv::Context*>(_ctx);
		assert(ctx);
		if (!priv->writeConstructorBody(intf, intf, code_deepness, ctx, ec))
			return false;

		switch (ctx->role())
		{
		case Role::Client:
			break;
		case Role::Server:
			if (priv->hasObjects(intf))
			{
				ctx->writeTabs(code_deepness) << "_functions.Add(\"_dispose_object\", new Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>>());" << std::endl;
				ctx->writeTabs(code_deepness) << "_functions[\"_dispose_object\"][string.Empty] = (root, ec) =>" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "var ret = new _FunctionRet();" << std::endl;
				ctx->writeTabs(code_deepness) << "string _arg_object_data = default(string);" << std::endl;
				ctx->writeTabs(code_deepness) << "XElement aa;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_getValue(root, \"arguments\", PIDL.JSONTools.Type.Object, out aa, ec))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_getValue(aa, \"object_data\", out _arg_object_data, ec))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;
				ctx->writeTabs(code_deepness) << "var stat = _callFunction(() => { _dispose_object(_arg_object_data); return null; }, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "if (stat != _InvokeStatus.Ok)" << std::endl;
				ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.Error; return ret; }" << std::endl;
				ctx->writeTabs(code_deepness) << "ret.status = _InvokeStatus.Ok;" << std::endl;
				ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
				ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
			}
			break;
		}

		return true;
	}

	bool JSON_CSCodeGen::writeFunctionBody(Language::Interface * intf, Language::MethodVariant * function, short code_deepness, CSCodeGenContext * _ctx, ErrorCollector & ec)
	{
		auto ctx = dynamic_cast<Priv::Context*>(_ctx);
		assert(ctx);
		return priv->writeFunctionBody(intf, function, code_deepness, ctx, ec);
	}

	bool JSON_CSCodeGen::writeConstructorBody(Language::Interface * intf, Language::Object * obj, short code_deepness, CSCodeGenContext * _ctx, ErrorCollector & ec)
	{
		auto ctx = dynamic_cast<Priv::Context*>(_ctx);
		assert(ctx);
		return priv->writeConstructorBody(intf, obj, code_deepness, ctx, ec);
	}

	bool JSON_CSCodeGen::writeDestructorBody(Language::Interface * intf, Language::Object * obj, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
        (void)intf;
        (void)obj;
        (void)ec;
        switch (ctx->role())
		{
		case Role::Client:
			ctx->writeTabs(code_deepness) << "if (_intf != null)" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "_intf._dispose_object(_data);" << std::endl;
			break;
		case Role::Server:
			break;
		}
		return true;
	}

	bool JSON_CSCodeGen::writeInvoke(Language::Interface * intf, short code_deepness, CSCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
        (void)intf;
        (void)obj;
        (void)ec;
        switch (ctx->role())
		{
		case Role::Server:
			ctx->writeTabs(code_deepness) << "public _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;

			ctx->writeTabs(code_deepness) << "if (PIDL.JSONTools.getValue(root, \"method\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string name, variant;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "PIDL.JSONTools.getValue(v, \"variant\", out variant);" << std::endl;
			ctx->writeTabs(code_deepness) << "ec.Clear();" << std::endl;
			ctx->writeTabs(code_deepness) << "var stat = _callFunction(name, variant, v, out ret, ec);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf._addValue(ret, \"object_data\", _data);" << std::endl;
			ctx->writeTabs(code_deepness) << "return stat;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl;

			ctx->writeTabs(code_deepness) << "else if (PIDL.JSONTools.getValue(root, \"property_get\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string name;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "ec.Clear();" << std::endl;
			ctx->writeTabs(code_deepness) << "var stat = _callFunction(name, \"get\", v, out ret, ec);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf._addValue(ret, \"object_data\", _data);" << std::endl;
			ctx->writeTabs(code_deepness) << "return stat;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl;

			ctx->writeTabs(code_deepness) << "else if (PIDL.JSONTools.getValue(root, \"property_set\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string name;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "ec.Clear();" << std::endl;
			ctx->writeTabs(code_deepness) << "var stat = _callFunction(name, \"set\", v, out ret, ec);" << std::endl;
			ctx->writeTabs(code_deepness) << "if (ret == null) ret = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
			ctx->writeTabs(code_deepness) << "_intf._addValue(ret, \"object_data\", _data);" << std::endl;
			ctx->writeTabs(code_deepness) << "return stat;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl;

			ctx->writeTabs(code_deepness) << "ret = null;" << std::endl;
			ctx->writeTabs(code_deepness) << "return _InvokeStatus.MarshallingError;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			break;
		case Role::Client:
			break;
		}

		return true;
	}

	bool JSON_CSCodeGen::writePropertyGetterBody(Language::Interface * intf, Language::Property * prop, short code_deepness, CSCodeGenContext * _ctx, ErrorCollector & ec)
	{
		auto ctx = dynamic_cast<Priv::Context*>(_ctx);
		assert(ctx);
		return priv->writePropertyGetterBody(intf, prop, code_deepness, ctx, ec);
	}

	bool JSON_CSCodeGen::writePropertySetterBody(Language::Interface * intf, Language::Property * prop, short code_deepness, CSCodeGenContext * _ctx, ErrorCollector & ec)
	{
		auto ctx = dynamic_cast<Priv::Context*>(_ctx);
		assert(ctx);
		return priv->writePropertySetterBody(intf, prop, code_deepness, ctx, ec);
	}


}
