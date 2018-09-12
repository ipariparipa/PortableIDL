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

#include <assert.h>

namespace PIDL
{

	struct JSON_CSCodeGen::Priv
	{
		Priv(JSON_CSCodeGen * that_, const std::shared_ptr<CSCodeGenHelper> & helper_) : that(that_), helper(helper_)
		{ }

		JSON_CSCodeGen * that;
		std::shared_ptr<CSCodeGenHelper> helper;

		template<class Class_T>
		bool writeMembers(short code_deepness, CSCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Server:
				ctx->writeTabs(code_deepness) << "struct _FunctionRet { public _InvokeStatus status; public XElement ret; };" << std::endl;
				ctx->writeTabs(code_deepness) << "Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>> _functions = new Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>>();" << std::endl;
				ctx->writeTabs(code_deepness) << "_InvokeStatus _callFunction(string name, XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_functions.ContainsKey(name))" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "ec.Add(-1, \"function '\" + name + \"' is not found\");" << std::endl;
				ctx->writeTabs(code_deepness) << "ret = null;" << std::endl;
				ctx->writeTabs(code_deepness) << "return _InvokeStatus.NotImplemented;" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				ctx->writeTabs(code_deepness) << "var retval = _functions[name](root, ec);" << std::endl;
				ctx->writeTabs(code_deepness) << "ret = retval.ret;" << std::endl;
				ctx->writeTabs(code_deepness) << "return retval.status;" << std::endl;
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
		bool writeConstructorBody(Class_T * cl, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
		{

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
						ctx->writeTabs(code_deepness++) << "_functions[\"" << function->hash() << "\"] = (root, ec) => {" << std::endl;
						if (!dynamic_cast<Language::Method* > (function))
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
								o << "!_intf._getValue(aa, \"" << a->name() << "\", out _arg_" << a->name() << ", ec)" << std::endl;
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
							case Language::Function::Argument::Direction::In:
								*ctx << "_arg_" << a->name();
								break;
							case Language::Function::Argument::Direction::InOut:
								*ctx << "ref _arg_" << a->name();
								break;
							case Language::Function::Argument::Direction::Out:
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
								ctx->writeTabs(code_deepness) << "_intf._addValue(out_v, \"" << a->name() << "\", _arg_" << a->name() << ");" << std::endl;
						}

						ctx->writeTabs(code_deepness) << "ret.status = _InvokeStatus.Ok;" << std::endl;
						ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
						ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
					}
					else if (dynamic_cast<Language::Property*>(d.get()))
					{
					//getter
						auto property = dynamic_cast<Language::Property*>(d.get());
						ctx->writeTabs(code_deepness++) << "_functions[\"_get_" << property->name() << "\"] = (root, ec) => {" << std::endl;
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
						ctx->writeTabs(code_deepness) << "_intf._addValue(ret.ret, \"retval\", retval);" << std::endl;

						ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
						ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;

					//setter
						if (!property->readOnly())
						{
							ctx->writeTabs(code_deepness++) << "_functions[\"_set_" << property->name() << "\"] = (root, ec) => {" << std::endl;
							ctx->writeTabs(code_deepness) << "var ret = new _FunctionRet();" << std::endl;
							auto & o = ctx->writeTabs(code_deepness);
							if (!that->writeType(property->type().get(), code_deepness, ctx, ec))
								return false;
							o << " value = default(";
							if (!that->writeType(property->type().get(), code_deepness, ctx, ec))
								return false;
							o << ");" << std::endl;
							ctx->writeTabs(code_deepness) << "if (!_intf._getValue(root, \"value\", out value, ec))" << std::endl;
							ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;

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

		bool writeFunctionBody(Language::Function * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
				{
					ctx->writeTabs(code_deepness) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
					ctx->writeTabs(code_deepness) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
					if (dynamic_cast<Language::Method*>(function))
					{
						ctx->writeTabs(code_deepness) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
						ctx->writeTabs(code_deepness) << "_intf._addValue(_r, \"object_id\", _id);" << std::endl;
						ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_r, \"method\", PIDL.JSONTools.Type.Object);" << std::endl;
					}
					else
					{
						ctx->writeTabs(code_deepness) << "var _intf = this;" << std::endl;
						ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_root, \"function\", PIDL.JSONTools.Type.Object);" << std::endl;
					}
					ctx->writeTabs(code_deepness) << "_intf._addValue(_v, \"name\", \"" << function->hash() << "\");" << std::endl;

					auto in_args = function->in_arguments();
					if (in_args.size())
					{
						ctx->writeTabs(code_deepness) << "var _aa = PIDL.JSONTools.addValue(_v, \"arguments\", PIDL.JSONTools.Type.Object);" << std::endl;
						for (const auto & a : in_args)
							ctx->writeTabs(code_deepness) << "_intf._addValue(_aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;
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
							o << "!_getValue(_out_v, \"" << a->name() << "\", out " << a->name() << ", _ec)" << std::endl;
						}
						ctx->writeTabs(--code_deepness) << ")" << std::endl;
						ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;
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

		bool writePropertyGetterBody(Language::Property * prop, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
			{
				ctx->writeTabs(code_deepness) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
				ctx->writeTabs(code_deepness) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf._addValue(_r, \"object_id\", _id);" << std::endl;
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
				ctx->writeTabs(code_deepness) << "if (!_intf._getValue(_ret, \"retval\", out _retval, _ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;

				ctx->writeTabs(code_deepness) << "return _retval;" << std::endl;
			}
			break;
			case Role::Server:
				break;
			}

			return true;
		}

		bool writePropertySetterBody(Language::Property * prop, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
			{
				ctx->writeTabs(code_deepness) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
				ctx->writeTabs(code_deepness) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf._addValue(_r, \"object_id\", _id);" << std::endl;
				ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_r, \"property_set\", PIDL.JSONTools.Type.Object);" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf._addValue(_v, \"name\", \"" << prop->name() << "\");" << std::endl;

				ctx->writeTabs(code_deepness) << "_intf._addValue(_v, \"value\", value);" << std::endl;

				ctx->writeTabs(code_deepness) << "XElement _ret;" << std::endl;
				ctx->writeTabs(code_deepness) << "if (!_intf._invokeCall(_root, out _ret, _ec))" << std::endl;
				ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;
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

	bool JSON_CSCodeGen::writeUsings(short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		ctx->writeTabs(code_deepness) << "using System;" << std::endl;
		ctx->writeTabs(code_deepness) << "using System.Collections.Generic;" << std::endl;
		ctx->writeTabs(code_deepness) << "using System.Linq;" << std::endl;
		ctx->writeTabs(code_deepness) << "using System.Text;" << std::endl;
		ctx->writeTabs(code_deepness) << "using System.Xml.Linq;" << std::endl;

		return true;
	}

	bool JSON_CSCodeGen::writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		if (!priv->writeMembers(code_deepness, ctx, intf, ec))
			return false;

		ctx->writeTabs(code_deepness) << "#region marshallers" << std::endl;

		for (auto & d : intf->definitions())
		{
			bool is_first;
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
				auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
				auto s = dynamic_cast<Language::Structure*>(td->finalType().get());
				if (s)
				{
					ctx->writeTabs(code_deepness) << td->name() << " _getValue_" << td->name() << "(XElement v, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "var ret = new " << td->name() << "();" << std::endl;
					ctx->writeTabs(code_deepness) << "if(!PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "ec.Add(-1, \"value of '" << td->name() << "'is not object\");" << std::endl;
					ctx->writeTabs(code_deepness) << "isOk = false; return ret;" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl;

					auto & members = s->members();
					if (members.size())
					{
						ctx->writeTabs(code_deepness) << "isOk =" << std::endl;
						is_first = true;
						for (auto & m : members)
						{
							if (is_first)
							{
								is_first = false;
								ctx->writeTabs(code_deepness + 1) << "  ";
							}
							else
								ctx->writeTabs(code_deepness + 1) << "& ";
							*ctx << "_getValue(v, \"" << m->name() << "\", out ret." << m->name() << ", ec)" << std::endl;
						}
						ctx->writeTabs(code_deepness + 1) << ";" << std::endl;
					}
					ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

					ctx->writeTabs(code_deepness) << td->name() << " _getValue_" << td->name() << "(XElement r, string name, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "var ret = new " << td->name() << "();" << std::endl;
					ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!_getValue(r, name, PIDL.JSONTools.Type.Object, out v, ec))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return ret; }" << std::endl;
					ctx->writeTabs(code_deepness) << "return _getValue_" << td->name() << "(v, out isOk, ec);" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				}
			}
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());
				switch (ctx->role())
				{
				case Role::Client:
					ctx->writeTabs(code_deepness) << object->name() << " _getValue_" << object->name() << "(XElement v, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "string object_id;" << std::endl;
					ctx->writeTabs(code_deepness) << "if(!_getValue(v, out object_id, ec))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "isOk = true;" << std::endl;
					ctx->writeTabs(code_deepness) << "return new " << object->name() << "(this, object_id);" << std::endl;
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
					ctx->writeTabs(code_deepness) << "string object_id;" << std::endl;
					ctx->writeTabs(code_deepness) << "if(!_getValue(v, out object_id, ec))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "var o = _get_object(object_id, ec);" << std::endl;
					ctx->writeTabs(code_deepness) << "if (o == null)" << std::endl;
					ctx->writeTabs(code_deepness) << "{ isOk = false; return null; }" << std::endl;
					ctx->writeTabs(code_deepness) << "if (!(o is " << object->name() << "))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ ec.Add(-1, \"unexpected: invalid object type for id '\" + object_id + \" '\"); isOk = false; return null; }" << std::endl;
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
		ctx->writeTabs(--code_deepness) << "}" << std::endl;


		ctx->writeTabs(code_deepness) << "bool _getValue<T>(XElement r, string name, out T ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "bool isOk = false;" << std::endl;
		bool is_first = true;
		for (auto & d : intf->definitions())
		{
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
				auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					if (is_first)
					{
						is_first = false;
						ctx->writeTabs(code_deepness);
					}
					else
						ctx->writeTabs(code_deepness) << "else ";
					*ctx << "if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
					ctx->writeTabs(code_deepness + 1) << "ret = (T)(object)_getValue_" << td->name() << "(r, name, out isOk, ec);" << std::endl;
				}
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
				auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					ctx->writeTabs(code_deepness) << "if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "bool isOk;" << std::endl;
					ctx->writeTabs(code_deepness) << "ret = (T)(object)_getValue_" << td->name() << "(v, out isOk, ec);" << std::endl;
					ctx->writeTabs(code_deepness) << "return isOk;" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl;
				}
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

		ctx->writeTabs(code_deepness) << "bool _getValue<T>(XElement r, string name, out Nullable<T> ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		ctx->writeTabs(code_deepness + 1) << "where T : struct" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
		ctx->writeTabs(code_deepness) << "PIDL.JSONTools.Type t;" << std::endl;
		ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue(r, name, out v) || (t = PIDL.JSONTools.getType(v)) == PIDL.JSONTools.Type.None)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "ec.Add(-1, \"value '\" + name + \"' is not found or invalid\");" << std::endl;
		ctx->writeTabs(code_deepness) << "ret = null; return false;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
		ctx->writeTabs(code_deepness) << "{ ret = null; return true; }" << std::endl;
		ctx->writeTabs(code_deepness) << "T _ret;" << std::endl;
		ctx->writeTabs(code_deepness) << "if (!_getValue(v, out _ret, ec))" << std::endl;
		ctx->writeTabs(code_deepness) << "{ ret = null; return false; }" << std::endl;
		ctx->writeTabs(code_deepness) << "ret = _ret; return true;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		ctx->writeTabs(code_deepness) << "bool _getValue<T>(XElement r, string name, out T[] ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		ctx->writeTabs(code_deepness + 1) << "where T : struct" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;
		ctx->writeTabs(code_deepness) << "PIDL.JSONTools.Type t;" << std::endl;
		ctx->writeTabs(code_deepness) << "if (!PIDL.JSONTools.getValue(r, name, out v) || (t = PIDL.JSONTools.getType(v)) == PIDL.JSONTools.Type.None)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "ec.Add(-1, \"value '\" + name + \"' is not found or invalid\");" << std::endl;
		ctx->writeTabs(code_deepness) << "ret = null; return false;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
		ctx->writeTabs(code_deepness) << "{ ret = null; return true; }" << std::endl;
		ctx->writeTabs(code_deepness) << "if (typeof(T) == typeof(byte))" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "string tmp;" << std::endl;
		ctx->writeTabs(code_deepness) << "if (!_getValue(v, out tmp, ec))" << std::endl;
		ctx->writeTabs(code_deepness) << "{ ret = null; return false; }" << std::endl;
		ctx->writeTabs(code_deepness) << "ret = (T[])(object)Convert.FromBase64String(tmp);" << std::endl;
		ctx->writeTabs(code_deepness) << "return true;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "if (t != PIDL.JSONTools.Type.Array)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "ec.Add(-1, \"value '\" + name + \"' is not array\");" << std::endl;
		ctx->writeTabs(code_deepness) << "ret = null; return false;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "var elems = v.Elements(\"item\");" << std::endl;
		ctx->writeTabs(code_deepness) << "int i = 0;" << std::endl;
		ctx->writeTabs(code_deepness) << "foreach(var e in elems)" << std::endl;
		ctx->writeTabs(code_deepness) << "++i;" << std::endl;
		ctx->writeTabs(code_deepness) << "ret = new T[i];" << std::endl;
		ctx->writeTabs(code_deepness) << "i = 0;" << std::endl;
		ctx->writeTabs(code_deepness) << "bool isOk = true;" << std::endl;
		ctx->writeTabs(code_deepness) << "foreach(var e in elems)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "if (!_getValue(e, out ret[i++], ec))" << std::endl;
		ctx->writeTabs(code_deepness + 1) << "isOk = false;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "return isOk;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		ctx->writeTabs(code_deepness) << "void _addValue<T>(XElement r, string name, T val)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		is_first = true;
		for (auto & d : intf->definitions())
		{
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
				auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					if (is_first)
					{
						ctx->writeTabs(code_deepness) << "if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
						is_first = false;
					}
					else
						ctx->writeTabs(code_deepness) << "else if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					ctx->writeTabs(code_deepness) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Object);" << std::endl;
					for (auto & m : s->members())
						ctx->writeTabs(code_deepness) << "_addValue(v, \"" << m->name() << "\", ((" << td->name() << ")(object)val)." << m->name() << ");" << std::endl;
					ctx->writeTabs(--code_deepness) << "}" << std::endl;
				}
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
				ctx->writeTabs(code_deepness) << "_addValue(r, name, ((" << object->name() << ")(object)val)._id);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
			}
		}

		if (!is_first)
		{
			ctx->writeTabs(code_deepness) << "else" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "PIDL.JSONTools.addValue<T>(r, name, val);" << std::endl;
		}
		ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

		ctx->writeTabs(code_deepness) << "void _addValue<T>(XElement r, string name, Nullable<T> val)" << std::endl;
		ctx->writeTabs(code_deepness + 1) << "where T : struct //, IComparable" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "if (val == null)" << std::endl;
		ctx->writeTabs(code_deepness) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		ctx->writeTabs(code_deepness) << "else" << std::endl;
		ctx->writeTabs(code_deepness) << "_addValue(r, name, val.Value);" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;

		ctx->writeTabs(code_deepness) << "void _addValue<T>(XElement r, string name, T[] val)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "if (val == null)" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		ctx->writeTabs(code_deepness) << "return;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "if (typeof(T) == typeof(byte))" << std::endl;
		ctx->writeTabs(code_deepness++) << "{" << std::endl;
		ctx->writeTabs(code_deepness) << "_addValue(r, name, Convert.ToBase64String((byte[])(object)val));" << std::endl;
		ctx->writeTabs(code_deepness) << "return;" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Array);" << std::endl;
		ctx->writeTabs(code_deepness) << "foreach(var it in val)" << std::endl;
		ctx->writeTabs(code_deepness) << "_addValue(v, \"item\", it);" << std::endl;
		ctx->writeTabs(--code_deepness) << "}" << std::endl;
		ctx->writeTabs(code_deepness) << "#endregion marshallers" << std::endl << std::endl;

		switch (ctx->role())
		{
		case Role::Server:
			ctx->writeTabs(code_deepness) << "protected abstract _IObject _get_object(string object_id, PIDL.IPIDLErrorCollector ec);" << std::endl;
			ctx->writeTabs(code_deepness) << "protected abstract void _dispose_object(string object_id);" << std::endl;
			break;
		case Role::Client:
			ctx->writeTabs(code_deepness) << "public void _dispose_object(string object_id)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "" << std::endl;
			ctx->writeTabs(code_deepness) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
			ctx->writeTabs(code_deepness) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
			ctx->writeTabs(code_deepness) << "var _v = PIDL.JSONTools.addValue(_root, \"function\", PIDL.JSONTools.Type.Object);" << std::endl;
			ctx->writeTabs(code_deepness) << "_addValue(_v, \"name\", \"_dispose_object\");" << std::endl;
			ctx->writeTabs(code_deepness) << "var _aa = PIDL.JSONTools.addValue(_v, \"arguments\", PIDL.JSONTools.Type.Object);" << std::endl;
			ctx->writeTabs(code_deepness) << "_addValue(_aa, \"object_id\", object_id);" << std::endl;
			ctx->writeTabs(code_deepness) << "XElement _ret;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_invokeCall(_root, out _ret, _ec))" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "_ec.ThrowException();" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
		}

		return true;
	}

	bool JSON_CSCodeGen::writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Server:
			ctx->writeTabs(code_deepness) << "public enum _InvokeStatus {Ok, NotImplemented, Error, MarshallingError, FatalError};" << std::endl;
			ctx->writeTabs(code_deepness) << "public _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;

			ctx->writeTabs(code_deepness) << "if (PIDL.JSONTools.getValue(root, \"function\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string name;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_getValue(v, \"name\", out name, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl;
			ctx->writeTabs(code_deepness) << "return _callFunction(name, v, out ret, ec);" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "else if (PIDL.JSONTools.getValue(root, \"object_call\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string object_id;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_getValue(v, \"object_id\", out object_id, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl;
			ctx->writeTabs(code_deepness) << "var obj = _get_object(object_id, ec);" << std::endl;
			ctx->writeTabs(code_deepness) << "if(obj == null)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.Error; }" << std::endl;
			ctx->writeTabs(code_deepness) << "return obj._invoke(v, out ret, ec);" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;



			ctx->writeTabs(code_deepness) << "ret = null;" << std::endl;
			ctx->writeTabs(code_deepness) << "return _InvokeStatus.MarshallingError;" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			
			break;
		case Role::Client:
			ctx->writeTabs(code_deepness) << "public enum _InvokeStatus {Ok, NotImplemented, Error, MarshallingError, FatalError};" << std::endl;
			ctx->writeTabs(code_deepness) << "protected abstract _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec);" << std::endl << std::endl;

			break;
		}

		return true;
	}

	bool JSON_CSCodeGen::writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		if (!priv->writeMembers(code_deepness, ctx, obj, ec))
			return false;

		return true;
	}

	bool JSON_CSCodeGen::writeFunctionBody(Language::Function * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writeFunctionBody(function, code_deepness, ctx, ec);
	}

	bool JSON_CSCodeGen::writeConstructorBody(Language::Interface * intf, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		if (!priv->writeConstructorBody(intf, code_deepness, ctx, ec))
			return false;

		switch (ctx->role())
		{
		case Role::Client:
			break;
		case Role::Server:
			ctx->writeTabs(code_deepness) << "_functions[\"_dispose_object\"] = (root, ec) =>" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "var ret = new _FunctionRet();" << std::endl;
			ctx->writeTabs(code_deepness) << "string _arg_object_id = default(string);" << std::endl;
			ctx->writeTabs(code_deepness) << "XElement aa;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_getValue(root, \"arguments\", PIDL.JSONTools.Type.Object, out aa, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_getValue(aa, \"object_id\", out _arg_object_id, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;
			ctx->writeTabs(code_deepness) << "var stat = _callFunction(() => { _dispose_object(_arg_object_id); return null; }, ec);" << std::endl;
			ctx->writeTabs(code_deepness) << "if (stat != _InvokeStatus.Ok)" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret.status = _InvokeStatus.Error; return ret; }" << std::endl;
			ctx->writeTabs(code_deepness) << "ret.status = _InvokeStatus.Ok;" << std::endl;
			ctx->writeTabs(code_deepness) << "return ret;" << std::endl;
			ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
			break;
		}

		return true;
	}

	bool JSON_CSCodeGen::writeFunctionBody(Language::Method * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writeFunctionBody(function, code_deepness, ctx, ec);
	}

	bool JSON_CSCodeGen::writeConstructorBody(Language::Object * obj, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writeConstructorBody(obj, code_deepness, ctx, ec);
	}

	bool JSON_CSCodeGen::writeDestructorBody(Language::Object * obj, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Client:
			ctx->writeTabs(code_deepness) << "if (_intf != null)" << std::endl;
			ctx->writeTabs(code_deepness + 1) << "_intf._dispose_object(_id);" << std::endl;
			break;
		case Role::Server:
			break;
		}
		return true;
	}

	bool JSON_CSCodeGen::writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Server:
			ctx->writeTabs(code_deepness) << "public _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "XElement v;" << std::endl;

			ctx->writeTabs(code_deepness) << "if (PIDL.JSONTools.getValue(root, \"method\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string name;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "return _callFunction(name, v, out ret, ec);" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl;

			ctx->writeTabs(code_deepness) << "else if (PIDL.JSONTools.getValue(root, \"property_get\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string name;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "return _callFunction(\"_get_\"+name, v, out ret, ec);" << std::endl;
			ctx->writeTabs(--code_deepness) << "}" << std::endl;

			ctx->writeTabs(code_deepness) << "else if (PIDL.JSONTools.getValue(root, \"property_set\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			ctx->writeTabs(code_deepness) << "string name;" << std::endl;
			ctx->writeTabs(code_deepness) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			ctx->writeTabs(code_deepness) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << "return _callFunction(\"_set_\"+name, v, out ret, ec);" << std::endl;
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

	bool JSON_CSCodeGen::writePropertyGetterBody(Language::Property * prop, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writePropertyGetterBody(prop, code_deepness, ctx, ec);
	}

	bool JSON_CSCodeGen::writePropertySetterBody(Language::Property * prop, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writePropertySetterBody(prop, code_deepness, ctx, ec);
	}


}
