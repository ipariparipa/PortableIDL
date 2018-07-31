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

		std::ostream & writeTabs(short code_deepness, CSCodeGenContext * ctx) const
		{
			char ch;
			for (short i = 0, l = code_deepness * helper->tabDefinition(ch); i < l; ++i)
				*ctx << ch;
			return ctx->stream();
		}

		template<class Class_T>
		bool writeMembers(short code_deepness, CSCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Server:
				writeTabs(code_deepness, ctx) << "struct _FunctionRet { public _InvokeStatus status; public XElement ret; };" << std::endl;
				writeTabs(code_deepness, ctx) << "Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>> _functions = new Dictionary<string, Func<XElement, PIDL.IPIDLErrorCollector, _FunctionRet>>();" << std::endl;
				writeTabs(code_deepness, ctx) << "_InvokeStatus _callFunction(string name, XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "if (!_functions.ContainsKey(name))" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "ec.Add(-1, \"function '\" + name + \"' is not found\");" << std::endl;
				writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus.NotImplemented;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "var retval = _functions[name](root, ec);" << std::endl;
				writeTabs(code_deepness, ctx) << "ret = retval.ret;" << std::endl;
				writeTabs(code_deepness, ctx) << "return retval.status;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

				writeTabs(code_deepness, ctx) << "_InvokeStatus _callFunction(Func<object> func, PIDL.IPIDLErrorCollector ec)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "try" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "func();" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "catch (PIDL.PIDLException e)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "e.Get(ec);" << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus.Error;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "catch (Exception e)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "ec.Add(-1, \"unhandled exception : '\" + e.ToString() + \"'\");" << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus.FatalError;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				writeTabs(code_deepness, ctx) << "return _InvokeStatus.Ok;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl;
				break;
			case Role::Client:
				writeTabs(code_deepness, ctx) << "bool _invokeCall(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (dynamic_cast<Language::Interface*>(cl))
					writeTabs(code_deepness, ctx) << "var _intf = this;" << std::endl;
				writeTabs(code_deepness, ctx) << "var status = _intf._invoke(root, out ret, ec);" << std::endl;
				writeTabs(code_deepness, ctx) << "switch(status)" << std::endl;
				writeTabs(code_deepness, ctx) << "{" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus.Ok:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "break;" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus.NotImplemented:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "ec.Add((int)status, \"function is not implemented\");" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus.Error:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "ec.Add((int)status, \"error while executing server function\");" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus.FatalError:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "ec.Add((int)status, \"fatal error while executing server function\");" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
				writeTabs(code_deepness, ctx) << "case _InvokeStatus.MarshallingError:" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "ec.Add((int)status, \"error while marshalling of function call\");" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
				writeTabs(code_deepness, ctx) << "}" << std::endl << std::endl;
				writeTabs(code_deepness, ctx) << "return true;" << std::endl;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
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
						writeTabs(code_deepness++, ctx) << "_functions[\"" << function->name() << "\"] = (root, ec) => {" << std::endl;
						if (!dynamic_cast<Language::Method* > (function))
							writeTabs(code_deepness, ctx) << "var _intf = this;" << std::endl;
						writeTabs(code_deepness, ctx) << "var ret = new _FunctionRet();" << std::endl;
						for (auto & a : function->arguments())
						{
							auto & o = writeTabs(code_deepness, ctx);
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
							writeTabs(code_deepness, ctx) << "XElement aa;" << std::endl;
							writeTabs(code_deepness, ctx) << "if (!_intf._getValue(root, \"arguments\", PIDL.JSONTools.Type.Object, out aa, ec))" << std::endl;
							writeTabs(code_deepness, ctx) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;

							auto & o = writeTabs(code_deepness++, ctx) << "if (" << std::endl;
							bool is_first = true;
							for (auto & a : in_args)
							{
								writeTabs(code_deepness, ctx);
								if (!is_first)
									o << "| ";
								is_first = false;
								o << "!_intf._getValue(aa, \"" << a->name() << "\", out _arg_" << a->name() << ", ec)" << std::endl;
							}
							writeTabs(--code_deepness, ctx) << ")" << std::endl;
							writeTabs(code_deepness, ctx) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;
						}

						auto ret_type = function->returnType().get();
						if (dynamic_cast<Language::Void*>(function->returnType().get()))
							ret_type = nullptr;
						if (ret_type)
						{
							writeTabs(code_deepness, ctx);
							if (!that->writeType(ret_type, code_deepness, ctx, ec))
								return false;
							*ctx << " retval = default(";
							if (!that->writeType(ret_type, code_deepness, ctx, ec))
								return false;
							*ctx << ");" << std::endl;
						}

						writeTabs(code_deepness, ctx) << "ret.status = _callFunction(() => { ";
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
						writeTabs(code_deepness, ctx) << "if (ret.status != _InvokeStatus.Ok)" << std::endl;
						writeTabs(code_deepness, ctx) << "{ ret.status = _InvokeStatus.Error; return ret; }" << std::endl;

						writeTabs(code_deepness, ctx) << "ret.ret = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

						if (ret_type)
							writeTabs(code_deepness, ctx) << "_intf._addValue(ret.ret, \"retval\", retval);" << std::endl;

						const auto & out_args = function->out_arguments();
						if (out_args.size())
						{
							writeTabs(code_deepness, ctx) << "var out_v = PIDL.JSONTools.addValue(ret.ret, \"output\", PIDL.JSONTools.Type.Object);" << std::endl;

							for (auto & a : out_args)
								writeTabs(code_deepness, ctx) << "_intf._addValue(out_v, \"" << a->name() << "\", _arg_" << a->name() << ");" << std::endl;
						}

						writeTabs(code_deepness, ctx) << "ret.status = _InvokeStatus.Ok;" << std::endl;
						writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
						writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
					}
					else if (dynamic_cast<Language::Property*>(d.get()))
					{
					//getter
						auto property = dynamic_cast<Language::Property*>(d.get());
						writeTabs(code_deepness++, ctx) << "_functions[\"_get_" << property->name() << "\"] = (root, ec) => {" << std::endl;
						writeTabs(code_deepness, ctx) << "var ret = new _FunctionRet();" << std::endl;

						auto ret_type = property->type().get();
						writeTabs(code_deepness, ctx);
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " retval = default(";
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << ");" << std::endl;

						writeTabs(code_deepness, ctx) << "ret.status = _callFunction(() => { ";
						*ctx << "retval = " << property->name() << "; return null; }, ec);" << std::endl;
						writeTabs(code_deepness, ctx) << "if (ret.status != _InvokeStatus.Ok)" << std::endl;
						writeTabs(code_deepness, ctx) << "{ ret.status = _InvokeStatus.Error; return ret; }" << std::endl;

						writeTabs(code_deepness, ctx) << "ret.ret = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
						writeTabs(code_deepness, ctx) << "_intf._addValue(ret.ret, \"retval\", retval);" << std::endl;

						writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
						writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;

					//setter
						if (!property->readOnly())
						{
							writeTabs(code_deepness++, ctx) << "_functions[\"_set_" << property->name() << "\"] = (root, ec) => {" << std::endl;
							writeTabs(code_deepness, ctx) << "var ret = new _FunctionRet();" << std::endl;
							auto & o = writeTabs(code_deepness, ctx);
							if (!that->writeType(property->type().get(), code_deepness, ctx, ec))
								return false;
							o << " value = default(";
							if (!that->writeType(property->type().get(), code_deepness, ctx, ec))
								return false;
							o << ");" << std::endl;
							writeTabs(code_deepness, ctx) << "if (!_intf._getValue(root, \"value\", out value, ec))" << std::endl;
							writeTabs(code_deepness, ctx) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;

							writeTabs(code_deepness, ctx) << "ret.status = _callFunction(() => { " << property->name() << " = value; return null; }, ec);" << std::endl;
							writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
							writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
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
					writeTabs(code_deepness, ctx) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
					writeTabs(code_deepness, ctx) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
					if (dynamic_cast<Language::Method*>(function))
					{
						writeTabs(code_deepness, ctx) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
						writeTabs(code_deepness, ctx) << "_intf._addValue(_r, \"object_id\", _id);" << std::endl;
						writeTabs(code_deepness, ctx) << "var _v = PIDL.JSONTools.addValue(_r, \"method\", PIDL.JSONTools.Type.Object);" << std::endl;
					}
					else
					{
						writeTabs(code_deepness, ctx) << "var _intf = this;" << std::endl;
						writeTabs(code_deepness, ctx) << "var _v = PIDL.JSONTools.addValue(_root, \"function\", PIDL.JSONTools.Type.Object);" << std::endl;
					}
					writeTabs(code_deepness, ctx) << "_intf._addValue(_v, \"name\", \"" << function->name() << "\");" << std::endl;

					auto in_args = function->in_arguments();
					if (in_args.size())
					{
						writeTabs(code_deepness, ctx) << "var _aa = PIDL.JSONTools.addValue(_v, \"arguments\", PIDL.JSONTools.Type.Object);" << std::endl;
						for (const auto & a : in_args)
							writeTabs(code_deepness, ctx) << "_intf._addValue(_aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;
					}

					writeTabs(code_deepness, ctx) << "XElement _ret;" << std::endl;
					writeTabs(code_deepness, ctx) << "if (!_intf._invokeCall(_root, out _ret, _ec))" << std::endl;
					writeTabs(code_deepness + 1, ctx) << "_ec.ThrowException();" << std::endl;

					auto ret_type = function->returnType().get();
					if (!dynamic_cast<Language::Void*>(ret_type))
					{
						writeTabs(code_deepness, ctx);
						if (!that->writeType(ret_type, code_deepness, ctx, ec))
							return false;
						*ctx << " _retval;" << std::endl;
						writeTabs(code_deepness, ctx) << "if (!_intf._getValue(_ret, \"retval\", out _retval, _ec))" << std::endl;
						writeTabs(code_deepness + 1, ctx) << "_ec.ThrowException();" << std::endl;
					}

					const auto & out_args = function->out_arguments();
					if (out_args.size())
					{
						writeTabs(code_deepness, ctx) << "XElement _out_v;" << std::endl;
						writeTabs(code_deepness, ctx) << "if (!_intf._getValue(_ret, \"output\", PIDL.JSONTools.Type.Object, out _out_v, _ec))" << std::endl;
						writeTabs(code_deepness + 1, ctx) << "_ec.ThrowException();" << std::endl;

						bool has_error = false;
						for (auto & a : out_args)
						{
							auto & o = writeTabs(code_deepness, ctx) << a->name() << " = default(";
							if (!that->writeType(a->type().get(), code_deepness, ctx, ec))
								has_error = true;
							o << ");" << std::endl;
						}
						if (has_error)
							return false;

						writeTabs(code_deepness++, ctx) << "if (" << std::endl;

						bool is_first = true;
						for (auto & a : out_args)
						{
							auto & o = writeTabs(code_deepness, ctx);
							(code_deepness, ctx);
							if (is_first)
							{
								o << "  ";
								is_first = false;
							}
							else
								o << "| ";
							o << "!_getValue(_out_v, \"" << a->name() << "\", out " << a->name() << ", _ec)" << std::endl;
						}
						writeTabs(--code_deepness, ctx) << ")" << std::endl;
						writeTabs(code_deepness + 1, ctx) << "_ec.ThrowException();" << std::endl;
					}

					if (!dynamic_cast<Language::Void*>(ret_type))
						writeTabs(code_deepness, ctx) << "return _retval;" << std::endl;
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
				writeTabs(code_deepness, ctx) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
				writeTabs(code_deepness, ctx) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
				writeTabs(code_deepness, ctx) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
				writeTabs(code_deepness, ctx) << "_intf._addValue(_r, \"object_id\", _id);" << std::endl;
				writeTabs(code_deepness, ctx) << "var _v = PIDL.JSONTools.addValue(_r, \"property_get\", PIDL.JSONTools.Type.Object);" << std::endl;
				writeTabs(code_deepness, ctx) << "_intf._addValue(_v, \"name\", \"" << prop->name() << "\");" << std::endl;

				writeTabs(code_deepness, ctx) << "XElement _ret;" << std::endl;
				writeTabs(code_deepness, ctx) << "if (!_intf._invokeCall(_root, out _ret, _ec))" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "_ec.ThrowException();" << std::endl;

				auto ret_type = prop->type().get();
				writeTabs(code_deepness, ctx);
				if (!that->writeType(ret_type, code_deepness, ctx, ec))
					return false;
				*ctx << " _retval;" << std::endl;
				writeTabs(code_deepness, ctx) << "if (!_intf._getValue(_ret, \"retval\", out _retval, _ec))" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "_ec.ThrowException();" << std::endl;

				writeTabs(code_deepness, ctx) << "return _retval;" << std::endl;
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
				writeTabs(code_deepness, ctx) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
				writeTabs(code_deepness, ctx) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
				writeTabs(code_deepness, ctx) << "var _r = PIDL.JSONTools.addValue(_root, \"object_call\", PIDL.JSONTools.Type.Object);" << std::endl;
				writeTabs(code_deepness, ctx) << "_intf._addValue(_r, \"object_id\", _id);" << std::endl;
				writeTabs(code_deepness, ctx) << "var _v = PIDL.JSONTools.addValue(_r, \"property_set\", PIDL.JSONTools.Type.Object);" << std::endl;
				writeTabs(code_deepness, ctx) << "_intf._addValue(_v, \"name\", \"" << prop->name() << "\");" << std::endl;

				writeTabs(code_deepness, ctx) << "_intf._addValue(_v, \"value\", value);" << std::endl;

				writeTabs(code_deepness, ctx) << "XElement _ret;" << std::endl;
				writeTabs(code_deepness, ctx) << "if (!_intf._invokeCall(_root, out _ret, _ec))" << std::endl;
				writeTabs(code_deepness + 1, ctx) << "_ec.ThrowException();" << std::endl;
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
		priv(new Priv(this, std::make_shared<CSBasicCodegenHelper>()))
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
		priv->writeTabs(code_deepness, ctx) << "using System;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using System.Collections.Generic;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using System.Linq;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using System.Text;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "using System.Xml.Linq;" << std::endl;

		return true;
	}

	bool JSON_CSCodeGen::writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		if (!priv->writeMembers(code_deepness, ctx, intf, ec))
			return false;

		priv->writeTabs(code_deepness, ctx) << "#region marshallers" << std::endl;

		for (auto & d : intf->definitions())
		{
			bool is_first;
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
				auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
				auto s = dynamic_cast<Language::Structure*>(td->finalType().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << td->name() << " _getValue_" << td->name() << "(XElement v, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "var ret = new " << td->name() << "();" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if(!PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"value of '" << td->name() << "'is not object\");" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "isOk = false;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

					auto & members = s->members();
					if (members.size())
					{
						priv->writeTabs(code_deepness, ctx) << "isOk =" << std::endl;
						is_first = true;
						for (auto & m : members)
						{
							if (is_first)
							{
								is_first = false;
								priv->writeTabs(code_deepness + 1, ctx) << "  ";
							}
							else
								priv->writeTabs(code_deepness + 1, ctx) << "& ";
							*ctx << "_getValue(v, \"" << m->name() << "\", out ret." << m->name() << ", ec)" << std::endl;
						}
						priv->writeTabs(code_deepness + 1, ctx) << ";" << std::endl;
					}
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

					priv->writeTabs(code_deepness, ctx) << td->name() << " _getValue_" << td->name() << "(XElement r, string name, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "var ret = new " << td->name() << "();" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!_getValue(r, name, PIDL.JSONTools.Type.Object, out v, ec))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "isOk = false;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return _getValue_" << td->name() << "(v, out isOk, ec);" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				}
			}
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());
				switch (ctx->role())
				{
				case Role::Client:
					priv->writeTabs(code_deepness, ctx) << object->name() << " _getValue_" << object->name() << "(XElement v, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "string object_id;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if(!_getValue(v, out object_id, ec))" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "{ isOk = false; return null; }" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "isOk = true;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return new " << object->name() << "(this, object_id);" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

					priv->writeTabs(code_deepness, ctx) << object->name() << " _getValue_" << object->name() << "(XElement r, string name, out bool isOk, PIDL.IPIDLErrorCollector ec)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!_getValue(r, name, PIDL.JSONTools.Type.String, out v, ec))" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "{ isOk = false; return null; }" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return _getValue_" << object->name() << "(v, out isOk, ec);" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
					break;
				case Role::Server:
					break;
				}
			}
		}

		priv->writeTabs(code_deepness, ctx) << "bool _getValue(XElement r, string name, PIDL.JSONTools.Type type, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if(!PIDL.JSONTools.getValue(r, name, type, out ret))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"value '\" + name + \"' is not found or invalid\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;


		priv->writeTabs(code_deepness, ctx) << "bool _getValue<T>(XElement r, string name, out T ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "bool isOk = false;" << std::endl;
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
						priv->writeTabs(code_deepness, ctx);
					}
					else
						priv->writeTabs(code_deepness, ctx) << "else ";
					*ctx << "if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "ret = (T)(object)_getValue_" << td->name() << "(r, name, out isOk, ec);" << std::endl;
				}
			}
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());
				switch (ctx->role())
				{
				case Role::Client:
					if (is_first)
					{
						is_first = false;
						priv->writeTabs(code_deepness, ctx);
					}
					else
						priv->writeTabs(code_deepness, ctx) << "else ";
					*ctx << "if (typeof(T) == typeof(" << object->name() << "))" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "ret = (T)(object)_getValue_" << object->name() << "(r, name, out isOk, ec);" << std::endl;
					break;
				case Role::Server:
					break;
				}
			}
		}
		if (!is_first)
		{
			priv->writeTabs(code_deepness, ctx) << "else" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx);
		}
		else
			priv->writeTabs(code_deepness, ctx);
		*ctx << "isOk = PIDL.JSONTools.getValue(r, name, out ret);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!isOk)" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "ec.Add(-1, \"value '\" + name + \"' is not found or invalid\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return isOk;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "bool _getValue<T>(XElement v, out T ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		for (auto & d : intf->definitions())
		{
			if (dynamic_cast<Language::TypeDefinition*>(d.get()))
			{
				auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "bool isOk;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ret = (T)(object)_getValue_" << td->name() << "(v, out isOk, ec);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return isOk;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				}
			}
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());
				switch (ctx->role())
				{
				case Role::Client:
					priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(" << object->name() << "))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "bool isOk;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ret = (T)(object)_getValue_" << object->name() << "(v, out isOk, ec);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return isOk;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					break;
				case Role::Server:
					break;
				}
			}
		}

		priv->writeTabs(code_deepness, ctx) << "if (!PIDL.JSONTools.getValue<T>(v, out ret))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"value is invalid\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "bool _getValue<T>(XElement r, string name, out Nullable<T> ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "where T : struct" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.Type t;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL.JSONTools.getValue(r, name, out v) || (t = PIDL.JSONTools.getType(v)) == PIDL.JSONTools.Type.None)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"value '\" + name + \"' is not found or invalid\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "T _ret;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!_getValue(v, out _ret, ec))" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = _ret;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "bool _getValue<T>(XElement r, string name, out T[] ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "where T : struct" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.Type t;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL.JSONTools.getValue(r, name, out v) || (t = PIDL.JSONTools.getType(v)) == PIDL.JSONTools.Type.None)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"value '\" + name + \"' is not found or invalid\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ ret = null; return true; }" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(byte))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "string tmp;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!_getValue(v, out tmp, ec))" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{ ret = null; return false; }" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = (T[])(object)Convert.FromBase64String(tmp);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (t != PIDL.JSONTools.Type.Array)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"value '\" + name + \"' is not array\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "var elems = v.Elements(\"item\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "int i = 0;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "foreach(var e in elems)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "++i;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = new T[i];" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "i = 0;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "bool isOk = true;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "foreach(var e in elems)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!_getValue(e, out ret[i++], ec))" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "isOk = false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return isOk;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "void _addValue<T>(XElement r, string name, T val)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
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
						priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
						is_first = false;
					}
					else
						priv->writeTabs(code_deepness, ctx) << "else if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Object);" << std::endl;
					for (auto & m : s->members())
						priv->writeTabs(code_deepness, ctx) << "_addValue(v, \"" << m->name() << "\", ((" << td->name() << ")(object)val)." << m->name() << ");" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				}
			}
			else if (dynamic_cast<Language::Object*>(d.get()))
			{
				auto object = dynamic_cast<Language::Object*>(d.get());

				if (is_first)
				{
					priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(" << object->name() << "))" << std::endl;
					is_first = false;
				}
				else
					priv->writeTabs(code_deepness, ctx) << "else if (typeof(T) == typeof(" << object->name() << "))" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "_addValue(r, name, ((" << object->name() << ")(object)val)._id);" << std::endl;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
			}
		}

		if (!is_first)
		{
			priv->writeTabs(code_deepness, ctx) << "else" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "PIDL.JSONTools.addValue<T>(r, name, val);" << std::endl;
		}
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "void _addValue<T>(XElement r, string name, Nullable<T> val)" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "where T : struct //, IComparable" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (val == null)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "else" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "_addValue(r, name, val.Value);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

		priv->writeTabs(code_deepness, ctx) << "void _addValue<T>(XElement r, string name, T[] val)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (val == null)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(byte))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "_addValue(r, name, Convert.ToBase64String((byte[])(object)val));" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Array);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "foreach(var it in val)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "_addValue(v, \"item\", it);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "#endregion marshallers" << std::endl << std::endl;

		switch (ctx->role())
		{
		case Role::Server:
			priv->writeTabs(code_deepness, ctx) << "protected abstract _IObject _get_object(string object_id, PIDL.IPIDLErrorCollector ec);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "protected abstract void _dispose_object(string object_id);" << std::endl;
			break;
		case Role::Client:
			priv->writeTabs(code_deepness, ctx) << "public void _dispose_object(string object_id)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var _ec = new PIDL.PIDLExceptionErrorCollector();" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var _root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var _v = PIDL.JSONTools.addValue(_root, \"function\", PIDL.JSONTools.Type.Object);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_addValue(_v, \"name\", \"_dispose_object\");" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var _aa = PIDL.JSONTools.addValue(_v, \"arguments\", PIDL.JSONTools.Type.Object);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_addValue(_aa, \"object_id\", object_id);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "XElement _ret;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_invokeCall(_root, out _ret, _ec))" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "_ec.ThrowException();" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
		}

		return true;
	}

	bool JSON_CSCodeGen::writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Server:
			priv->writeTabs(code_deepness, ctx) << "public enum _InvokeStatus {Ok, NotImplemented, Error, MarshallingError, FatalError};" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "public _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "if (PIDL.JSONTools.getValue(root, \"function\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "string name;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_getValue(v, \"name\", out name, ec))" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _callFunction(name, v, out ret, ec);" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			priv->writeTabs(code_deepness, ctx) << "else if (PIDL.JSONTools.getValue(root, \"object_call\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "string object_id;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_getValue(v, \"object_id\", out object_id, ec))" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var obj = _get_object(object_id, ec);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if(obj == null)" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret = null; return _InvokeStatus.Error; }" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return obj._invoke(v, out ret, ec);" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;



			priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus.MarshallingError;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			
			break;
		case Role::Client:
			priv->writeTabs(code_deepness, ctx) << "public enum _InvokeStatus {Ok, NotImplemented, Error, MarshallingError, FatalError};" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "protected abstract _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec);" << std::endl << std::endl;

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
			priv->writeTabs(code_deepness, ctx) << "_functions[\"_dispose_object\"] = (root, ec) =>" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var ret = new _FunctionRet();" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "string _arg_object_id = default(string);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "XElement aa;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_getValue(root, \"arguments\", PIDL.JSONTools.Type.Object, out aa, ec))" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_getValue(aa, \"object_id\", out _arg_object_id, ec))" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret.status = _InvokeStatus.MarshallingError; return ret; }" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var stat = _callFunction(() => { _dispose_object(_arg_object_id); return null; }, ec);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (stat != _InvokeStatus.Ok)" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret.status = _InvokeStatus.Error; return ret; }" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ret.status = _InvokeStatus.Ok;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
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
			priv->writeTabs(code_deepness, ctx) << "if (_intf != null)" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "_intf._dispose_object(_id);" << std::endl;
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
			priv->writeTabs(code_deepness, ctx) << "public _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "if (PIDL.JSONTools.getValue(root, \"method\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "string name;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _callFunction(name, v, out ret, ec);" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "else if (PIDL.JSONTools.getValue(root, \"property_get\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "string name;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _callFunction(\"_get_\"+name, v, out ret, ec);" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "else if (PIDL.JSONTools.getValue(root, \"property_set\", out v) && PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "string name;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_intf._getValue(v, \"name\", out name, ec))" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{ ret = null; return _InvokeStatus.MarshallingError; }" << std::endl << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _callFunction(\"_set_\"+name, v, out ret, ec);" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

			priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus.MarshallingError;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

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
