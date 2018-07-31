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
		Priv(std::ostream & o_, Mode mode_, Role role_) : 
			o(o_), 
			mode(mode_), 
			role(role_)
		{ }

		std::ostream & o;
		Mode mode; 
		Role role;
	};

	CPPCodeGenContext::CPPCodeGenContext(std::ostream & o, Mode mode, Role role) : priv(new Priv(o, mode, role))
	{ }

	CPPCodeGenContext::~CPPCodeGenContext()
	{
		delete priv;
	}

	std::ostream & CPPCodeGenContext::stream() const
	{
		return priv->o;
	}

	CPPCodeGenContext::Mode CPPCodeGenContext::mode() const
	{
		return priv->mode;
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

	std::string CPPBasicCodegenHelper::getName(const Language::TopLevel * e) const
	{
		return e->name();
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
		Priv(CPPCodeGen * that_) :
			that(that_)
		{ }

		CPPCodeGen * that;
		
		std::string getName(Language::TopLevel * t)
		{
			return that->helper()->getName(t);
		}

		std::ostream & writeTabs(short code_deepness, CPPCodeGenContext * ctx) const
		{
			char ch;
			for (short i = 0, l = code_deepness * that->helper()->tabDefinition(ch); i < l; ++i)
				*ctx << ch;
			return ctx->stream();
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
						auto & o = writeTabs(code_deepness, ctx) << "typedef ";
						if (!addType(code_deepness, ctx, type, ec))
							return false;
						o << " " << type_definition->name() << ";" << std::endl;
					}
				}
				break;
			case Mode::Implementatinon:
				break;
			}

			return true;
		}

		bool writeFunction(short code_deepness, CPPCodeGenContext * ctx, Language::Function * function, ErrorCollector & ec)
		{
			if (ctx->role() == Role::Server && ctx->mode() == Mode::Implementatinon)
				return true;

			auto & o = writeTabs(code_deepness, ctx);

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				switch (ctx->role())
				{
				case Role::Client:
					break;
				case Role::Server:
					o << "virtual ";
					break;
				}
				break;
			case Mode::Implementatinon:
				break;
			}

			if (!addType(code_deepness, ctx, function->returnType().get(), ec))
				return false;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				o << " " << function->name() << "(";
				break;
			case Mode::Implementatinon:
				o << " " << getScope(function) << function->name() << "(";
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

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Implementatinon:
				switch (ctx->role())
				{
				case Role::Client:
					o << std::endl;
					writeTabs(code_deepness++, ctx) << "{" << std::endl;

					if (!that->writeFunctionBody(function, code_deepness, ctx, ec))
						return false;

					writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
					break;
				case Role::Server:
					o << " = 0;" << std::endl;
					break;
				}
				break;
			case Mode::Declaration:
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

		bool writeProperty(short code_deepness, CPPCodeGenContext * ctx, Language::Property * property, ErrorCollector & ec)
		{
			if (ctx->role() == Role::Server && ctx->mode() == Mode::Implementatinon)
				return true;

		//getter
			auto & o = writeTabs(code_deepness, ctx);

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				switch (ctx->role())
				{
				case Role::Client:
					break;
				case Role::Server:
					o << "virtual ";
					break;
				}
				break;
			case Mode::Implementatinon:
				break;
			}

			if (!addType(code_deepness, ctx, property->type().get(), ec))
				return false;

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				o << " get_" << property->name() << "()";
				break;
			case Mode::Implementatinon:
				o << " " << getScope(property) << "get_" << property->name() << "()";
				break;
			}

			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Implementatinon:
				switch (ctx->role())
				{
				case Role::Client:
					o << std::endl;
					writeTabs(code_deepness++, ctx) << "{" << std::endl;

					if (!that->writePropertyGetterBody(property, code_deepness, ctx, ec))
						return false;

					writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
					break;
				case Role::Server:
					o << " = 0;" << std::endl;
					break;
				}
				break;
			case Mode::Declaration:
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

			//setter
			if(!property->readOnly())
			{
				auto & o = writeTabs(code_deepness, ctx);

				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					switch (ctx->role())
					{
					case Role::Client:
						break;
					case Role::Server:
						o << "virtual ";
						break;
					}
					break;
				case Mode::Implementatinon:
					break;
				}

				switch (ctx->mode())
				{
				case Mode::AllInOne:
				case Mode::Declaration:
					o << "void set_" << property->name() << "(const ";
					break;
				case Mode::Implementatinon:
					o << "void " << getScope(property) << "set_" << property->name() << "(const ";
					break;
				}

				if (!addType(code_deepness, ctx, property->type().get(), ec))
					return false;

				o << " & value)";

				switch (ctx->role())
				{
				case Role::Client:
					switch (ctx->mode())
					{
					case Mode::AllInOne:
					case Mode::Implementatinon:
						o << std::endl;
						writeTabs(code_deepness++, ctx) << "{" << std::endl;

						if (!that->writePropertySetterBody(property, code_deepness, ctx, ec))
							return false;

						writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
						break;
					case Mode::Declaration:
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
					break;
				case Role::Server:
					o << " = 0;" << std::endl;
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
				writeTabs(code_deepness, ctx) << "class " << obj->name() << " : public _Object" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (!that->writePrivateSection(code_deepness, ctx, obj, ec))
					return false;
				writeTabs(code_deepness - 1, ctx) << "public:" << std::endl;
				writeTabs(code_deepness, ctx) << "typedef std::shared_ptr<" << obj->name() << "> Ptr;" << std::endl;
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
				writeTabs(code_deepness - 1, ctx) << "protected:" << std::endl;
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
				writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
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
				writeTabs(code_deepness, ctx) << cl->name() << "()" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (!that->writeConstructorBody(cl, code_deepness, ctx, ec))
					return false;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				writeTabs(code_deepness, ctx) << cl->name() << "();" << std::endl;
				break;
			case Mode::Implementatinon:
				writeTabs(code_deepness, ctx) << getScope(cl) << cl->name() << "::" << cl->name() << "() : _priv(new _Priv(this))" << std::endl;
				writeTabs(code_deepness, ctx) << "{ }" << std::endl << std::endl;
				break;
			}

			return true;
		}

		bool writeConstructor(short code_deepness, CPPCodeGenContext * ctx, Language::Object * cl, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::AllInOne:
				writeTabs(code_deepness, ctx) << cl->name() << "(" << getScope(cl, false) << " * intf, const std::string & id)" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (!that->writeConstructorBody(cl, code_deepness, ctx, ec))
					return false;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				writeTabs(code_deepness, ctx) << cl->name() << "(" << getScope(cl, false) << " * intf, const std::string & id);" << std::endl;
				break;
			case Mode::Implementatinon:
				writeTabs(code_deepness, ctx) << getScope(cl) << cl->name() << "::" << cl->name() << "(" << getScope(cl, false) << " * intf, const std::string & id) : _priv(new _Priv(this, intf, id))" << std::endl;
				writeTabs(code_deepness, ctx) << "{ }" << std::endl << std::endl;
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
				writeTabs(code_deepness, ctx) << "virtual ~" << cl->name() << "()" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (!that->writeDestructorBody(cl, code_deepness, ctx, ec))
					return false;
				writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				writeTabs(code_deepness, ctx) << "virtual ~" << cl->name() << "();" << std::endl;
				break;
			case Mode::Implementatinon:
				writeTabs(code_deepness, ctx) << getScope(cl) << cl->name() << "::~" << cl->name() << "()" << std::endl;
				writeTabs(code_deepness, ctx) << "{ delete _priv; }" << std::endl << std::endl;
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
				if (!writeFunction(code_deepness, ctx, dynamic_cast<Language::Method*>(definition), ec))
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
				if (!writeFunction(code_deepness, ctx, dynamic_cast<Language::Function*>(definition), ec))
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
				else if (dynamic_cast<Language::Function*>(definition.get()))
				{
					if (!writeFunction(code_deepness, ctx, dynamic_cast<Language::Function*>(definition.get()), ec))
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
				else if (dynamic_cast<Language::Method*>(definition.get()))
				{
					if (!writeFunction(code_deepness, ctx, dynamic_cast<Language::Method*>(definition.get()), ec))
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
				writeTabs(code_deepness, ctx) << "class " << that->helper()->getName(intf) << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;
				if (!that->writePrivateSection(code_deepness, ctx, intf, ec))
					return false;
				writeTabs(code_deepness - 1, ctx) << "public:" << std::endl;
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
				writeTabs(code_deepness - 1, ctx) << "protected:" << std::endl;
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
				writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
				break;
			case Mode::Implementatinon:
				break;
			}

			return true;
		}

		bool writeModule(short code_deepness, CPPCodeGenContext * ctx, Language::Module * module, ErrorCollector & ec)
		{
			auto & o = writeTabs(code_deepness++, ctx) << "namespace " << that->helper()->getName(module) << " {" << std::endl;
			for (auto & element : module->elements())
			{
				o << std::endl;
				if (!writeTopLevel(code_deepness, ctx, element.get(), ec))
					return false;
			}
			writeTabs(--code_deepness, ctx) << "}" << std::endl;
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
			writeTabs(code_deepness, ctx) << "_Priv(" << cl->name() << " * _that_): _that(_that_)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			if (!that->writeConstructorBody(cl, code_deepness, ctx, ec))
				return false;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "~_Priv()" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			if (!that->writeDestructorBody(cl, code_deepness, ctx, ec))
				return false;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << cl->name() << " * _that;" << std::endl << std::endl;

			return true;
		}

		bool writePriv(short code_deepness, CPPCodeGenContext * ctx, Language::Object * cl, ErrorCollector & ec)
		{
			writeTabs(code_deepness, ctx) << "_Priv(" << cl->name() << " * _that_, " << getScope(cl, false) << " * _intf_, const std::string & _id_): _that(_that_), _intf(_intf_), __id(_id_)" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			if (!that->writeConstructorBody(cl, code_deepness, ctx, ec))
				return false;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;

			writeTabs(code_deepness, ctx) << "~_Priv()" << std::endl;
			writeTabs(code_deepness++, ctx) << "{" << std::endl;
			if (!that->writeDestructorBody(cl, code_deepness, ctx, ec))
				return false;
			writeTabs(--code_deepness, ctx) << "}" << std::endl << std::endl;
			writeTabs(code_deepness, ctx) << cl->name() << " * _that;" << std::endl;
			writeTabs(code_deepness, ctx) << getScope(cl, false) << " *  _intf;" << std::endl;
			writeTabs(code_deepness, ctx) << "std::string __id;" << std::endl;
			return true;
		}

		template<class Class_T>
		bool writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Class_T * cl, ErrorCollector & ec)
		{
			switch (ctx->mode())
			{
			case Mode::Implementatinon:
				writeTabs(code_deepness, ctx) << "struct " << getScope(cl) << cl->name() << "::_Priv" << std::endl;
				writeTabs(code_deepness++, ctx) << "{" << std::endl;

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
				writeTabs(--code_deepness, ctx) << "};" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				writeTabs(code_deepness, ctx) << "struct _Priv;" << std::endl;
				writeTabs(code_deepness, ctx) << "_Priv * _priv;" << std::endl;
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
			switch (ctx->mode())
			{
			case Mode::AllInOne:
			case Mode::Declaration:
				if (dynamic_cast<Language::Interface*>(cl) && !that->writeAliases(code_deepness, ctx, ec))
					return false;
				break;
			case Mode::Implementatinon:
				break;
			}

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
				priv->writeTabs(code_deepness, ctx) << "virtual std::string _id() const override" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "{ return __id; }" << std::endl << std::endl;
				break;
			case Mode::Declaration:
				priv->writeTabs(code_deepness, ctx) << "virtual std::string _id() const override;" << std::endl;
				break;
			case Mode::Implementatinon:
				priv->writeTabs(code_deepness, ctx) << "std::string " << priv->getScope(obj) << obj->name() << "::_id() const" << std::endl;
				priv->writeTabs(code_deepness, ctx) << "{ return _priv->__id; }" << std::endl << std::endl;
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
