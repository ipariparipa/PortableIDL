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

    std::vector<Include> CPPVoidLogging::includes() const { return std::vector<Include>(); }
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

    std::vector<Include> CPPBasicCodeGenHelper::includes() const
	{
		return priv->customIncludes;
	}

    Include CPPBasicCodeGenHelper::coreIncludePath() const
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
        std::string getScope(const T * t, const std::string & delim, bool isComplete = true)
		{
			std::string ret;
			bool is_first = true;
			for (auto & sc : t->scope())
			{
				if (is_first)
					is_first = false;
				else
                    ret += delim;
				ret += sc;
			}
			if (isComplete && !is_first)
                ret += delim;
			return ret;
		}

        template<class T>
        std::string getScope(const T * t, bool isComplete = true)
        {
            return getScope(t, "::", isComplete);
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
            *ctx << "ptr<" << getScope(obj) << obj->name() << ">";
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

		bool writeFunction(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::FunctionVariant * function, ErrorCollector & ec)
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

					if (!that->writeFunctionBody(intf, function, code_deepness, ctx, ec))
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

		bool writeProperty(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Property * property, ErrorCollector & ec)
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

					if (!that->writePropertyGetterBody(intf, property, code_deepness, ctx, ec))
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

						if (!that->writePropertySetterBody(intf, property, code_deepness, ctx, ec))
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

		bool writeObject(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!writeDocumentation(code_deepness, ctx, CStyleDocumentation::Before, obj, ec))
					return false;
				ctx->writeTabs(code_deepness) << "class " << obj->name() << " : public _Object" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writePrivateSection(intf, code_deepness, ctx, obj, ec))
					return false;
				ctx->writeTabs(code_deepness - 1) << "public:" << std::endl;
               ctx->writeTabs(code_deepness) << "typedef ptr<" << obj->name() << "> Ptr;" << std::endl;
				break;
			case Mode::Implementatinon:
				if (!that->writePrivateSection(intf, code_deepness, ctx, obj, ec))
					return false;
				break;
			}

			if (!that->writePublicSection(intf, code_deepness, ctx, obj, ec))
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

			if (!that->writeProtectedSection(intf, code_deepness, ctx, obj, ec))
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

		bool writeConstructor(Language::Interface *, short code_deepness, CPPCodeGenContext * ctx, Language::Interface * cl, ErrorCollector & ec)
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

		bool writeConstructor(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * cl, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Server:
				switch (ctx->mode())
				{
				case Mode::AllInOne:
					ctx->writeTabs(code_deepness) << cl->name() << "(" << getScope(cl, false) << " * intf)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					if (!that->writeConstructorBody(intf, cl, code_deepness, ctx, ec))
						return false;
					ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
					break;
				case Mode::Declaration:
					ctx->writeTabs(code_deepness) << cl->name() << "(" << getScope(cl, false) << " * intf);" << std::endl;
					break;
				case Mode::Implementatinon:
					ctx->writeTabs(code_deepness) << getScope(cl) << cl->name() << "::" << cl->name() << "(" << getScope(cl, false) << " * intf) : _priv(new _Priv(this, intf))" << std::endl;
					ctx->writeTabs(code_deepness) << "{ }" << std::endl << std::endl;
					break;
				}
				break;
			case Role::Client:
				switch (ctx->mode())
				{
				case Mode::AllInOne:
					ctx->writeTabs(code_deepness) << cl->name() << "(" << getScope(cl, false) << " * intf, const std::string & id)" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					if (!that->writeConstructorBody(intf, cl, code_deepness, ctx, ec))
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
				break;
			}

			return true;
		}

		bool _writeDestructorBody(Language::Interface * intf, Language::Object * obj, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
		{
			return that->writeDestructorBody(intf, obj, code_deepness, ctx, ec);
		}

		bool _writeDestructorBody(Language::Interface *, Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
		{
			return that->writeDestructorBody(intf, code_deepness, ctx, ec);
		}

		template<class Class_T>
		bool writeDestructor(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
				ctx->writeTabs(code_deepness) << "virtual ~" << cl->name() << "()" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!_writeDestructorBody(intf, cl, code_deepness, ctx, ec))
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
		
		bool writeObjectDefinition(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Definition * definition, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::Property*>(definition))
			{
				if (!writeProperty(intf, code_deepness, ctx, dynamic_cast<Language::Property*>(definition), ec))
					return false;
			}
			else if (dynamic_cast<Language::Method*>(definition))
			{
				for (auto &v : dynamic_cast<Language::Method*>(definition)->variants())
				if (!writeFunction(intf, code_deepness, ctx, v.second.get(), ec))
					return false;
			}
			return true;
		}

		bool writeDefinition(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Definition * definition, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::TypeDefinition*>(definition))
			{
				if (!writeTypeDefinition(code_deepness, ctx, dynamic_cast<Language::TypeDefinition*>(definition), ec))
					return false;
			}
			else if (dynamic_cast<Language::Function*>(definition))
			{
				for (auto &v : dynamic_cast<Language::Function*>(definition)->variants())
					if (!writeFunction(intf, code_deepness, ctx, v.second.get(), ec))
						return false;
			}
			else if (dynamic_cast<Language::Object*>(definition))
			{
				if (!writeObject(intf, code_deepness, ctx, dynamic_cast<Language::Object*>(definition), ec))
					return false;
			}
			return true;
		}

		bool writeDefinitions(Language::Interface *, short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (!that->writeObjectBase(intf, code_deepness, ctx, ec))
					return false;

                //write predeclarations of objects
                for (auto & definition : intf->definitions())
                    if (auto o = dynamic_cast<Language::Object*>(definition.get()))
                        ctx->writeTabs(code_deepness) << "class " << o->name() << ";" << std::endl;

                break;
			case Mode::Implementatinon:
				break;
			}

            for (auto & definition : intf->definitions())
                if (auto o = dynamic_cast<Language::TypeDefinition*>(definition.get()))
				{
                    if (!writeTypeDefinition(code_deepness, ctx, o, ec))
						return false;
				}
                else if (auto o = dynamic_cast<Language::FunctionVariant*>(definition.get()))
				{
                    if (!writeFunction(intf, code_deepness, ctx, o, ec))
						return false;
				}
                else if (auto o = dynamic_cast<Language::Object*>(definition.get()))
				{
                    if (!writeObject(intf, code_deepness, ctx, o, ec))
						return false;
				}

			return true;
		}

		bool writeDefinitions(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
		{
			for (auto & definition : obj->definitions())
                if (auto o = dynamic_cast<Language::Property*>(definition.get()))
				{
                    if (!writeProperty(intf, code_deepness, ctx, o, ec))
						return false;
				}
                else if (auto o = dynamic_cast<Language::MethodVariant*>(definition.get()))
				{
                    if (!writeFunction(intf, code_deepness, ctx, o, ec))
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

        bool writePriv(Language::Interface *intf, short code_deepness, CPPCodeGenContext * ctx, Language::Interface * cl, ErrorCollector & ec)
		{
			ctx->writeTabs(code_deepness) << "_Priv(" << cl->name() << " * _that_): _that(_that_)" << std::endl;

            if(helper()->logging() && helper()->logging()->loggerType().length())
                ctx->writeTabs(code_deepness + 2) << ", _logger("<< helper()->logging()->initLogger("\"" + intf->loggerName() + "\"") <<")" << std::endl;

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

            if(helper()->logging() && helper()->logging()->loggerType().length())
                ctx->writeTabs(code_deepness) << helper()->logging()->loggerType() << " _logger;" << std::endl << std::endl;

			return true;
		}

		bool writePriv(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
				ctx->writeTabs(code_deepness) << "_Priv(" << obj->name() << " * _that_, " << getScope(obj, false) << " * _intf_, const std::string & _data_): _that(_that_), _intf(_intf_), __data(_data_)" << std::endl;

                if(helper()->logging() && helper()->logging()->loggerType().length())
                    ctx->writeTabs(code_deepness + 2) << ", _logger("<< helper()->logging()->initLogger("\"" + obj->loggerName() + "\"") <<")" << std::endl;

                ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writeConstructorBody(intf, obj, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "~_Priv()" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writeDestructorBody(intf, obj, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				ctx->writeTabs(code_deepness) << obj->name() << " * _that;" << std::endl;
				ctx->writeTabs(code_deepness) << getScope(obj, false) << " *  _intf;" << std::endl;
				ctx->writeTabs(code_deepness) << "std::string __data;" << std::endl;

                if(helper()->logging() && helper()->logging()->loggerType().length())
                    ctx->writeTabs(code_deepness) << helper()->logging()->loggerType() << " _logger;" << std::endl << std::endl;
                break;
			case Role::Server:
				ctx->writeTabs(code_deepness) << "_Priv(" << obj->name() << " * _that_, " << getScope(obj, false) << " * _intf_): _that(_that_), _intf(_intf_)" << std::endl;

                if(helper()->logging() && helper()->logging()->loggerType().length())
                    ctx->writeTabs(code_deepness + 2) << ", _logger("<< helper()->logging()->initLogger("\"" + obj->loggerName() + "\"") <<")" << std::endl;

                ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writeConstructorBody(intf, obj, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "~_Priv()" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writeDestructorBody(intf, obj, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;
				ctx->writeTabs(code_deepness) << obj->name() << " * _that;" << std::endl;
				ctx->writeTabs(code_deepness) << getScope(obj, false) << " *  _intf;" << std::endl;

                if(helper()->logging() && helper()->logging()->loggerType().length())
                    ctx->writeTabs(code_deepness) << helper()->logging()->loggerType() << " _logger;" << std::endl << std::endl;
                break;
			}

			return true;
		}

		bool _writePrivateMembers(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec)
		{
			return that->writePrivateMembers(intf, code_deepness, ctx, object, ec);
		}

		bool _writePrivateMembers(Language::Interface * , short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			return that->writePrivateMembers(code_deepness, ctx, intf, ec);
		}

		template<class Class_T>
		bool writePrivateSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
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
					if (!writePriv(intf, code_deepness, ctx, cl, ec))
						return false;
					break;
				}

				if (!_writePrivateMembers(intf, code_deepness, ctx, cl, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "};" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				ctx->writeTabs(code_deepness) << "struct _Priv;" << std::endl;
				ctx->writeTabs(code_deepness) << "_Priv * _priv;" << std::endl;
				break;
			case Mode::AllInOne:
				if (!_writePrivateMembers(intf, code_deepness, ctx, cl, ec))
					return false;
				break;
			}

			return true;
		}

		bool _writeInvoke(Language::Interface *, short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			return that->writeInvoke(code_deepness, ctx, intf, ec);
		}

		bool _writeInvoke(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
		{
			return that->writeInvoke(intf, code_deepness, ctx, obj, ec);
		}

		template<class Class_T>
		bool writePublicSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
				if (!writeConstructor(intf, code_deepness, ctx, cl, ec) ||
				    !writeDestructor(intf, code_deepness, ctx, cl, ec) ||
					!writeDefinitions(intf, code_deepness, ctx, cl, ec))
					return false;
				break;
			case Role::Server:
				if (!_writeInvoke(intf, code_deepness, ctx, cl, ec) ||
				    !writeDestructor(intf, code_deepness, ctx, cl, ec))
					return false;
				break;
			}

			return true;
		}

		template<class Class_T>
		bool writeProtectedSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
				if (!_writeInvoke(intf, code_deepness, ctx, cl, ec))
					return false;
				break;
			case Role::Server:
				if (!writeConstructor(intf, code_deepness, ctx, cl, ec) ||
				    !writeDefinitions(intf, code_deepness, ctx, cl, ec))
					return false;
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

	//virtual 
	CPPCodeGenContext * CPPCodeGen::createContext(short tab_length, char tab_char, std::ostream & o, Role role, Mode mode) const
	{
		return new CPPCodeGenContext(tab_length, tab_char, o, role, mode);
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

        if(auto logging = helper()->logging())
        {
            for (auto & include : logging->includes())
            {
                if (!writeInclude(code_deepness, ctx, include, ec))
                    return false;
            }
        }

		return writeIncludes(code_deepness, ctx, ec);
	}

    bool CPPCodeGen::writeInclude(short code_deepness, CPPCodeGenContext * ctx, const Include & include, ErrorCollector & ec)
	{
		auto & o = ctx->writeTabs(code_deepness) << "#include ";
		switch (include.first)
		{
        case IncludeType::GLobal:
			o << "<" << include.second << ">";
			break;
        case IncludeType::Local:
			o << "\"" << include.second << "\"";
			break;
		}
		o << std::endl;

		return true;
	}

	bool CPPCodeGen::writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		return priv->writePrivateSection(intf, code_deepness, ctx, intf, ec);
	}

	bool CPPCodeGen::writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		return priv->writePublicSection(intf, code_deepness, ctx, intf, ec);
	}

	bool CPPCodeGen::writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
	{
		return priv->writeProtectedSection(intf, code_deepness, ctx, intf, ec);
	}

	bool CPPCodeGen::writePrivateSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		return priv->writePrivateSection(intf, code_deepness, ctx, obj, ec);
	}

	bool CPPCodeGen::writePublicSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		if (!priv->writePublicSection(intf, code_deepness, ctx, obj, ec))
			return false;
		return true;
	}

	bool CPPCodeGen::writeProtectedSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec)
	{
		return priv->writeProtectedSection(intf, code_deepness, ctx, obj, ec);
	}

	bool CPPCodeGen::writeDestructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return true;
	}

	bool CPPCodeGen::writeDestructorBody(Language::Interface * intf, Language::Object * obj, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return true;
	}

	bool CPPCodeGen::writeType(Language::Type * type, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec)
	{
		return priv->addType(code_deepness, ctx, type, ec);
	}

}
