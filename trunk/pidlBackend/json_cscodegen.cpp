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
			priv->writeTabs(code_deepness, ctx) << "Dictionary<string, Func<XElement, PIDL.IErrorCollector, XElement>> _functions = new Dictionary<string, Func<XElement, PIDL.IErrorCollector, XElement>>();" << std::endl;
			break;
		case Role::Client:
			break;
		}

		priv->writeTabs(code_deepness, ctx) << "#region marshallers" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "static my_struct getValue__my_struct(XElement r, string name, out bool isOk)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "var ret = new my_struct();" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL.JSONTools.getValue(r, name, PIDL.JSONTools.Type.Object, out v))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "isOk = false;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return getValue__my_struct(v, out isOk);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		//for (auto & d : intf->definitions())
		//{
		//	auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
		//	if (td)
		//	{
		//		auto s = dynamic_cast<Language::Structure*>(td->type().get());
		//		if (s)
		//		{
		//			priv->writeTabs(code_deepness, ctx) << "static bool getValue__" << td->name() << "(rapidjson::Value & v, " << td->name() << " & ret)" << std::endl;
		//			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		//			priv->writeTabs(code_deepness, ctx) << "return !v.IsNull() && v.IsObject()" << std::endl;
		//			++code_deepness;
		//			for (auto & m : s->members())
		//				priv->writeTabs(code_deepness, ctx) << "&& getValue(v, \"" << m->name() << "\", ret." << m->name() << ")" << std::endl;
		//			--code_deepness;
		//			priv->writeTabs(code_deepness, ctx) << ";" << std::endl;
		//			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
		//		}
		//	}
		//}

		for (auto & d : intf->definitions())
		{
			auto td = dynamic_cast<Language::TypeDefinition*>(d.get());
			if (td)
			{
				auto s = dynamic_cast<Language::Structure*>(td->type().get());
				if (s)
				{
					priv->writeTabs(code_deepness, ctx) << "static my_struct getValue__" << td->name() << "(XElement v, out bool isOk)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "var ret = new " << td->name() << "();" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "isOk = PIDL.JSONTools.checkType(v, PIDL.JSONTools.Type.Object)" << std::endl;
					for (auto & m : s->members())
						priv->writeTabs(code_deepness + 1, ctx) << "&& getValue(v, \"" << m->name() << "\", out ret."<< m->name() << ")" << std::endl;
					priv->writeTabs(code_deepness + 1, ctx) << ";" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				}
			}
		}

		priv->writeTabs(code_deepness, ctx) << "static bool getValue<T>(XElement r, string name, out T ret)" << std::endl;
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
					priv->writeTabs(code_deepness, ctx) << "ret = (T)(object)getValue__" << td->name() << "(r, name, out isOk);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return isOk;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				}
			}
		}
		priv->writeTabs(code_deepness, ctx) << "return PIDL.JSONTools.getValue(r, name, out ret);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static bool getValue<T>(XElement v, out T ret)" << std::endl;
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
					priv->writeTabs(code_deepness, ctx) << "ret = (T)(object)getValue__" << td->name() << "(v, out isOk);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return isOk;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				}
			}
		}

		priv->writeTabs(code_deepness, ctx) << "return PIDL.JSONTools.getValue<T>(v, out ret);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static bool getValue<T>(XElement r, string name, out Nullable<T> ret)" << std::endl;
		priv->writeTabs(code_deepness + 1, ctx) << "where T : struct" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.Type t;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL.JSONTools.getValue(r, name, out v) || (t = PIDL.JSONTools.getType(v)) == PIDL.JSONTools.Type.None)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (t == PIDL.JSONTools.Type.Null)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "T _ret;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!getValue(v, out _ret))" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = _ret;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static bool getValue<T>(XElement r, string name, out T[] ret)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "where T : struct" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.Type t;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!PIDL.JSONTools.getValue(r, name, out v) || (t = PIDL.JSONTools.getType(v)) == PIDL.JSONTools.Type.None)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
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
		priv->writeTabs(code_deepness, ctx) << "if (!getValue(v, out tmp))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = (T[])(object)Convert.FromBase64String(tmp);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (t != PIDL.JSONTools.Type.Array)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = null;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "var elems = v.Elements(\"item\");" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "int i = 0;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "foreach(var e in elems)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "++i;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "ret = new T[i];" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "i = 0;" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "foreach(var e in elems)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (!getValue(e, out ret[i++]))" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return false;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return true;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static void addValue<T>(XElement r, string name, T val)" << std::endl;
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
					priv->writeTabs(code_deepness, ctx) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Object);" << std::endl;
					++code_deepness;
					for (auto & m : s->members())
						priv->writeTabs(code_deepness, ctx) << "addValue(v, \"" << m->name() << "\", ((" << td->name() << ")(object)val)." << m->name() << ");" << std::endl;
					--code_deepness;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "else" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.addValue<T>(r, name, val);" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				}
			}
		}
		priv->writeTabs(code_deepness, ctx) << "static void addValue<T>(XElement r, string name, Nullable<T> val)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "where T : struct //, IComparable" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (val == null)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "else" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "addValue(r, name, val.Value);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

		priv->writeTabs(code_deepness, ctx) << "static void addValue<T>(XElement r, string name, T[] val)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (val == null)" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Null);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "if (typeof(T) == typeof(byte))" << std::endl;
		priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "addValue(r, name, Convert.ToBase64String((byte[])(object)val));" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "return;" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "var v = PIDL.JSONTools.addValue(r, name, PIDL.JSONTools.Type.Array);" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "foreach(var it in val)" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "addValue(v, \"item\", it);" << std::endl;
		priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
		priv->writeTabs(code_deepness, ctx) << "#endregion marshallers" << std::endl << std::endl;

		return true;
	}

	bool JSON_CSCodeGen::writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Server:
			priv->writeTabs(code_deepness, ctx) << "public XElement _invoke(XElement root, PIDL.IErrorCollector ec)" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "XElement v;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!PIDL.JSONTools.getValue(root, \"function\", PIDL.JSONTools.Type.Object, out v))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"'function' is not found or invalid\");" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return null;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "string name;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!getValue(v, \"name\", out name))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"name of function is not found or invalid\");" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return null;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "if (!_functions.ContainsKey(name))" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"function '\" + name + \"' is not found\");" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return null;" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "return _functions[name](v, ec);" << std::endl;
			priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			break;
		case Role::Client:
			priv->writeTabs(code_deepness, ctx) << "protected abstract XElement _invoke(XElement root, PIDL.IErrorCollector ec);" << std::endl << std::endl;
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
				priv->writeTabs(code_deepness, ctx) << "var ec = new PIDL.ExceptionErrorCollector();" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "var root = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "var v = PIDL.JSONTools.addValue(root, \"function\", PIDL.JSONTools.Type.Object);" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "addValue(v, \"name\", \"my_function\");" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "var aa = PIDL.JSONTools.addValue(v, \"arguments\", PIDL.JSONTools.Type.Object);" << std::endl;

				for (const auto & a : function->in_arguments())
						priv->writeTabs(code_deepness, ctx) << "addValue(aa, \"" << a->name() << "\", " << a->name() << ");" << std::endl;

				priv->writeTabs(code_deepness, ctx) << "XElement ret;" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "if ((ret = _invoke(root, ec)) == null)" << std::endl;
				priv->writeTabs(code_deepness + 1, ctx) << "ec.ThrowException();" << std::endl;

				auto ret_type = function->returnType().get();
				if (!dynamic_cast<Language::Void*>(ret_type))
				{
					priv->writeTabs(code_deepness, ctx);
					if (!writeType(ret_type, code_deepness, ctx, ec))
						return false;
					*ctx << " __retval;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!getValue(ret, \"retval\", out __retval))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"'retval' is not found or invalid\");" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.ThrowException();" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
				}

				const auto & out_args = function->out_arguments();
				if (out_args.size())
				{
					priv->writeTabs(code_deepness, ctx) << "XElement out_v;" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "if (!PIDL.JSONTools.getValue(ret, \"output\", PIDL.JSONTools.Type.Object, out out_v))" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"'output' is not found or invalid\");" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.ThrowException();" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "out_val = null;" << std::endl;
					auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;

					bool is_first = true;
					for (auto & a : out_args)
					{
						priv->writeTabs(code_deepness, ctx);
						if (!is_first)
							o << "|| ";
						is_first = false;
						o << "!getValue(out_v, \"" << a->name() << "\", out " << a->name() << ")" << std::endl;
					}
					priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"could not get one or more arguments\");" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "ec.ThrowException();" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
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
					priv->writeTabs(code_deepness++, ctx) << "_functions[\"my_function\"] = (root, ec) => {" << std::endl;

					for (auto & a : function->arguments())
					{
						auto & o = priv->writeTabs(code_deepness, ctx);
						if (!writeType(a->type().get(), code_deepness, ctx, ec))
							return false;
						o << " arg__" << a->name() << ";" << std::endl;
					}

					const auto & in_args = function->in_arguments();
					if (in_args.size())
					{
						priv->writeTabs(code_deepness, ctx) << "XElement aa;" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "if ((aa = root.Element(\"arguments\")) == null)" << std::endl;
						priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"arguments are not found\");" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "return null;" << std::endl;
						priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

						auto & o = priv->writeTabs(code_deepness++, ctx) << "if (" << std::endl;
						bool is_first = true;
						for (auto & a : in_args)
						{
							priv->writeTabs(code_deepness, ctx);
							if (!is_first)
								o << "|| ";
							is_first = false;
							o << "!getValue(aa, \"" << a->name() << "\", out arg__" << a->name() << ")" << std::endl;
						}
						priv->writeTabs(--code_deepness, ctx) << ")" << std::endl;

						priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "ec.Add(-1, \"one or more arguments are not found\");" << std::endl;
						priv->writeTabs(code_deepness, ctx) << "return null;" << std::endl;
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
							o << "arg__" << a->name();
							break;
						case Language::Function::Argument::Direction::InOut:
							o << "ref arg__" << a->name();
							break;
						case Language::Function::Argument::Direction::Out:
							o << "out arg__" << a->name();
							break;
						}
					}
					o << ");" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "catch (PIDL.Exception e)" << std::endl;
					priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "e.Get(ec);" << std::endl;
					priv->writeTabs(code_deepness, ctx) << "return null;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "}" << std::endl;

					priv->writeTabs(code_deepness, ctx) << "var ret = PIDL.JSONTools.createValue(\"root\", PIDL.JSONTools.Type.Object);" << std::endl;

					if (ret_type)
						priv->writeTabs(code_deepness, ctx) << "addValue(ret, \"retval\", __retval);" << std::endl;

					const auto & out_args = function->out_arguments();
					if (out_args.size())
					{
						priv->writeTabs(code_deepness, ctx) << "var out_v = PIDL.JSONTools.addValue(ret, \"output\", PIDL.JSONTools.Type.Object);" << std::endl;

						for (auto & a : out_args)
							priv->writeTabs(code_deepness, ctx) << "addValue(out_v, \"" << a->name() << "\", arg__" << a->name() << ");" << std::endl;
					}

					priv->writeTabs(code_deepness, ctx) << "return ret;" << std::endl;
					priv->writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
				}
			}
			break;
		}
		return true;
	}

}
