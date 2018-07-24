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

#include "include/pidlBackend/cppcodegen.h"
#include "include/pidlBackend/language.h"

#include <assert.h>

namespace PIDL
{

	struct CPPCodeGenContext::Priv
	{
		Priv(std::ostream & o_, Nature nature_, Role role_) : 
			o(o_), 
			nature(nature_), 
			role(role_)
		{ }

		std::ostream & o;
		Nature nature; 
		Role role;
	};

	CPPCodeGenContext::CPPCodeGenContext(std::ostream & o, Nature nature, Role role) : priv(new Priv(o, nature, role))
	{ }

	CPPCodeGenContext::~CPPCodeGenContext()
	{
		delete priv;
	}

	std::ostream & CPPCodeGenContext::stream() const
	{
		return priv->o;
	}

	CPPCodeGenContext::Nature CPPCodeGenContext::nature() const
	{
		return priv->nature;
	}

	CPPCodeGenContext::Role CPPCodeGenContext::role() const
	{
		return priv->role;
	}


	CPPCodeGenLogging::CPPCodeGenLogging() : priv(nullptr)
	{ }

	CPPCodeGenLogging::~CPPCodeGenLogging() = default;

	CPPVoidLogging::CPPVoidLogging() : priv(nullptr)
	{ }

	CPPVoidLogging::~CPPVoidLogging() = default;

	std::string CPPVoidLogging::initLogger(const std::string & scope) const { return std::string(); }
	std::string CPPVoidLogging::loggerType() const { return std::string(); }
	std::string CPPVoidLogging::loggingStart(const std::string & logger) const { return std::string(); }
	std::string CPPVoidLogging::loggingAssert(const std::string & logger, const std::string & expression, const std::string & message) const { return std::string(); }
	std::string CPPVoidLogging::loggingTrace(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CPPVoidLogging::loggingDebug(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CPPVoidLogging::loggingInfo(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CPPVoidLogging::loggingWarning(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CPPVoidLogging::loggingError(const std::string & logger, const std::string & message) const { return std::string(); }
	std::string CPPVoidLogging::loggingFatal(const std::string & logger, const std::string & message) const { return std::string(); }


	CPPCodeGenHelper::CPPCodeGenHelper() : priv(nullptr)
	{ }

	CPPCodeGenHelper::~CPPCodeGenHelper() = default;


	struct CPPBasicCodegenHelper::Priv
	{
		Priv(const std::vector<Include> & customIncludes_) : 
			customIncludes(customIncludes_), 
			logging(std::make_shared<CPPVoidLogging>())
		{ }

		std::vector<Include> customIncludes;
		std::shared_ptr<CPPCodeGenLogging> logging;
	};

	CPPBasicCodegenHelper::CPPBasicCodegenHelper(const std::vector<Include> & customIncludes) : CPPCodeGenHelper(), priv(new Priv(customIncludes))
	{ }

	CPPBasicCodegenHelper::~CPPBasicCodegenHelper()
	{
		delete priv;
	}

	std::vector<CPPCodeGenHelper::Include> CPPBasicCodegenHelper::includes() const
	{
		return priv->customIncludes;
	}

	CPPCodeGenHelper::Include CPPBasicCodegenHelper::coreIncludePath() const
	{
		return std::make_pair(IncludeType::GLobal, "pidlCore");
	}

	std::string CPPBasicCodegenHelper::getName(const Language::TopLevel * t) const
	{
		return t->name();
	}

	short CPPBasicCodegenHelper::tabDefinition(char & ch) const
	{
		ch = '\t';
		return 1;
	}

	std::shared_ptr<CPPCodeGenLogging> CPPBasicCodegenHelper::logging() const
	{
		return priv->logging;
	}


	struct CPPCodeGen::Priv 
	{
		Priv(CPPCodeGen * obj_) : 
			obj(obj_)
		{ }

		CPPCodeGen * obj;
		
		std::ostream & writeTabs(short code_deepness, CPPCodeGenContext * ctx) const
		{
			char ch;
			for (short i = 0, l = code_deepness * obj->helper()->tabDefinition(ch); i < l; ++i)
				*ctx << ch;
			return ctx->stream();
		}

		bool addStructureBudy(short code_deepness, CPPCodeGenContext * ctx, Language::Structure * structure, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			for (auto & member : structure->members())
			{
				writeTabs(code_deepness, ctx);
				if (!addType(code_deepness, ctx, member->type().get(), ec))
					return false;
				o << " " << member->name() << ";" << std::endl;
			}

			return true;
		}

		bool addStructure(short code_deepness, CPPCodeGenContext * ctx, Language::TypeDefinition * structure, ErrorCollector & ec)
		{
			writeTabs(code_deepness, ctx) << "struct " << structure->name() << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;

			auto struct_type = dynamic_cast<Language::Structure*>(structure->type().get());
			assert(struct_type);

			if (!addStructureBudy(code_deepness, ctx, struct_type, ec))
				return false;

			writeTabs(--code_deepness, ctx) << "};" << std::endl;

			return true;
		}

		bool addStructure(short code_deepness, CPPCodeGenContext * ctx, Language::Structure * structure, ErrorCollector & ec)
		{
			writeTabs(code_deepness, ctx) << "struct " << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;

			if (!addStructureBudy(code_deepness, ctx, structure, ec))
				return false;

			writeTabs(--code_deepness, ctx) << "}";

			return true;
		}

		bool addGeneric(short code_deepness, CPPCodeGenContext * ctx, Language::Generic * generic, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			if (dynamic_cast<Language::Nullable*>(generic))
				o << "nullable<";
			else if (dynamic_cast<Language::Array*>(generic))
				o << "array<";
			if (!addType(code_deepness, ctx, generic->type().get(), ec))
				return false;
			o << ">";
			return true;
		}

		bool addNative(CPPCodeGenContext * ctx, Language::NativeType * native, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			if (dynamic_cast<Language::Integer*>(native))
				o << "long long";
			else if (dynamic_cast<Language::Float*>(native))
				o << "double";
			else if (dynamic_cast<Language::Boolean*>(native))
				o << "bool";
			return true;
		}

		bool addEmbedded(CPPCodeGenContext * ctx, Language::EmbeddedType * embedded, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			if (dynamic_cast<Language::String*>(embedded))
				o << "string";
			else if (dynamic_cast<Language::DateTime*>(embedded))
				o << "datetime";
			else if (dynamic_cast<Language::Blob*>(embedded))
				o << "blob";

			return true;
		}

		bool addType(short code_deepness, CPPCodeGenContext * ctx, Language::Type * type, ErrorCollector & ec)
		{
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

		bool addTypeDefinition(short code_deepness, CPPCodeGenContext * ctx, Language::TypeDefinition * type_definition, ErrorCollector & ec)
		{
			switch (ctx->nature())
			{
			case Nature::AllInOne:
			case Nature::Declaration:
				{
					auto type = type_definition->type().get();
					if (dynamic_cast<Language::Structure*>(type))
					{
						if (!addStructure(code_deepness, ctx, type_definition, ec))
							return false;
					}
					else
					{
						auto & o = writeTabs(code_deepness, ctx) << "typedef ";
						if (!addType(code_deepness, ctx, type, ec))
							return false;
						o << " " << type_definition->name() << ";" << std::endl;
					}
				}
				break;
			case Nature::Implementatinon:
				break;
			}

			return true;
		}

		bool addFunction(short code_deepness, CPPCodeGenContext * ctx, Language::Function * function, ErrorCollector & ec)
		{
			if (ctx->role() == Role::Server && ctx->nature() == Nature::Implementatinon)
				return true;

			auto & o = writeTabs(code_deepness, ctx);

			switch (ctx->nature())
			{
			case Nature::AllInOne:
			case Nature::Declaration:
				switch (ctx->role())
				{
				case Role::Client:
					break;
				case Role::Server:
					o << "virtual ";
					break;
				}
				break;
			case Nature::Implementatinon:
				break;
			}

			if (!addType(code_deepness, ctx, function->returnType().get(), ec))
				return false;

			switch (ctx->nature())
			{
			case Nature::AllInOne:
			case Nature::Declaration:
				o << " " << function->name() << "(";
				break;
			case Nature::Implementatinon:
				o << " ";
				for (auto sc : function->scope())
					o << sc << "::";
				o << function->name() << "(";
				break;
			}

			bool is_first_arg = true;
			for (auto & arg : function->arguments())
			{
				if (!is_first_arg)
					o << ", ";
				is_first_arg = false;
				switch (arg->direction())
				{
				case Language::Function::Argument::Direction::In:
					o << "const ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					o << " & " << arg->name();
					break;
				case Language::Function::Argument::Direction::InOut:
					o << "/*in-out*/ ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					o << " & " << arg->name();
					break;
				case Language::Function::Argument::Direction::Out:
					o << "/*out*/ ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					o << " & " << arg->name();
					break;
				}
			}
			o << ")";

			switch (ctx->nature())
			{
			case Nature::AllInOne:
			case Nature::Implementatinon:
				switch (ctx->role())
				{
				case Role::Client:
					o << std::endl;
					writeTabs(code_deepness++, ctx) << "{" << std::endl;

					if (!obj->writeFunctionBody(function, code_deepness, ctx, ec))
						return false;

					writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
					break;
				case Role::Server:
					o << " = 0;" << std::endl;
					break;
				}
				break;
			case Nature::Declaration:
				switch (ctx->role())
				{
				case Role::Client:
					o << ";" << std::endl;
					break;
				case Role::Server:
					o << " = 0;" << std::endl;
					break;
				}
				break;
			}

			return true;
		}

		bool addDefinition(short code_deepness, CPPCodeGenContext * ctx, Language::Definition * definition, ErrorCollector & ec)
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

		bool addDefinitions(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
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

		bool addInterface(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			switch (ctx->nature())
			{
			case Nature::AllInOne:
			case Nature::Declaration:
				writeTabs(code_deepness, ctx) << "class " << obj->helper()->getName(intf) << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (!obj->writePrivateSection(code_deepness, ctx, intf, ec))
					return false;
				writeTabs(code_deepness - 1, ctx) << "public:" << std::endl;
				break;
			case Nature::Implementatinon:
				if (!obj->writePrivateSection(code_deepness, ctx, intf, ec))
					return false;
				break;
			}

			if (!obj->writePublicSection(code_deepness, ctx, intf, ec))
				return false;

			switch (ctx->nature())
			{
			case Nature::AllInOne:
			case Nature::Declaration:
				writeTabs(code_deepness - 1, ctx) << "protected:" << std::endl;
				break;
			case Nature::Implementatinon:
				break;
			}

			if (!obj->writeProtectedSection(code_deepness, ctx, intf, ec))
				return false;

			switch (ctx->nature())
			{
			case Nature::AllInOne:
			case Nature::Declaration:
				writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
				break;
			case Nature::Implementatinon:
				break;
			}

			return true;
		}

		bool addConstructor(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			switch (ctx->nature())
			{
			case Nature::AllInOne:
				writeTabs(code_deepness, ctx) << intf->name() << "()" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (!obj->writeConstructorBody(intf, code_deepness, ctx, ec))
					return false;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			case Nature::Declaration:
				writeTabs(code_deepness, ctx) << intf->name() << "();" << std::endl;
				break;
			case Nature::Implementatinon:
				writeTabs(code_deepness, ctx) << intf->name() << "::" << intf->name() << "() : priv(new Priv(this)) { }" << std::endl << std::endl;
				break;
			}

			return true;
		}

		bool addDestructor(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			switch (ctx->nature())
			{
			case Nature::AllInOne:
				writeTabs(code_deepness, ctx) << "virtual ~" << intf->name() << "() { }" << std::endl << std::endl;
				break;
			case Nature::Declaration:
				writeTabs(code_deepness, ctx) << "virtual ~" << intf->name() << "();" << std::endl;
				break;
			case Nature::Implementatinon:
				writeTabs(code_deepness, ctx) << intf->name() << "::~" << intf->name() << "() { delete priv; }" << std::endl << std::endl;
				break;
			}

			return true;
		}

		bool addModule(short code_deepness, CPPCodeGenContext * ctx, Language::Module * module, ErrorCollector & ec)
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

		bool addTopLevel(short code_deepness, CPPCodeGenContext * ctx, Language::TopLevel * top_level, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::Interface*>(top_level))
				return addInterface(code_deepness, ctx, dynamic_cast<Language::Interface*>(top_level), ec);
			if (dynamic_cast<Language::Module*>(top_level))
				return addModule(code_deepness, ctx, dynamic_cast<Language::Module*>(top_level), ec);

			return true;
		}
	};

	CPPCodeGen::CPPCodeGen() : priv(new Priv(this))
	{ }

	CPPCodeGen::~CPPCodeGen()
	{
		delete priv;
	}

	bool CPPCodeGen::generateCode(Language::TopLevel * topLevel, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->addTopLevel(code_deepness, ctx, topLevel, ec);
	}

	bool CPPCodeGen::generateIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		for (auto & include : helper()->includes())
		{
			if (!writeInclude(code_deepness, ctx, include, ec))
				return false;
		}

		return writeIncludes(code_deepness, ctx, ec);
	}

	bool CPPCodeGen::writeInclude(short code_deepness, CPPCodeGenContext * ctx, const CPPCodeGenHelper::Include & include, ErrorCollector & ec)
	{
		auto & o = priv->writeTabs(code_deepness, ctx) << "#include ";
		switch (include.first)
		{
		case CPPCodeGenHelper::IncludeType::GLobal:
			o << "<" << include.second << ">";
			break;
		case CPPCodeGenHelper::IncludeType::Local:
			o << "\"" << include.second << "\"";
			break;
		}
		o << std::endl;

		return true;
	}

	bool CPPCodeGen::writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		switch (ctx->nature())
		{
		case Nature::Implementatinon:
			priv->writeTabs(code_deepness, ctx) << "struct " << helper()->getName(intf) << "::Priv" << std::endl;
			priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;

			switch (ctx->nature())
			{
			case Nature::AllInOne:
			case Nature::Declaration:
				break;
			case Nature::Implementatinon:
				priv->writeTabs(code_deepness, ctx) << "Priv(" << helper()->getName(intf) << " * that_): that(that_)" << std::endl;
				priv->writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (!writeConstructorBody(intf, code_deepness, ctx, ec))
					return false;
				priv->writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				priv->writeTabs(code_deepness, ctx) << helper()->getName(intf) << " * that;" << std::endl << std::endl;
				break;
			}

			if (!writePrivateMembers(code_deepness, ctx, intf, ec))
				return false;
			priv->writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
			break;
		case Nature::Declaration:
			priv->writeTabs(code_deepness, ctx) << "struct Priv;" << std::endl;
			priv->writeTabs(code_deepness, ctx) << "Priv * priv;" << std::endl;
			break;
		case Nature::AllInOne:
			if (!writePrivateMembers(code_deepness, ctx, intf, ec))
				return false;
			break;
		}

		return true;
	}

	bool CPPCodeGen::writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		if (!writeAliases(code_deepness, ctx, ec))
			return false;

		switch (ctx->role())
		{
		case Role::Client:
			if (!priv->addConstructor(code_deepness, ctx, intf, ec) ||
			    !priv->addDestructor(code_deepness, ctx, intf, ec) ||
				!priv->addDefinitions(code_deepness, ctx, intf, ec))
				return false;
			break;
		case Role::Server:
			if (!priv->addDestructor(code_deepness, ctx, intf, ec) ||
			    !writeInvoke(code_deepness, ctx, intf, ec))
				return false;
			break;
		}

		return true;
	}

	bool CPPCodeGen::writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		switch (ctx->role())
		{
		case Role::Client:
			if (!writeInvoke(code_deepness, ctx, intf, ec))
				return false;
			break;
		case Role::Server:
			if (!priv->addConstructor(code_deepness, ctx, intf, ec) ||
				!priv->addDefinitions(code_deepness, ctx, intf, ec))
				return false;
			break;
		}

		return true;
	}


	bool CPPCodeGen::writeType(Language::Type * type, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->addType(code_deepness, ctx, type, ec);
	}

}
