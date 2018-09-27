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
#include "include/pidlBackend/cstyledocumentation.h"

#include <assert.h>

#include <sstream>
#include <algorithm>

namespace PIDL
{

	struct CPPCodeGenContext::Priv
	{
		Priv(Mode mode_) :
			mode(mode_)
		{ }

		Mode mode;
	};

	CPPCodeGenContext::CPPCodeGenContext(short tab_length, char tab_char, std::ostream & o, Role role, Mode mode) :
		CodeGenContext(tab_length, tab_char, o, role),
		priv(new Priv(mode))
	{ }

	CPPCodeGenContext::~CPPCodeGenContext()
	{
		delete priv;
	}

	CPPCodeGenContext::Mode CPPCodeGenContext::mode() const
	{
		return priv->mode;
	}



	//struct CPPCodeGenLogging::Priv { };
	CPPCodeGenLogging::CPPCodeGenLogging() : priv(nullptr){ }
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


	struct CPPBasicCodeGenHelper::Priv
	{
		Priv(const std::vector<Include> & customIncludes_) : 
			customIncludes(customIncludes_), 
			logging(std::make_shared<CPPVoidLogging>()),
			documentation(std::make_shared<CStyleVoidDocumentation>())
		{ }

		std::vector<Include> customIncludes;
		std::shared_ptr<CPPCodeGenLogging> logging;
		std::shared_ptr<CStyleDocumentation> documentation;
	};

	CPPBasicCodeGenHelper::CPPBasicCodeGenHelper(const std::vector<Include> & customIncludes) : CPPCodeGenHelper(), priv(new Priv(customIncludes))
	{ }

	CPPBasicCodeGenHelper::~CPPBasicCodeGenHelper()
	{
		delete priv;
	}

	std::vector<CPPCodeGenHelper::Include> CPPBasicCodeGenHelper::includes() const
	{
		return priv->customIncludes;
	}

	CPPCodeGenHelper::Include CPPBasicCodeGenHelper::coreIncludePath() const
	{
		return std::make_pair(IncludeType::GLobal, "pidlCore");
	}

	std::string CPPBasicCodeGenHelper::getName(const Language::TopLevel * e) const
	{
		return e->name();
	}

	std::shared_ptr<CPPCodeGenLogging> CPPBasicCodeGenHelper::logging() const
	{
		return priv->logging;
	}

	std::shared_ptr<CStyleDocumentation> CPPBasicCodeGenHelper::documentation() const
	{
		return priv->documentation;
	}


	struct CPPCodeGen::Priv 
	{
		Priv(CPPCodeGen * that_) :
			that(that_)
		{ }

		CPPCodeGen * that;
		
		CPPCodeGenHelper * helper() const
		{
			return that->helper();
		}

		bool writeDocumentation(short code_deepness, CPPCodeGenContext * ctx, CStyleDocumentation::Place place, Language::DocumentationProvider *docprov, ErrorCollector & ec)
		{
			return that->helper()->documentation()->write(code_deepness, ctx, place, docprov, ec);
		}

		std::string getName(Language::TopLevel * t)
		{
			return helper()->getName(t);
		}

		template<class T>
		std::string getScope(const T * t, bool isComplete = true)
		{
			std::string ret;
			bool is_first = true;
			for (auto & sc : t->scope())
			{
				if (is_first)
					is_first = false;
				else
					ret += "::";
				ret += sc;
			}
			if (isComplete && !is_first)
				ret += "::";
			return ret;
		}

		bool addStructureBudy(short code_deepness, CPPCodeGenContext * ctx, Language::Structure * structure, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			for (auto & member : structure->members())
			{
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, member.get(), ec))
					return false;
				ctx->writeTabs(code_deepness);
				if (!addType(code_deepness, ctx, member->type().get(), ec))
					return false;
				o << " " << member->name() << ";";
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, member.get(), ec))
					return false;
				o << std::endl;
			}

			return true;
		}

		bool addStructure(short code_deepness, CPPCodeGenContext * ctx, Language::TypeDefinition * structure, ErrorCollector & ec)
		{
			if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, structure, ec))
				return false;
			ctx->writeTabs(code_deepness) << "struct " << structure->name() << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;

			auto struct_type = dynamic_cast<Language::Structure*>(structure->type().get());
			assert(struct_type);

			if (!addStructureBudy(code_deepness, ctx, struct_type, ec))
				return false;

			ctx->writeTabs(--code_deepness) << "};";
			if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, structure, ec))
				return false;
			**ctx << std::endl;

			return true;
		}

		bool addStructure(short code_deepness, CPPCodeGenContext * ctx, Language::Structure * structure, ErrorCollector & ec)
		{
			ctx->writeTabs(code_deepness) << "struct " << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;

			if (!addStructureBudy(code_deepness, ctx, structure, ec))
				return false;

			ctx->writeTabs(--code_deepness) << "}";

			return true;
		}

		bool addGeneric(short code_deepness, CPPCodeGenContext * ctx, Language::Generic * generic, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			if (dynamic_cast<Language::Nullable*>(generic))
				o << "nullable<";
			else if (dynamic_cast<Language::Array*>(generic))
				o << "array<";
			else if (dynamic_cast<Language::Tuple*>(generic))
				o << "tuple<";

			bool is_first = true;
			for (auto & t : generic->types())
			{
				if (is_first)
					is_first = false;
				else
					*ctx << ", ";
				if (!addType(code_deepness, ctx, t.get(), ec))
					return false;
			}

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

		bool addObject(CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
		{
			*ctx << getScope(obj) << obj->name() << "::Ptr";
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
			else if (dynamic_cast<Language::Object*>(type))
			{
				if (!addObject(ctx, dynamic_cast<Language::Object*>(type), ec))
					return false;
			}
			else
			{
				*ctx << getScope(type) << type->name();
			}

			return true;
		}

		bool writeTypeDefinition(short code_deepness, CPPCodeGenContext * ctx, Language::TypeDefinition * type_definition, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				{
					auto type = type_definition->type().get();
					if (dynamic_cast<Language::Structure*>(type))
					{
						if (!addStructure(code_deepness, ctx, type_definition, ec))
							return false;
					}
					else
					{
						if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, type_definition, ec))
							return false;
						auto & o = ctx->writeTabs(code_deepness) << "typedef ";
						if (!addType(code_deepness, ctx, type, ec))
							return false;
						o << " " << type_definition->name() << ";";
						if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, type_definition, ec))
							return false;
						o << std::endl;
					}
				}
				break;
			case Mode::Implementatinon:
				break;
			}

			return true;
		}

		bool writeFunction(short code_deepness, CPPCodeGenContext * ctx, Language::FunctionVariant * function, ErrorCollector & ec)
		{
			if (ctx->role() == Role::Server && ctx->mode() == Mode::Implementatinon)
				return true;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, function, ec))
					return false;
				switch (ctx->role())
				{
				case Role::Client:
					ctx->writeTabs(code_deepness);
					break;
				case Role::Server:
					ctx->writeTabs(code_deepness) << "virtual ";
					break;
				}
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness);
				break;
			}

			if (!addType(code_deepness, ctx, function->returnType().get(), ec))
				return false;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				*ctx << " " << function->name() << "(";
				break;
			case Mode::Implementatinon:
				*ctx << " " << getScope(function->function().get()) << function->name() << "(";
				break;
			}

			size_t _args_i = 0;
			auto & _args = function->arguments();
			for (auto & arg : _args)
			{
				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, arg.get(), ec))
						return false;
					break;
				case Mode::Implementatinon:
					break;
				}

				switch (arg->direction())
				{
				case Language::FunctionVariant::Argument::Direction::In:
					*ctx << "const ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					*ctx << " & " << arg->name();
					break;
				case Language::FunctionVariant::Argument::Direction::InOut:
					*ctx << "/*in-out*/ ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					*ctx << " & " << arg->name();
					break;
				case Language::FunctionVariant::Argument::Direction::Out:
					*ctx << "/*out*/ ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					*ctx << " & " << arg->name();
					break;
				}

				if (++_args_i != _args.size())
					**ctx << ", ";

				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, arg.get(), ec))
						return false;
					break;
				case Mode::Implementatinon:
					break;
				}
			}
			*ctx << ")";

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Implementatinon:
				switch (ctx->role())
				{
				case Role::Client:
					**ctx << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;

					if (!that->writeFunctionBody(function, code_deepness, ctx, ec))
						return false;

					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					break;
				case Role::Server:
					*ctx << " = 0;";
					break;
				}
				break;
			case Mode::Declaration:
				switch (ctx->role())
				{
				case Role::Client:
					*ctx << ";";
					break;
				case Role::Server:
					*ctx << " = 0;";
					break;
				}
				break;
			}

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, function, ec))
					return false;
				//no break
			case Mode::Implementatinon:
				**ctx << std::endl;
			}

			return true;
		}

		bool writeProperty(short code_deepness, CPPCodeGenContext * ctx, Language::Property * property, ErrorCollector & ec)
		{
			if (ctx->role() == Role::Server && ctx->mode() == Mode::Implementatinon)
				return true;

		//getter
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, property, ec))
					return false;
				switch (ctx->role())
				{
				case Role::Client:
					ctx->writeTabs(code_deepness);
					break;
				case Role::Server:
					ctx->writeTabs(code_deepness) << "virtual ";
					break;
				}
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness);
				break;
			}

			if (!addType(code_deepness, ctx, property->type().get(), ec))
				return false;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				*ctx << " get_" << property->name() << "()";
				break;
			case Mode::Implementatinon:
				*ctx << " " << getScope(property) << "get_" << property->name() << "()";
				break;
			}

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Implementatinon:
				switch (ctx->role())
				{
				case Role::Client:
					**ctx << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;

					if (!that->writePropertyGetterBody(property, code_deepness, ctx, ec))
						return false;

					ctx->writeTabs(--code_deepness) << "}" << std::endl;
					break;
				case Role::Server:
					*ctx << " = 0;";
					break;
				}
				break;
			case Mode::Declaration:
				switch (ctx->role())
				{
				case Role::Client:
					*ctx << ";";
					break;
				case Role::Server:
					*ctx << " = 0;";
					break;
				}
				break;
			}
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, property, ec))
					return false;
				//no break
			case Mode::Implementatinon:
				**ctx << std::endl;
				break;
			}

			//setter
			if(!property->readOnly())
			{
				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, property, ec))
						return false;
					switch (ctx->role())
					{
					case Role::Client:
						ctx->writeTabs(code_deepness);
						break;
					case Role::Server:
						ctx->writeTabs(code_deepness) << "virtual ";
						break;
					}
					break;
				case Mode::Implementatinon:
					ctx->writeTabs(code_deepness);
					break;
				}

				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					*ctx << "void set_" << property->name() << "(const ";
					break;
				case Mode::Implementatinon:
					*ctx << "void " << getScope(property) << "set_" << property->name() << "(const ";
					break;
				}

				if (!addType(code_deepness, ctx, property->type().get(), ec))
					return false;

				*ctx << " & value)";

				switch (ctx->role())
				{
				case Role::Client:
					switch (ctx->mode())
					{
					case Mode::AllInOne:
					case Mode::Implementatinon:
						**ctx << std::endl;
						ctx->writeTabs(code_deepness++) << "{" << std::endl;

						if (!that->writePropertySetterBody(property, code_deepness, ctx, ec))
							return false;

						ctx->writeTabs(--code_deepness) << "}" << std::endl;
						break;
					case Mode::Declaration:
						switch (ctx->role())
						{
						case Role::Client:
							*ctx << ";";
							break;
						case Role::Server:
							*ctx << " = 0;";
							break;
						}
						break;
					}
					break;
				case Role::Server:
					*ctx << " = 0;";
					break;
				}

				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, property, ec))
						return false;
					//no break
				case Mode::Implementatinon:
					**ctx << std::endl;
					break;
				}
			}

			return true;
		}

		bool writeObject(short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, obj, ec))
					return false;
				ctx->writeTabs(code_deepness) << "class " << obj->name() << " : public _Object" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writePrivateSection(code_deepness, ctx, obj, ec))
					return false;
				ctx->writeTabs(code_deepness - 1) << "public:" << std::endl;
				ctx->writeTabs(code_deepness) << "typedef std::shared_ptr<" << obj->name() << "> Ptr;" << std::endl;
				break;
			case Mode::Implementatinon:
				if (!that->writePrivateSection(code_deepness, ctx, obj, ec))
					return false;
				break;
			}

			if (!that->writePublicSection(code_deepness, ctx, obj, ec))
				return false;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness - 1) << "protected:" << std::endl;
				break;
			case Mode::Implementatinon:
				break;
			}

			if (!that->writeProtectedSection(code_deepness, ctx, obj, ec))
				return false;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(--code_deepness) << "};" << std::endl;
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, obj, ec))
					return false;
				**ctx << std::endl;
				break;
			case Mode::Implementatinon:
				break;
			}

			return true;
		}

		bool writeConstructor(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * cl, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
				ctx->writeTabs(code_deepness) << cl->name() << "()" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writeConstructorBody(cl, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << cl->name() << "();" << std::endl;
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << getScope(cl) << cl->name() << "::" << cl->name() << "() : _priv(new _Priv(this))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ }" << std::endl << std::endl;
				break;
			}

			return true;
		}

		bool writeConstructor(short code_deepness, CPPCodeGenContext * ctx, Language::Object * cl, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
				ctx->writeTabs(code_deepness) << cl->name() << "(" << getScope(cl, false) << " * intf, const std::string & id)" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writeConstructorBody(cl, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << cl->name() << "(" << getScope(cl, false) << " * intf, const std::string & id);" << std::endl;
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << getScope(cl) << cl->name() << "::" << cl->name() << "(" << getScope(cl, false) << " * intf, const std::string & id) : _priv(new _Priv(this, intf, id))" << std::endl;
				ctx->writeTabs(code_deepness) << "{ }" << std::endl << std::endl;
				break;
			}

			return true;
		}

		template<class Class_T>
		bool writeDestructor(short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
				ctx->writeTabs(code_deepness) << "virtual ~" << cl->name() << "()" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writeDestructorBody(cl, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "virtual ~" << cl->name() << "();" << std::endl;
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << getScope(cl) << cl->name() << "::~" << cl->name() << "()" << std::endl;
				ctx->writeTabs(code_deepness) << "{ delete _priv; }" << std::endl << std::endl;
				break;
			}

			return true;
		}
		
		bool writeObjectDefinition(short code_deepness, CPPCodeGenContext * ctx, Language::Definition * definition, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::Property*>(definition))
			{
				if (!writeProperty(code_deepness, ctx, dynamic_cast<Language::Property*>(definition), ec))
					return false;
			}
			else if (dynamic_cast<Language::Method*>(definition))
			{
				for (auto &v : dynamic_cast<Language::Method*>(definition)->variants())
				if (!writeFunction(code_deepness, ctx, v.second.get(), ec))
					return false;
			}
			return true;
		}

		bool writeDefinition(short code_deepness, CPPCodeGenContext * ctx, Language::Definition * definition, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::TypeDefinition*>(definition))
			{
				if (!writeTypeDefinition(code_deepness, ctx, dynamic_cast<Language::TypeDefinition*>(definition), ec))
					return false;
			}
			else if (dynamic_cast<Language::Function*>(definition))
			{
				for (auto &v : dynamic_cast<Language::Function*>(definition)->variants())
					if (!writeFunction(code_deepness, ctx, v.second.get(), ec))
						return false;
			}
			else if (dynamic_cast<Language::Object*>(definition))
			{
				if (!writeObject(code_deepness, ctx, dynamic_cast<Language::Object*>(definition), ec))
					return false;
			}
			return true;
		}

		bool writeDefinitions(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!that->writeObjectBase(intf, code_deepness, ctx, ec))
					return false;
				break;
			case Mode::Implementatinon:
				break;
			}

			for (auto & definition : intf->definitions())
				if (dynamic_cast<Language::TypeDefinition*>(definition.get()))
				{
					if (!writeTypeDefinition(code_deepness, ctx, dynamic_cast<Language::TypeDefinition*>(definition.get()), ec))
						return false;
				}
				else if (dynamic_cast<Language::FunctionVariant*>(definition.get()))
				{
					if (!writeFunction(code_deepness, ctx, dynamic_cast<Language::FunctionVariant*>(definition.get()), ec))
						return false;
				}
				else if (dynamic_cast<Language::Object*>(definition.get()))
				{
					if (!writeObject(code_deepness, ctx, dynamic_cast<Language::Object*>(definition.get()), ec))
						return false;
				}

			return true;
		}

		bool writeDefinitions(short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
		{
			for (auto & definition : obj->definitions())
				if (dynamic_cast<Language::Property*>(definition.get()))
				{
					if (!writeProperty(code_deepness, ctx, dynamic_cast<Language::Property*>(definition.get()), ec))
						return false;
				}
				else if (dynamic_cast<Language::MethodVariant*>(definition.get()))
				{
					if (!writeFunction(code_deepness, ctx, dynamic_cast<Language::MethodVariant*>(definition.get()), ec))
						return false;
				}

			return true;
		}

		bool writeInterface(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, intf, ec))
					return false;
				ctx->writeTabs(code_deepness) << "class " << that->helper()->getName(intf) << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writePrivateSection(code_deepness, ctx, intf, ec))
					return false;
				ctx->writeTabs(code_deepness - 1) << "public:" << std::endl;
				break;
			case Mode::Implementatinon:
				if (!that->writePrivateSection(code_deepness, ctx, intf, ec))
					return false;
				break;
			}

			if (!that->writePublicSection(code_deepness, ctx, intf, ec))
				return false;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(code_deepness - 1) << "protected:" << std::endl;
				break;
			case Mode::Implementatinon:
				break;
			}

			if (!that->writeProtectedSection(code_deepness, ctx, intf, ec))
				return false;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				ctx->writeTabs(--code_deepness) << "};" << std::endl;
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, intf, ec))
					return false;
				**ctx << std::endl;
				break;
			case Mode::Implementatinon:
				break;
			}

			return true;
		}

		bool writeLongString(short code_deepness, CPPCodeGenContext * ctx, const std::string & str, ErrorCollector & ec)
		{
			if (!str.length())
			{
				*ctx << "\"\"";
				return true;
			}

			auto normalize = [](std::string str) -> std::string
			{
				auto replace_all = [](std::string & str, const std::string & from, const std::string & to) -> std::string &
				{
					for (size_t start_pos = 0; (start_pos = str.find(from, start_pos)) != std::string::npos; start_pos += to.length())
						str.replace(start_pos, from.length(), to);
					return str;
				};

				replace_all(str, "\\", "\\\\");
				replace_all(str, "\n", "\\n");
				replace_all(str, "\r", "\\r");
				replace_all(str, "\t", "\\t");
				replace_all(str, "\"", "\\\"");

				return str;
			};

			bool is_first = true;
			for (size_t i = 0; i < str.length(); i += 512)
			{
				if (is_first)
					is_first = false;
				else
				{
					ctx->stream() << std::endl;
					ctx->writeTabs(code_deepness + 2);
				}
				if (i + 512 > str.length())
					*ctx << "\"" << normalize(str.substr(i, str.length() - i)) << "\"";
				else
					*ctx << "\"" << normalize(str.substr(i, 512)) << "\"";
			}

			return true;
		}

		bool writeModule(short code_deepness, CPPCodeGenContext * ctx, Language::Module * module, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, module, ec))
					return false;
				break;
			case Mode::Implementatinon:
				break;
			}

			auto & o = ctx->writeTabs(code_deepness++) << "namespace " << that->helper()->getName(module) << " {" << std::endl;

			switch (ctx->role())
			{
			case Role::Client:
				break;
			case Role::Server:
				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Implementatinon:
					if (module->info().size())
					{
						ctx->writeTabs(code_deepness++) << "namespace _Info {" << std::endl;
						for (auto & i : module->info())
						{
							ctx->writeTabs(code_deepness) << "const char * " << i.first << " =";
							if (!writeLongString(code_deepness, ctx, i.second, ec))
								return false;
							*ctx << ";" << std::endl << std::endl;
						}
						ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					}
					break;
				case Mode::Declaration:
					if (module->info().size())
					{
						ctx->writeTabs(code_deepness++) << "namespace _Info {" << std::endl;
						for (auto & i : module->info())
						{
							ctx->writeTabs(code_deepness) << "extern const char * " << i.first << ";" << std::endl << std::endl;
						}
						ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					}
					break;
					break;
				}
				break;
			}

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!that->writeAliases(code_deepness, ctx, ec))
					return false;
				break;
			case Mode::Implementatinon:
				break;
			}

			for (auto & element : module->elements())
			{
				o << std::endl;
				if (!writeTopLevel(code_deepness, ctx, element.get(), ec))
					return false;
			}
			ctx->writeTabs(--code_deepness) << "}" << std::endl;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::After, module, ec))
					return false;
				break;
			case Mode::Implementatinon:
				break;
			}
			**ctx << std::endl;

			return true;
		}

		bool writeTopLevel(short code_deepness, CPPCodeGenContext * ctx, Language::TopLevel * top_level, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::Interface*>(top_level))
				return writeInterface(code_deepness, ctx, dynamic_cast<Language::Interface*>(top_level), ec);
			if (dynamic_cast<Language::Module*>(top_level))
				return writeModule(code_deepness, ctx, dynamic_cast<Language::Module*>(top_level), ec);

			return true;
		}

		bool writePriv(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * cl, ErrorCollector & ec)
		{
			ctx->writeTabs(code_deepness) << "_Priv(" << cl->name() << " * _that_): _that(_that_)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			if (!that->writeConstructorBody(cl, code_deepness, ctx, ec))
				return false;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "~_Priv()" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			if (!that->writeDestructorBody(cl, code_deepness, ctx, ec))
				return false;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << cl->name() << " * _that;" << std::endl << std::endl;

			return true;
		}

		bool writePriv(short code_deepness, CPPCodeGenContext * ctx, Language::Object * cl, ErrorCollector & ec)
		{
			ctx->writeTabs(code_deepness) << "_Priv(" << cl->name() << " * _that_, " << getScope(cl, false) << " * _intf_, const std::string & _id_): _that(_that_), _intf(_intf_), __id(_id_)" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			if (!that->writeConstructorBody(cl, code_deepness, ctx, ec))
				return false;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			ctx->writeTabs(code_deepness) << "~_Priv()" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			if (!that->writeDestructorBody(cl, code_deepness, ctx, ec))
				return false;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
			ctx->writeTabs(code_deepness) << cl->name() << " * _that;" << std::endl;
			ctx->writeTabs(code_deepness) << getScope(cl, false) << " *  _intf;" << std::endl;
			ctx->writeTabs(code_deepness) << "std::string __id;" << std::endl;
			return true;
		}

		template<class Class_T>
		bool writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << "struct " << getScope(cl) << cl->name() << "::_Priv" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;

				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					break;
				case Mode::Implementatinon:
					if (!writePriv(code_deepness, ctx, cl, ec))
						return false;
					break;
				}

				if (!that->writePrivateMembers(code_deepness, ctx, cl, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "struct _Priv;" << std::endl;
				ctx->writeTabs(code_deepness) << "_Priv * _priv;" << std::endl;
				break;
			case Mode::AllInOne:
				if (!that->writePrivateMembers(code_deepness, ctx, cl, ec))
					return false;
				break;
			}

			return true;
		}

		template<class Class_T>
		bool writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
				if (!writeConstructor(code_deepness, ctx, cl, ec) ||
				    !writeDestructor(code_deepness, ctx, cl, ec) ||
					!writeDefinitions(code_deepness, ctx, cl, ec))
					return false;
				break;
			case Role::Server:
				if (!that->writeInvoke(code_deepness, ctx, cl, ec) ||
				    !writeDestructor(code_deepness, ctx, cl, ec))
					return false;
				break;
			}

			return true;
		}

		template<class Class_T>
		bool writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
				if (!that->writeInvoke(code_deepness, ctx, cl, ec))
					return false;
				break;
			case Role::Server:
				if (!writeConstructor(code_deepness, ctx, cl, ec) ||
				    !writeDefinitions(code_deepness, ctx, cl, ec))
					return false;
				break;
			}

			return true;
		}

		template<class Class_T>
		bool writeDestructorBody(Class_T * cl, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
			case Mode::Implementatinon:
				break;
			}
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
		return priv->writeTopLevel(code_deepness, ctx, topLevel, ec);
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
		auto & o = ctx->writeTabs(code_deepness) << "#include ";
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
		return priv->writePrivateSection(code_deepness, ctx, intf, ec);
	}

	bool CPPCodeGen::writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		return priv->writePublicSection(code_deepness, ctx, intf, ec);
	}

	bool CPPCodeGen::writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		return priv->writeProtectedSection(code_deepness, ctx, intf, ec);
	}

	bool CPPCodeGen::writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		return priv->writePrivateSection(code_deepness, ctx, obj, ec);
	}

	bool CPPCodeGen::writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		if (!priv->writePublicSection(code_deepness, ctx, obj, ec))
			return false;
		switch (ctx->mode())	
		{
			case Mode::AllInOne:
				ctx->writeTabs(code_deepness) << "virtual std::string _id() const override" << std::endl;
				ctx->writeTabs(code_deepness) << "{ return __id; }" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "virtual std::string _id() const override;" << std::endl;
				break;
			case Mode::Implementatinon:
				ctx->writeTabs(code_deepness) << "std::string " << priv->getScope(obj) << obj->name() << "::_id() const" << std::endl;
				ctx->writeTabs(code_deepness) << "{ return _priv->__id; }" << std::endl << std::endl;
				break;
		}
		return true;
	}

	bool CPPCodeGen::writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		return priv->writeProtectedSection(code_deepness, ctx, obj, ec);
	}

	bool CPPCodeGen::writeDestructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writeDestructorBody(intf, code_deepness, ctx, ec);
	}

	bool CPPCodeGen::writeDestructorBody(Language::Object * obj, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->writeDestructorBody(obj, code_deepness, ctx, ec);
	}

	bool CPPCodeGen::writeType(Language::Type * type, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->addType(code_deepness, ctx, type, ec);
	}

}
