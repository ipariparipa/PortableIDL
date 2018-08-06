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
#include <sstream>
#include <algorithm>

namespace PIDL {

	struct CSCodeGenContext::Priv
	{
		Priv(short tab_length_, char tab_char_, std::ostream & o_, Role role_) :
			o(o_),
			role(role_),
			tab_length(tab_length_),
			tab_char(tab_char_)
		{ }

		std::ostream & o;
		Role role;

		short tab_length;
		char tab_char;
	};

	CSCodeGenContext::CSCodeGenContext(short tab_length, char tab_char, std::ostream & o, Role role) : 
		priv(new Priv(tab_length, tab_char, o, role))
	{ }

	CSCodeGenContext::~CSCodeGenContext()
	{
		delete priv;
	}

	std::ostream & CSCodeGenContext::operator * () const
	{
		return priv->o;
	}

	std::ostream & CSCodeGenContext::stream() const
	{
		return priv->o;
	}

	CSCodeGenContext::Role CSCodeGenContext::role() const
	{
		return priv->role;
	}

	std::ostream & CSCodeGenContext::writeTabs(short code_deepness)
	{
		for (short i = 0, l = code_deepness * priv->tab_length; i < l; ++i)
			priv->o << priv->tab_char;
		return priv->o;
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


	//struct CSCodeGenDocumentation::Priv { };
	CSCodeGenDocumentation::CSCodeGenDocumentation() : priv(nullptr) { }
	CSCodeGenDocumentation::~CSCodeGenDocumentation() = default;


	//struct CSVoidDocumentation::Priv { };
	CSVoidDocumentation::CSVoidDocumentation() : priv(nullptr) { }
	CSVoidDocumentation::~CSVoidDocumentation() = default;

	bool CSVoidDocumentation::write(short code_deepness, CSCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
		return true;
	}


	//struct CPPBasicDocumentation::Priv { };
	CSBasicDocumentation::CSBasicDocumentation() : priv(nullptr) { }
	CSBasicDocumentation::~CSBasicDocumentation() = default;

	bool CSBasicDocumentation::write(short code_deepness, CSCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
		auto & doc = docprov->documentation();

		if (!doc.brief.length() && !doc.details.size())
			return true;

		auto split = [](const std::string & src, char delimeter) -> std::list<std::string>
		{
			std::stringstream ss(src);
			std::string item;
			std::list<std::string> splittedStrings;
			while (std::getline(ss, item, delimeter))
				splittedStrings.push_back(item);
			return splittedStrings;
		};

		auto join = [](const std::list<std::string> & src, char delimeter) -> std::string
		{
			std::stringstream ss;
			bool is_first = true;
			for (auto & s : src)
			{
				if (is_first)
					is_first = false;
				else if (delimeter)
					ss << delimeter;
				ss << s;
			}
			return ss.str();
		};

		auto writeLines = [&](const std::string & title, const std::list<std::string> & lines)
		{
			if (title.length())
				ctx->writeTabs(code_deepness) << " * " << title << std::endl;
			for (auto & l : lines)
				ctx->writeTabs(code_deepness) << " * " << join(split(l, '\r'), '\0') << std::endl;
		};

		switch (place)
		{
		case Place::Before:
			if (!dynamic_cast<Language::Function::Argument*>(docprov))
			{
				ctx->writeTabs(code_deepness) << std::endl << "/*" << std::endl;

				if (doc.brief.size())
					writeLines(std::string(), split(doc.brief, '\n'));

				if (doc.details.count(Language::DocumentationProvider::Documentation::Description))
					writeLines("Description:", split(docprov->documentation().details.at(Language::DocumentationProvider::Documentation::Description), '\n'));

				if (dynamic_cast<Language::Function*>(docprov) &&
					doc.details.count(Language::DocumentationProvider::Documentation::Return))
					writeLines("Return:", split(docprov->documentation().details.at(Language::DocumentationProvider::Documentation::Description), '\n'));

				ctx->writeTabs(code_deepness) << " */" << std::endl;
			}
			break;
		case Place::After:
			if (dynamic_cast<Language::Function::Argument*>(docprov))
			{
				*ctx << " // ";
				if (doc.details.count(doc.ArgDirection))
					*ctx << "[" << doc.details.at(doc.ArgDirection) << "] ";
				*ctx << doc.brief << std::endl;
				ctx->writeTabs(code_deepness + 1);
			}
			break;
		}

		return true;
	}

	//struct CSdotNetDocumentation::Priv { };
	CSdotNetDocumentation::CSdotNetDocumentation() : priv(nullptr) { }
	CSdotNetDocumentation::~CSdotNetDocumentation() = default;

	bool CSdotNetDocumentation::write(short code_deepness, CSCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
		auto & doc = docprov->documentation();

		if (!doc.brief.length() && !doc.details.size())
			return true;

		auto split = [](const std::string & src, char delimeter) -> std::list<std::string>
		{
			std::stringstream ss(src);
			std::string item;
			std::list<std::string> splittedStrings;
			while (std::getline(ss, item, delimeter))
				splittedStrings.push_back(item);
			return splittedStrings;
		};

		auto join = [](const std::list<std::string> & src, char delimeter) -> std::string
		{
			std::stringstream ss;
			bool is_first = true;
			for (auto & s : src)
			{
				if (is_first)
					is_first = false;
				else if (delimeter)
					ss << delimeter;
				ss << s;
			}
			return ss.str();
		};

		auto writeLines = [&](const std::list<std::string> & lines)
		{
			for (auto & l : lines)
				ctx->writeTabs(code_deepness) << "/// " << join(split(l, '\r'), '\0') << std::endl;
		};

		switch (place)
		{
		case Place::Before:
			if (!dynamic_cast<Language::Function::Argument*>(docprov))
			{
				**ctx << std::endl;

				if (doc.brief.length())
				{
					ctx->writeTabs(code_deepness) << "/// <summary>" << std::endl;
					writeLines(split(doc.brief, '\n'));
					ctx->writeTabs(code_deepness) << "/// </summary>" << std::endl;
				}

				if (doc.details.count(Language::DocumentationProvider::Documentation::Description))
				{
					ctx->writeTabs(code_deepness) << "/// <remarks>" << std::endl;
					writeLines(split(docprov->documentation().details.at(Language::DocumentationProvider::Documentation::Description), '\n'));
					ctx->writeTabs(code_deepness) << "/// </remarks>" << std::endl;
				}

				if (dynamic_cast<Language::Function*>(docprov))
				{
					if (doc.details.count(Language::DocumentationProvider::Documentation::Return))
					{
						ctx->writeTabs(code_deepness) << "/// <returns>" << std::endl;
						writeLines(split(docprov->documentation().details.at(Language::DocumentationProvider::Documentation::Description), '\n'));
						ctx->writeTabs(code_deepness) << "/// </returns>" << std::endl;
					}

					for (auto & arg : dynamic_cast<Language::Function*>(docprov)->arguments())
					{
						if ((long)arg->documentation().brief.find("\n") == -1)
						{
							ctx->writeTabs(code_deepness) << "/// <param name=\"" << arg->name() << "\">";
							*ctx << arg->documentation().brief;
							*ctx << "</param>" << std::endl;
						}
						else
						{
							ctx->writeTabs(code_deepness) << "/// <param name=\"" << arg->name() << "\">" << std::endl;
							writeLines(split(arg->documentation().brief, '\n'));
							ctx->writeTabs(code_deepness) << "/// </param>" << std::endl;
						}
					}
				}

			}
			break;
		case Place::After:
			break;
		}

		return true;
	}

	CSCodeGenHelper::CSCodeGenHelper() : priv(nullptr)
	{ }

	CSCodeGenHelper::~CSCodeGenHelper() = default;


	struct CSBasicCodeGenHelper::Priv
	{
		Priv() :
			logging(std::make_shared<CSVoidLogging>()),
			documentation(std::make_shared<CSVoidDocumentation>())
		{ }

		std::shared_ptr<CSCodeGenLogging> logging;
		std::shared_ptr<CSCodeGenDocumentation> documentation;
	};

	CSBasicCodeGenHelper::CSBasicCodeGenHelper() : CSCodeGenHelper(), priv(new Priv())
	{ }

	CSBasicCodeGenHelper::~CSBasicCodeGenHelper()
	{
		delete priv;
	}

	std::string CSBasicCodeGenHelper::getName(const Language::TopLevel * t) const
	{
		return t->name();
	}

	std::shared_ptr<CSCodeGenLogging> CSBasicCodeGenHelper::logging() const
	{
		return priv->logging;
	}

	std::shared_ptr<CSCodeGenDocumentation> CSBasicCodeGenHelper::documentation() const
	{
		return priv->documentation;
	}

	struct CSCodeGen::Priv
	{
		Priv(CSCodeGen * that_) :
			that(that_)
		{ }

		CSCodeGen * that;

		CSCodeGenHelper * helper() const
		{
			return that->helper();
		}

		bool writeDocumentation(short code_deepness, CSCodeGenContext * ctx, CSCodeGenDocumentation::Place place, Language::DocumentationProvider *docprov, ErrorCollector & ec)
		{
			return that->helper()->documentation()->write(code_deepness, ctx, place, docprov, ec);
		}

		bool writeStructureBody(short code_deepness, CSCodeGenContext * ctx, Language::Structure * structure, ErrorCollector & ec)
		{
			auto & o = ctx->stream();
			for (auto & member : structure->members())
			{
				if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, member.get(), ec))
					return false;
				ctx->writeTabs(code_deepness) << "public ";
				if (!addType(code_deepness, ctx, member->type().get(), ec))
					return false;
				o << " " << member->name() << ";";
				if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::After, member.get(), ec))
					return false;
				o << std::endl;
			}

			return true;
		}

		bool writeStructure(short code_deepness, CSCodeGenContext * ctx, Language::TypeDefinition * structure, ErrorCollector & ec)
		{
			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, structure, ec))
				return false;
			switch (ctx->role())
			{
			case CSCodeGenContext::Role::Client:
				 ctx->writeTabs(code_deepness) << "public ";
				 break;
			case CSCodeGenContext::Role::Server:
				ctx->writeTabs(code_deepness) << "protected ";
				break;
			}
			*ctx << "struct " << structure->name() << std::endl;

			ctx->writeTabs(code_deepness++) << "{" << std::endl;

			auto struct_type = dynamic_cast<Language::Structure*>(structure->type()->finalType().get());
			assert(struct_type);

			if (!writeStructureBody(code_deepness, ctx, struct_type, ec))
				return false;

			ctx->writeTabs(--code_deepness) << "}" << std::endl;
			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::After, structure, ec))
				return false;
			**ctx << std::endl;

			return true;
		}

		bool addStructure(short code_deepness, CSCodeGenContext * ctx, Language::Structure * structure, ErrorCollector & ec)
		{
			*ctx << "struct " << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;

			if (!writeStructureBody(code_deepness, ctx, structure, ec))
				return false;

			ctx->writeTabs(--code_deepness) << "}";

			return true;
		}

		bool addGeneric(short code_deepness, CSCodeGenContext * ctx, Language::Generic * generic, ErrorCollector & ec)
		{
			auto fn_type = generic->type()->finalType().get();
			if (dynamic_cast<Language::Nullable*>(generic))
			{
				if (dynamic_cast<Language::NativeType*>(fn_type) ||
					dynamic_cast<Language::DateTime*>(fn_type) ||
					dynamic_cast<Language::Structure*>(fn_type))
				{
					*ctx << "Nullable<";
					if (!addType(code_deepness, ctx, generic->type().get(), ec))
						return false;
					*ctx << ">";
				}
				else
				{
					*ctx << "/*nullable*/ ";
					if (!addType(code_deepness, ctx, generic->type().get(), ec))
						return false;
				}
			}
			else if (dynamic_cast<Language::Array*>(generic))
			{
				if (!addType(code_deepness, ctx, generic->type().get(), ec))
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
			if (dynamic_cast<Language::Generic*>(type_))
				return addGeneric(code_deepness, ctx, dynamic_cast<Language::Generic*>(type_), ec);

			Language::Type * type = dynamic_cast<Language::Structure*>(type_->finalType().get()) ? type_ : type_->finalType().get();
			if (dynamic_cast<Language::NativeType*>(type))
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

		bool writeTypeDefinition(short code_deepness, CSCodeGenContext * ctx, Language::TypeDefinition * type_definition, ErrorCollector & ec)
		{
			auto type = type_definition->type()->finalType().get();
			if (dynamic_cast<Language::Structure*>(type))
			{
				if (!writeStructure(code_deepness, ctx, type_definition, ec))
					return false;
			}
			else
			{
				//type definitions are not really supoported by C# :-(
			}

			return true;
		}

		bool writeFunction(short code_deepness, CSCodeGenContext * ctx, Language::Function * function, ErrorCollector & ec)
		{
			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, function, ec))
				return false;

			switch (ctx->role())
			{
			case CSCodeGenContext::Role::Client:
				ctx->writeTabs(code_deepness) << "public " ;
				break;
			case CSCodeGenContext::Role::Server:
				ctx->writeTabs(code_deepness) << "protected abstract ";
				break;
			}

			if (!addType(code_deepness, ctx, function->returnType().get(), ec))
				return false;

			*ctx << " " << function->name() << "(";

			bool is_first_arg = true;
			for (auto & arg : function->arguments())
			{
				if (!is_first_arg)
					*ctx << ", ";
				is_first_arg = false;

				if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, arg.get(), ec))
					return false;

				switch (arg->direction())
				{
				case Language::Function::Argument::Direction::In:
					addType(code_deepness, ctx, arg->type().get(), ec);
					*ctx << " " << arg->name();
					break;
				case Language::Function::Argument::Direction::InOut:
					*ctx << "ref ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					*ctx << " " << arg->name();
					break;
				case Language::Function::Argument::Direction::Out:
					*ctx << "out ";
					addType(code_deepness, ctx, arg->type().get(), ec);
					*ctx << " " << arg->name();
					break;
				}

				if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::After, arg.get(), ec))
					return false;
			}
			*ctx << ")";

			switch (ctx->role())
			{
			case CSCodeGenContext::Role::Client:
				**ctx << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;

				if (!that->writeFunctionBody(function, code_deepness, ctx, ec))
					return false;

				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				break;
			case CSCodeGenContext::Role::Server:
				*ctx << ";";
				break;
			}

			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::After, function, ec))
				return false;

			**ctx << std::endl;

			return true;
		}

		bool writeProperty(short code_deepness, CSCodeGenContext * ctx, Language::Property * property, ErrorCollector & ec)
		{
			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, property, ec))
				return false;

			switch (ctx->role())
			{
			case CSCodeGenContext::Role::Client:
				ctx->writeTabs(code_deepness) << "public ";
				if (!addType(code_deepness, ctx, property->type().get(), ec))
					return false;
				*ctx << " " << property->name() << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "get" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writePropertyGetterBody(property, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				if (!property->readOnly())
				{
					ctx->writeTabs(code_deepness) << "set" << std::endl;
					ctx->writeTabs(code_deepness++) << "{" << std::endl;
					if (!that->writePropertySetterBody(property, code_deepness, ctx, ec))
						return false;
					ctx->writeTabs(--code_deepness) << "}" << std::endl;
				}
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				break;
			case CSCodeGenContext::Role::Server:
				ctx->writeTabs(code_deepness) << "protected abstract ";
				if (!addType(code_deepness, ctx, property->type().get(), ec))
					return false;
				*ctx << " " << property->name() << " { get; ";
				if (!property->readOnly())
					*ctx << "set;";
				*ctx << "}";
				break;
			}

			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::After, property, ec))
				return false;

			**ctx << std::endl;

			return true;
		}
		bool writeObject(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, Language::Object * object, ErrorCollector & ec)
		{
			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, intf, ec))
				return false;
			switch (ctx->role())
			{
			case Role::Client:
				ctx->writeTabs(code_deepness) << "public class " << object->name() << " : _IObject" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << intf->name() << " _intf;" << std::endl;

				ctx->writeTabs(code_deepness) << "public string _id { get; private set; }" << std::endl;

				if (!that->writeMembers(code_deepness, ctx, object, ec))
					return false;

				ctx->writeTabs(code_deepness) << "public " << object->name() << "(" << intf->name() << " intf, string id)" << std::endl;

				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf = intf;" << std::endl;
				ctx->writeTabs(code_deepness) << "_id = id;" << std::endl;
				if (!that->writeConstructorBody(object, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				ctx->writeTabs(code_deepness) << "~" << object->name() << "()" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				if (!that->writeDestructorBody(object, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				if (!writeDefinitions(code_deepness, ctx, object, ec))
					return false;

				if (!that->writeInvoke(code_deepness, ctx, object, ec))
					return false;

				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				break;
			case Role::Server:
				ctx->writeTabs(code_deepness) << "public abstract class " << object->name() << " : _IObject" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << intf->name() << " _intf;" << std::endl;
				ctx->writeTabs(code_deepness) << "public string _id { get; private set; }" << std::endl;

				if (!that->writeMembers(code_deepness, ctx, object, ec))
					return false;

				ctx->writeTabs(code_deepness) << "public " << object->name() << "(" << intf->name() << " intf, string id)" << std::endl;

				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "_intf = intf;" << std::endl;
				ctx->writeTabs(code_deepness) << "_id = id;" << std::endl;
				if (!that->writeConstructorBody(object, code_deepness, ctx, ec))
					return false;
				ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

				if (!writeDefinitions(code_deepness, ctx, object, ec))
					return false;

				if (!that->writeInvoke(code_deepness, ctx, object, ec))
					return false;

				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				break;
			}

			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::After, intf, ec))
				return false;

			**ctx << std::endl;

			return true;
		}

		bool writeDefinition(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, Language::Definition * definition, ErrorCollector & ec)
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
				if (!writeObject(code_deepness, ctx, intf, dynamic_cast<Language::Object*>(definition), ec))
					return false;
			}
			return true;
		}

		bool writeDefinition(short code_deepness, CSCodeGenContext * ctx, Language::Object * obj, Language::Definition * definition, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::Method*>(definition))
			{
				if (!writeFunction(code_deepness, ctx, dynamic_cast<Language::Function*>(definition), ec))
					return false;
			}
			else if (dynamic_cast<Language::Property*>(definition))
			{
				if (!writeProperty(code_deepness, ctx, dynamic_cast<Language::Property*>(definition), ec))
					return false;
			}
			return true;
		}

		bool writeDefinitions(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			switch (ctx->role())
			{
			case Role::Client:
				ctx->writeTabs(code_deepness) << "public interface _IObject" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "string _id { get; }" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
				break;
			case Role::Server:
				ctx->writeTabs(code_deepness) << "protected interface _IObject" << std::endl;
				ctx->writeTabs(code_deepness++) << "{" << std::endl;
				ctx->writeTabs(code_deepness) << "string _id { get; }" << std::endl;
				ctx->writeTabs(code_deepness) << "_InvokeStatus _invoke(XElement root, out XElement ret, PIDL.IPIDLErrorCollector ec);" << std::endl;
				ctx->writeTabs(--code_deepness) << "}" << std::endl;
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
					if (!writeObject(code_deepness, ctx, intf, dynamic_cast<Language::Object*>(definition.get()), ec))
						return false;
				}

			return true;
		}

		bool writeDefinitions(short code_deepness, CSCodeGenContext * ctx, Language::Object * intf, ErrorCollector & ec)
		{
			for (auto & definition : intf->definitions())
				if (dynamic_cast<Language::Method*>(definition.get()))
				{
					if (!writeFunction(code_deepness, ctx, dynamic_cast<Language::Function*>(definition.get()), ec))
						return false;
				}
				else if (dynamic_cast<Language::Property*>(definition.get()))
				{
					if (!writeProperty(code_deepness, ctx, dynamic_cast<Language::Property*>(definition.get()), ec))
						return false;
				}

				return true;
		}

		bool writeInterface(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, intf, ec))
				return false;
			ctx->writeTabs(code_deepness) << "abstract class " << helper()->getName(intf) << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;

			if (!that->writeMembers(code_deepness, ctx, intf, ec) ||
				!writeConstructor(code_deepness, ctx, intf, ec) ||
				!writeDefinitions(code_deepness, ctx, intf, ec) ||
				!that->writeInvoke(code_deepness, ctx, intf, ec))
				return false;

			ctx->writeTabs(--code_deepness) << "}" << std::endl;

			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::After, intf, ec))
				return false;

			**ctx << std::endl;

			return true;
		}

		bool writeConstructor(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec)
		{
			ctx->writeTabs(code_deepness) << "protected " << intf->name() << "()" << std::endl;
			ctx->writeTabs(code_deepness++) << "{" << std::endl;
			if (!that->writeConstructorBody(intf, code_deepness, ctx, ec))
				return false;
			ctx->writeTabs(--code_deepness) << "}" << std::endl << std::endl;

			return true;
		}

		bool writeModule(short code_deepness, CSCodeGenContext * ctx, Language::Module * module, ErrorCollector & ec)
		{
			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, module, ec))
				return false;
			auto & o = ctx->writeTabs(code_deepness++) << "namespace " << helper()->getName(module) << " {" << std::endl;
			for (auto & element : module->elements())
			{
				o << std::endl;
				if (!writeTopLevel(code_deepness, ctx, element.get(), ec))
					return false;
			}
			ctx->writeTabs(--code_deepness) << "}" << std::endl;
			
			if (!writeDocumentation(code_deepness, ctx, CSCodeGenDocumentation::Before, module, ec))
				return false;

			**ctx << std::endl;

			return true;
		}

		bool writeTopLevel(short code_deepness, CSCodeGenContext * ctx, Language::TopLevel * top_level, ErrorCollector & ec)
		{
			if (dynamic_cast<Language::Interface*>(top_level))
				return writeInterface(code_deepness, ctx, dynamic_cast<Language::Interface*>(top_level), ec);
			if (dynamic_cast<Language::Module*>(top_level))
				return writeModule(code_deepness, ctx, dynamic_cast<Language::Module*>(top_level), ec);

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
		return priv->writeTopLevel(code_deepness, ctx, topLevel, ec);
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
