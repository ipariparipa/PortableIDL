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
		Priv(const std::shared_ptr<CSCodeGenHelper> & helper_) : helper(helper_)
		{ }

		std::shared_ptr<CSCodeGenHelper> helper;

		std::ostream & writeTabs(short code_deepness, CSCodeGenContext * ctx) const
		{
			char ch;
			for (short i = 0, l = code_deepness * helper->tabDefinition(ch); i < l; ++i)
				*ctx << ch;
			return ctx->stream();
		}

	};

	JSON_CSCodeGen::JSON_CSCodeGen(const std::shared_ptr<CSCodeGenHelper> & helper) :
		CSCodeGen(),
		priv(new Priv(helper))
	{ }

	JSON_CSCodeGen::JSON_CSCodeGen() :
		CSCodeGen(),
		priv(new Priv(std::make_shared<CSBasicCodegenHelper>()))
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
		switch (ctx->role())
		{
		case Role::Server:
			priv->writeTabs(code_deepness, ctx) << "struct __FunctionRet { public _InvokeStatus status; public XElement ret; };" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "Dictionary<string, Func<XElement, PIDL.IErrorCollector, __FunctionRet>> __functions = new Dictionary<string, Func<XElement, PIDL.IErrorCollector, __FunctionRet>>();" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "_InvokeStatus __callFunction(string name, XElement root, out XElement ret, PIDL.IErrorCollector ec)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!__functions.ContainsKey(name))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"function '\" + name + \"' is not found\");" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus.NotImplemented;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var retval = __functions[name](root, ec);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ret = retval.ret;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return retval.status;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			break;
		case Role::Client:
			priv->writeTabs(code_deepness, ctx) << "bool __invokeCall(XElement root, out XElement ret, PIDL.IErrorCollector ec)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "var status = _invoke(root, out ret, ec);" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "switch(status)" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus.Ok:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "break;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus.NotImplemented:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "ec.Add((int)status, \"function is not implemented\");" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus.Error:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "ec.Add((int)status, \"error while executing server function\");" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus.FatalError:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "ec.Add((int)status, \"fatal error while executing server function\");" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "case _InvokeStatus.MarshallingError:" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "ec.Add((int)status, \"error while marshalling of function call\");" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "return false;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "}" << std::endl << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			break;
		}

		priv->writeTabs(code_deepness, ctx) << "#region marshallers" << std::endl;

		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			bool is_first;
			if (td)
			{
				auto s = dynamic_cast<Language::Structure*>(td->finalType().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << "static " << td->name() << " __getValue__" << td->name() << "(XElement v, out bool isOk, PIDL.IErrorCollector ec)" << std::endl;
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
							*ctx << "__getValue(v, \"" << m->name() << "\", out ret." << m->name() << ", ec)" << std::endl;
						}
						priv->writeTabs(code_deepness + 1, ctx) << ";" << std::endl;
					}
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

					priv->writeTabs(code_deepness, ctx) << "static " << td->name() << " __getValue__" << td->name() << "(XElement r, string name, out bool isOk, PIDL.IErrorCollector ec)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "var ret = new " << td->name() << "();" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!__getValue(r, name, PIDL.JSONTools.Type.Object, out v, ec))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "isOk = false;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return __getValue__" << td->name() << "(v, out isOk, ec);" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				}
			}
		}

		priv->writeTabs(code_deepness, ctx) << "static bool __getValue(XElement r, string name, PIDL.JSONTools.Type type, out XElement ret, PIDL.IErrorCollector ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if(!PIDL.JSONTools.getValue(r, name, type, out ret))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"value '\" + name + \"' is not found or invalid\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;


		priv->writeTabs(code_deepness, ctx) << "static bool __getValue<T>(XElement r, string name, out T ret, PIDL.IErrorCollector ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "bool isOk = false;" << std::endl;
		bool is_first = true;
		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			if (td)
			{
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
					priv->writeTabs(code_deepness + 1, ctx) << "ret = (T)(object)__getValue__" << td->name() << "(r, name, out isOk, ec);" << std::endl;
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

		priv->writeTabs(code_deepness, ctx) << "static bool __getValue<T>(XElement v, out T ret, PIDL.IErrorCollector ec)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			if (td)
			{
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(" << td->name() << "))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "bool isOk;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ret = (T)(object)__getValue__" << td->name() << "(v, out isOk, ec);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return isOk;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
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

		priv->writeTabs(code_deepness, ctx) << "static bool __getValue<T>(XElement r, string name, out Nullable<T> ret, PIDL.IErrorCollector ec)" << std::endl;
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
		priv->writeTabs(code_deepness, ctx) << "if (!__getValue(v, out _ret, ec))" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = _ret;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static bool __getValue<T>(XElement r, string name, out T[] ret, PIDL.IErrorCollector ec)" << std::endl;
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
		priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(byte))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "string tmp;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!__getValue(v, out tmp, ec))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
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
		priv->writeTabs(code_deepness, ctx) << "if (!__getValue(e, out ret[i++], ec))" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "isOk = false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return isOk;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static void __addValue<T>(XElement r, string name, T val)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		is_first = true;
		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			if (td)
			{
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
						priv->writeTabs(code_deepness, ctx) << "__addValue(v, \"" << m->name() << "\", ((" << td->name() << ")(object)val)." << m->name() << ");" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				}
			}
		}
		if (!is_first)
		{
			priv->writeTabs(code_deepness, ctx) << "else" << std::endl;
			priv->writeTabs(code_deepness + 1, ctx) << "PIDL.JSONTools.addValue<T>(r, name, val);" << std::endl;
		}
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static void __addValue<T>(XElement r, string name, Nullable<T> val)" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "where T : struct //, IComparable" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (val == null)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "else" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "__addValue(r, name, val.Value);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static void __addValue<T>(XElement r, string name, T[] val)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (val == null)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(byte))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "__addValue(r, name, Convert.ToBase64String((byte[])(object)val));" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Array);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "foreach(var it in val)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "__addValue(v, \"item\", it);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "#endregion marshallers" << std::endl << std::endl;

		return true;
	}

	bool JSON_CSCodeGen::writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Server:
			priv->writeTabs(code_deepness, ctx) << "public enum _InvokeStatus {Ok, NotImplemented, Error, MarshallingError, FatalError};" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "public _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IErrorCollector ec)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!__getValue(root, \"function\", PIDL.JSONTools.Type.Object, out v, ec))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus.MarshallingError;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			priv->writeTabs(code_deepness, ctx) << "string name;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!__getValue(v, \"name\", out name, ec))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _InvokeStatus.MarshallingError;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return __callFunction(name, v, out ret, ec);" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			break;
		case Role::Client:
			priv->writeTabs(code_deepness, ctx) << "protected enum _InvokeStatus {Ok, NotImplemented, Error, MarshallingError, FatalError};" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "protected abstract _InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IErrorCollector ec);" << std::endl << std::endl;
			break;
		}

		return true;
	}

	bool JSON_CSCodeGen::writeFunctionBody(Language::Function * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Client:
			{
				priv->writeTabs(code_deepness, ctx) << "var __ec = new PIDL.ExceptionErrorCollector();" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "var __root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "var __v = PIDL.JSONTools.addValue(__root, \"function\", PIDL.JSONTools.Type.Object);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "__addValue(__v, \"name\", \"" << function->name() << "\");" << std::endl;

				auto in_args = function->in_arguments();
				if (in_args.size())
				{
					priv->writeTabs(code_deepness, ctx) << "var __aa = PIDL.JSONTools.addValue(__v, \"arguments\", PIDL.JSONTools.Type.Object);" << std::endl;
					for (const auto & a : in_args)
						priv->writeTabs(code_deepness, ctx) << "__addValue(__aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;
				}

				priv->writeTabs(code_deepness, ctx) << "XElement __ret;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if (!__invokeCall(__root, out __ret, __ec))" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "__ec.ThrowException();" << std::endl;

				auto ret_type = function->returnType().get();
				if (!dynamic_cast<Language::Void*>(ret_type))
				{
					priv->writeTabs(code_deepness, ctx);
					if (!writeType(ret_type, code_deepness, ctx, ec))
						return false;
					*ctx << " __retval;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!__getValue(__ret, \"retval\", out __retval, __ec))" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "__ec.ThrowException();" << std::endl;
				}

				const auto & out_args = function->out_arguments();
				if (out_args.size())
				{
					priv->writeTabs(code_deepness, ctx) << "XElement __out_v;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!__getValue(__ret, \"output\", PIDL.JSONTools.Type.Object, out __out_v, __ec))" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "__ec.ThrowException();" << std::endl;

					bool has_error = false;
					for (auto & a : out_args)
					{
						auto & o = priv->writeTabs(code_deepness, ctx) << a->name() << " = default(";
						if (!writeType(a->type().get(), code_deepness, ctx, ec))
							has_error = true;
						o << ");" << std::endl;
					}
					if (has_error)
						return false;

					auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;

					bool is_first = true;
					for (auto & a : out_args)
					{
						priv->writeTabs(code_deepness, ctx);
						if (!is_first)
							o << "| ";
						is_first = false;
						o << "!__getValue(__out_v, \"" << a->name() << "\", out " << a->name() << ", __ec)" << std::endl;
					}
					priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << "__ec.ThrowException();" << std::endl;
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

	bool JSON_CSCodeGen::writeConstructorBody(Language::Interface * intf, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
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
					priv->writeTabs(code_deepness++, ctx) << "__functions[\"" << function->name() << "\"] = (root, ec) => {" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "var ret = new __FunctionRet();" << std::endl;
					for (auto & a : function->arguments())
					{
						auto & o = priv->writeTabs(code_deepness, ctx);
						if (!writeType(a->type().get(), code_deepness, ctx, ec))
							return false;
						o << " __arg__" << a->name() << ";" << std::endl;
					}

					const auto & in_args = function->in_arguments();
					if (in_args.size())
					{
						priv->writeTabs(code_deepness, ctx) << "XElement aa;" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "if (!__getValue(root, \"arguments\", PIDL.JSONTools.Type.Object, out aa, ec))" << std::endl;
						priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "ret.status = _InvokeStatus.MarshallingError;" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
						priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

						auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;
						bool is_first = true;
						for (auto & a : in_args)
						{
							priv->writeTabs(code_deepness, ctx);
							if (!is_first)
								o << "| ";
							is_first = false;
							o << "!__getValue(aa, \"" << a->name() << "\", out __arg__" << a->name() << ", ec)" << std::endl;
						}
						priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;
						priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "ret.status = _InvokeStatus.MarshallingError;" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
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
						*ctx << " retval;" << std::endl;
					}

					priv->writeTabs(code_deepness, ctx) << "try" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					auto & o = priv->writeTabs(code_deepness, ctx);
					if (ret_type)
						o << "retval = ";
					o << function->name() << "(";
					bool is_first = true;
					for (auto & a : function->arguments())
					{
						if (!is_first)
							o << ", ";
						is_first = false;
						switch (a->direction())
						{
						case Language::Function::Argument::Direction::In:
							o << "__arg__" << a->name();
							break;
						case Language::Function::Argument::Direction::InOut:
							o << "ref __arg__" << a->name();
							break;
						case Language::Function::Argument::Direction::Out:
							o << "out __arg__" << a->name();
							break;
						}
					}
					o << ");" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "catch (PIDL.Exception e)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "e.Get(ec);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ret.status = _InvokeStatus.Error;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "catch (Exception e)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"unhandled exception: '\" + e.ToString() + \"'\");" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ret.status = _InvokeStatus.FatalError;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

					priv->writeTabs(code_deepness, ctx) << "ret.ret = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

					if (ret_type)
						priv->writeTabs(code_deepness, ctx) << "__addValue(ret.ret, \"retval\", retval);" << std::endl;

					const auto & out_args = function->out_arguments();
					if (out_args.size())
					{
						priv->writeTabs(code_deepness, ctx) << "var out_v = PIDL.JSONTools.addValue(ret.ret, \"output\", PIDL.JSONTools.Type.Object);" << std::endl;

						for (auto & a : out_args)
							priv->writeTabs(code_deepness, ctx) << "__addValue(out_v, \"" << a->name() << "\", __arg__" << a->name() << ");" << std::endl;
					}

					priv->writeTabs(code_deepness, ctx) << "ret.status = _InvokeStatus.Ok;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
				}
			}
			break;
		}
		return true;
	}

}
