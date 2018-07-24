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

#include "include/pidlBackend/cscodegen.h"
#include "include/pidlBackend/language.h"

#include <assert.h>

namespace PIDL {

	struct CSCodeGenContext::Priv
	{
		Priv(std::ostream & o_, Role role_) :
			o(o_),
			role(role_)
		{ }

		std::ostream & o;
		Role role;
	};

	CSCodeGenContext::CSCodeGenContext(std::ostream & o, Role role) : priv(new Priv(o, role))
	{ }

	CSCodeGenContext::~CSCodeGenContext()
	{
		delete priv;
	}

	std::ostream & CSCodeGenContext::stream() const
	{
		return priv->o;
	}

	CSCodeGenContext::Role CSCodeGenContext::role() const
	{
		return priv->role;
	}


	CSCodeGenLogging::CSCodeGenLogging() : priv(nullptr)
	{ }

	CSCodeGenLogging::~CSCodeGenLogging() = default;

	CSVoidLogging::CSVoidLogging() : priv(nullptr)
	{ }

	CSVoidLogging::~CSVoidLogging() = default;

	std::string CSVoidLogging::initLogger(const std::string & scope) const { return std::string(); }
	std::string CSVoidLogging::loggerType() const { return std::string(); }
	std::string CSVoidLogging::loggingStart(const std::string & logger) const { return std::string(); }
	std::string CSVoidLogging::loggingAssert(const std::string & logger, const std::string & expression, const std::string & message) const { return std::string(); }
	std::string CSVoidLogging::loggingTrace(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CSVoidLogging::loggingDebug(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CSVoidLogging::loggingInfo(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CSVoidLogging::loggingWarning(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CSVoidLogging::loggingError(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CSVoidLogging::loggingFatal(const std::string & logger, const std::string & message) const { return std::string(); }


	CSCodeGenHelper::CSCodeGenHelper() : priv(nullptr)
	{ }

	CSCodeGenHelper::~CSCodeGenHelper() = default;


	struct CSBasicCodegenHelper::Priv
	{
		Priv() :
			logging(std::make_shared<CSVoidLogging>())
		{ }

		std::shared_ptr<CSCodeGenLogging> logging;
	};

	CSBasicCodegenHelper::CSBasicCodegenHelper() : CSCodeGenHelper(), priv(new Priv())
	{ }

	CSBasicCodegenHelper::~CSBasicCodegenHelper()
	{
		delete priv;
	}

	std::string CSBasicCodegenHelper::getName(const Language::TopLevel * t) const
	{
		return t->name();
	}

	short CSBasicCodegenHelper::tabDefinition(char & ch) const
	{
		ch = '\t';
		return 1;
	}

	std::shared_ptr<CSCodeGenLogging> CSBasicCodegenHelper::logging() const
	{
		return priv->logging;
	}

	struct CSCodeGen::Priv
	{
		Priv(CSCodeGen * obj_) :
			obj(obj_)
		{ }

		CSCodeGen * obj;

		std::ostream & writeTabs(short code_deepness, CSCodeGenContext * ctx) const
		{
			char ch;
			for (short i = 0, l = code_deepness * obj->helper()->tabDefinition(ch); i < l; ++i)
				*ctx << ch;
			return ctx->stream();
		}

		bool addStructureBudy(short code_deepness, CSCodeGenContext * ctx, Language::Structure * structure, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			for (auto & member : structure->members())
			{
				writeTabs(code_deepness, ctx) << "public ";
				if (!addType(code_deepness, ctx, member->type().get(), ec))
					return false;
				o << " " << member->name() << ";" << std::endl;
			}

			return true;
		}

		bool addStructure(short code_deepness, CSCodeGenContext * ctx, Language::TypeDefinition * structure, ErrorCollector & ec)
		{
			auto & o = writeTabs(code_deepness, ctx);
			switch (ctx->role())
			{
			case CSCodeGenContext::Role::Client:
				 o << "public ";
				 break;
			case CSCodeGenContext::Role::Server:
				o << "protected ";
				break;
			}
			o << "struct " << structure->name() << std::endl;

			writeTabs(code_deepness++, ctx) << "{" << std::endl;

			auto struct_type = dynamic_cast<Language::Structure*>(structure->type()->finalType().get());
			assert(struct_type);

			if (!addStructureBudy(code_deepness, ctx, struct_type, ec))
				return false;

			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			return true;
		}

		bool addStructure(short code_deepness, CSCodeGenContext * ctx, Language::Structure * structure, ErrorCollector & ec)
		{
			writeTabs(code_deepness, ctx) << "struct " << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;

			if (!addStructureBudy(code_deepness, ctx, structure, ec))
				return false;

			writeTabs(--code_deepness, ctx) << "}";

			return true;
		}

		bool addGeneric(short code_deepness, CSCodeGenContext * ctx, Language::Generic * generic, ErrorCollector & ec)
		{
			auto type = generic->type()->finalType().get();
			if (dynamic_cast<Language::Nullable*>(generic))
			{
				if (dynamic_cast<Language::NativeType*>(type) ||
					dynamic_cast<Language::DateTime*>(type) ||
					dynamic_cast<Language::Structure*>(type))
				{
					*ctx << "Nullable<";
					if (!addType(code_deepness, ctx, type, ec))
						return false;
					*ctx << ">";
				}
				else
				{
					*ctx << "/*nullable*/ ";
					if (!addType(code_deepness, ctx, type, ec))
						return false;
				}
			}
			else if (dynamic_cast<Language::Array*>(generic))
			{
				if (!addType(code_deepness, ctx, type, ec))
					return false;
				*ctx << "[]";
			}
			return true;
		}

		bool addNative(CSCodeGenContext * ctx, Language::NativeType * native, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			if (dynamic_cast<Language::Integer*>(native))
				o << "long";
			else if (dynamic_cast<Language::Float*>(native))
				o << "double";
			else if (dynamic_cast<Language::Boolean*>(native))
				o << "bool";
			return true;
		}

		bool addEmbedded(CSCodeGenContext * ctx, Language::EmbeddedType * embedded, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			if (dynamic_cast<Language::String*>(embedded))
				o << "string";
			else if (dynamic_cast<Language::DateTime*>(embedded))
				o << "DateTime";
			else if (dynamic_cast<Language::Blob*>(embedded))
				o << "byte[]";

			return true;
		}

		bool addType(short code_deepness, CSCodeGenContext * ctx, Language::Type * type_, ErrorCollector & ec)
		{
			Language::Type * type = dynamic_cast<Language::Structure*>(type_->finalType().get()) ? type_ : type_->finalType().get();

			if (dynamic_cast<Language::Generic*>(type))
			{
				if (!addGeneric(code_deepness, ctx, dynamic_cast<Language::Generic*>(type), ec))
					return false;
			}
			else if (dynamic_cast<Language::NativeType*>(type))
			{
				if (!addNative(ctx, dynamic_cast<Language::NativeType*>(type), ec))
					return false;
			}
			else if (dynamic_cast<Language::EmbeddedType*>(type))
			{
				if (!addEmbedded(ctx, dynamic_cast<Language::EmbeddedType*>(type), ec))
					return false;
			}
			else if (dynamic_cast<Language::Structure*>(type))
			{
				if (!addStructure(code_deepness, ctx, dynamic_cast<Language::Structure*>(type), ec))
					return false;
			}
			else
			{
				*ctx << type->name();
			}

			return true;
		}

		bool addTypeDefinition(short code_deepness, CSCodeGenContext * ctx, Language::TypeDefinition * type_definition, ErrorCollector & ec)
		{
			auto type = type_definition->type()->finalType().get();
			if (dynamic_cast<Language::Structure*>(type))
			{
				if (!addStructure(code_deepness, ctx, type_definition, ec))
					return false;
			}
			else
			{
				//type definitions are not really supoported by C# :-(
			}

			return true;
		}

		bool addFunction(short code_deepness, CSCodeGenContext * ctx, Language::Function * function, ErrorCollector & ec)
		{
			auto & o = writeTabs(code_deepness, ctx);

			switch (ctx->role())
			{
			case CSCodeGenContext::Role::Client:
				o << "public " ;
				break;
			case CSCodeGenContext::Role::Server:
				o << "protected abstract ";
				break;
			}

			if (!addType(code_deepness, ctx, function->returnType().get(), ec))
				return false;

			o << " " << function->name() << "(";

			bool is_first_arg = true;
			for (auto & arg : function->arguments())
			{
				if (!is_first_arg)
					o << ", ";
				is_first_arg = false;
				switch (arg->direction())
				{
				case Language::Function::Argument::Direction::In:
					addType(code_deepness, ctx, arg->type().get(), ec);
					o << " " << arg->name();
					break;
				case Language::Function::Argument::Direction::InOut:
					o << "ref ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					o << " " << arg->name();
					break;
				case Language::Function::Argument::Direction::Out:
					o << "out ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					o << " " << arg->name();
					break;
				}
			}
			o << ")";

			switch (ctx->role())
			{
			case CSCodeGenContext::Role::Client:
				o << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;

				if (!obj->writeFunctionBody(function, code_deepness, ctx, ec))
					return false;

				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			case CSCodeGenContext::Role::Server:
				o << ";" << std::endl;
				break;
			}

			return true;
		}

		bool addDefinition(short code_deepness, CSCodeGenContext * ctx, Language::Definition * definition, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::TypeDefinition*>(definition))
			{
				if (!addTypeDefinition(code_deepness, ctx, dynamic_cast<Language::TypeDefinition*>(definition), ec))
					return false;
			}
			else if (dynamic_cast<Language::Function*>(definition))
			{
				if (!addFunction(code_deepness, ctx, dynamic_cast<Language::Function*>(definition), ec))
					return false;
			}
			return true;
		}

		bool addDefinitions(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			for (auto & definition : intf->definitions())
				if (dynamic_cast<Language::TypeDefinition*>(definition.get()))
				{
					if (!addTypeDefinition(code_deepness, ctx, dynamic_cast<Language::TypeDefinition*>(definition.get()), ec))
						return false;
				}
				else if (dynamic_cast<Language::Function*>(definition.get()))
				{
					if (!addFunction(code_deepness, ctx, dynamic_cast<Language::Function*>(definition.get()), ec))
						return false;
				}

			return true;
		}

		bool addInterface(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			writeTabs(code_deepness, ctx) << "abstract class " << obj->helper()->getName(intf) << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;

			if (!obj->writeMembers(code_deepness, ctx, intf, ec) ||
				!addConstructor(code_deepness, ctx, intf, ec) ||
				!addDefinitions(code_deepness, ctx, intf, ec) ||
				!obj->writeInvoke(code_deepness, ctx, intf, ec))
				return false;

			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			return true;
		}

		bool addConstructor(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			writeTabs(code_deepness, ctx) << "protected " << intf->name() << "()" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			if (!obj->writeConstructorBody(intf, code_deepness, ctx, ec))
				return false;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			return true;
		}

		bool addModule(short code_deepness, CSCodeGenContext * ctx, Language::Module * module, ErrorCollector & ec)
		{
			auto & o = writeTabs(code_deepness++, ctx) << "namespace " << obj->helper()->getName(module) << " {" << std::endl;
			for (auto & element : module->elements())
			{
				o << std::endl;
				if (!addTopLevel(code_deepness, ctx, element.get(), ec))
					return false;
			}
			writeTabs(--code_deepness, ctx) << "}" << std::endl;
			return true;
		}

		bool addTopLevel(short code_deepness, CSCodeGenContext * ctx, Language::TopLevel * top_level, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::Interface*>(top_level))
				return addInterface(code_deepness, ctx, dynamic_cast<Language::Interface*>(top_level), ec);
			if (dynamic_cast<Language::Module*>(top_level))
				return addModule(code_deepness, ctx, dynamic_cast<Language::Module*>(top_level), ec);

			return true;
		}
	};

	CSCodeGen::CSCodeGen() : priv(new Priv(this))
	{ }

	CSCodeGen::~CSCodeGen()
	{
		delete priv;
	}

	bool CSCodeGen::generateCode(Language::TopLevel * topLevel, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->addTopLevel(code_deepness, ctx, topLevel, ec);
	}

	bool CSCodeGen::generateUsings(short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		return writeUsings(code_deepness, ctx, ec);
	}

	bool CSCodeGen::writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		return true;
	}

	bool CSCodeGen::writeType(Language::Type * type, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->addType(code_deepness, ctx, type, ec);
	}

}
