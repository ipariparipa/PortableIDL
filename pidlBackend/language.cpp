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

#include "include/pidlBackend/language.h"

namespace PIDL {
	namespace Language {

		//struct Element::Priv { };
		Element::Element() : priv(nullptr ){ }
		Element::~Element() = default;

		//struct Definition::Priv { };
		Definition::Definition() : priv(nullptr) { }
		Definition::~Definition() = default;

		//struct Type::Priv { };
		Type::Type() : priv(nullptr) { }
		Type::~Type() = default;

		Type::Ptr Type::finalType() const
		{
			return std::shared_ptr<Type>((Type*)this, [](void*){});
		}
		
		const std::vector<std::string> & Type::scope() const
		{
			static const std::vector<std::string> empty;
			return empty;
		}


		struct Variable::Priv
		{
			Priv(const Type::Ptr & t, const std::string & n) : type(t), name(n)
			{ }

			Type::Ptr type;
			std::string name;
		};
	
		Variable::Variable(const Type::Ptr & type, const std::string & name) :
			Element(),
			priv(new Priv(type, name))
		{ }

		Variable::~Variable()
		{
			delete priv;
		}

		const char * Variable::name() const
		{
			return priv->name.c_str();
		}

		std::shared_ptr<Type> Variable::type() const
		{
			return priv->type;
		}



		struct TypeDefinition::Priv
		{
			Priv(const std::string & n, const Type::Ptr & t, const std::vector<std::string> & s, const Documentation & d) :
				name(n), type(t), scope(s), doc(d)
			{ }

			std::string name;
			Type::Ptr type;
			std::vector<std::string> scope;
			Documentation doc;
		};

		TypeDefinition::TypeDefinition(const std::string & name, const Type::Ptr & type, const std::vector<std::string> & scope, const Documentation & doc) :
			Type(),
			priv(new Priv(name, type, scope, doc))
		{ }

		TypeDefinition::~TypeDefinition()
		{
			delete priv;
		}

		const char * TypeDefinition::name() const
		{
			return priv->name.c_str();
		}

		Type::Ptr TypeDefinition::type() const
		{
			return priv->type;
		}

		Type::Ptr TypeDefinition::finalType() const
		{
			return priv->type->finalType();
		}

		const std::vector<std::string> & TypeDefinition::scope() const
		{
			return priv->scope;
		}

		const TypeDefinition::Documentation & TypeDefinition::documentation() const
		{
			return priv->doc;
		}


		//struct ComplexType::Priv { };
		ComplexType::ComplexType() : priv(nullptr) { }
		ComplexType::~ComplexType() = default;


		//struct Blob::Priv { };
		Blob::Blob() : priv(nullptr) { }
		Blob::~Blob() = default;


		struct Generic::Priv
		{
			Priv(const std::vector<Type::Ptr> & t) : types(t)
			{ }

			Priv(const Type::Ptr & t)
			{
				types.push_back(t);
			}

			std::vector<Type::Ptr> types;
		};

		Generic::Generic(const std::vector<Type::Ptr> & types) :
			ComplexType(),
			priv(new Priv(types))
		{ }

		Generic::Generic(const Type::Ptr & type) :
			ComplexType(),
			priv(new Priv(type))
		{ }

		Generic::~Generic()
		{
			delete priv;
		}

		std::vector<Type::Ptr> Generic::types() const
		{
			return priv->types;
		}


		//struct Nullable::Priv { };
		Nullable::Nullable(const Type::Ptr & type) : Generic(type), priv(nullptr) { }
		Nullable::~Nullable() = default;

		//struct Array::Priv { };
		Array::Array(const Type::Ptr & type) : Generic(type), priv(nullptr) { }
		Array::~Array() = default;


		//struct Tuple::Priv { };
		Tuple::Tuple(const std::vector<Type::Ptr> & types) : Generic(types), priv(nullptr) { }
		Tuple::~Tuple() = default;


		struct Structure::Member::Priv 
		{
			Priv(const Documentation & doc_) : doc(doc_)
			{ }

			Documentation doc;
		};

		Structure::Member::Member(const Type::Ptr & type, const std::string & name, const Documentation & doc) : 
			Variable(type, name), 
			priv(new Priv(doc)) 
		{ }

		Structure::Member::~Member()
		{
			delete priv;
		}

		const Structure::Member::Documentation & Structure::Member::documentation() const
		{
			return priv->doc;
		}


		struct Structure::Priv
		{
			Priv(const std::vector<Member::Ptr> & m) : members(m)
			{ }

			Priv(const std::list<Member::Ptr> & m)
			{
				members.resize(m.size());
				std::copy(m.begin(), m.end(), members.begin());
			}

			std::vector<Member::Ptr> members;
		};

		Structure::Structure(const std::vector<Member::Ptr> & members) :
			ComplexType(),
			priv(new Priv(members))
		{ }

		Structure::Structure(const std::list<Member::Ptr> & members) :
			ComplexType(),
			priv(new Priv(members))
		{ }

		Structure::~Structure()
		{
			delete priv;
		}

		const std::vector<Structure::Member::Ptr> & Structure::members() const
		{
			return priv->members;
		}


		//struct NativeType::Priv { };
		NativeType::NativeType() : priv(nullptr) { }
		NativeType::~NativeType() = default;


		//struct Integer::Priv { };
		Integer::Integer() : priv(nullptr) { }
		Integer::~Integer() = default;

		//struct Float::Priv { };
		Float::Float() : priv(nullptr) { }
		Float::~Float() = default;

		//struct Boolean::Priv { };
		Boolean::Boolean() : priv(nullptr) { }
		Boolean::~Boolean() = default;

		//struct EmbeddedType::Priv { };
		EmbeddedType::EmbeddedType() : priv(nullptr) { }
		EmbeddedType::~EmbeddedType() = default;

		//struct String::Priv { };
		String::String() : priv(nullptr) { }
		String::~String() = default;

		//struct DateTime::Priv { };
		DateTime::DateTime() : priv(nullptr) { }
		DateTime::~DateTime() = default;

		//struct Void::Priv { };
		Void::Void() : priv(nullptr) { }
		Void::~Void() = default;


		struct Function::Variant::Argument::Priv
		{
			Priv(Direction direction_, const Documentation & doc_) : direction(direction_), doc(doc_)
			{ }

			Direction direction;
			Documentation doc;
		};

		Function::Variant::Argument::Argument(const Type::Ptr & type, const std::string & name, Direction direction, const Documentation & doc) :
			Variable(type, name),
			priv(new Priv(direction, doc))
		{ }

		Function::Variant::Argument::~Argument()
		{
			delete priv;
		}

		Function::Variant::Argument::Direction Function::Variant::Argument::direction() const
		{
			return priv->direction;
		}

		const Function::Variant::Argument::Documentation & Function::Variant::Argument::documentation() const
		{
			return priv->doc;
		}


		struct FunctionVariant::Priv
		{
			Priv(const Function::Ptr & function_, const Type::Ptr & returnType_, const std::string & name_, const std::vector<Argument::Ptr> & arguments_, const Documentation & doc_) :
				function(function_),
				returnType(returnType_), 
				name(name_), 
				arguments(arguments_),
				doc(doc_)
			{
				buildArguments();
				updateHash();
			}

			Priv(const Function::Ptr & function_, const Type::Ptr & returnType_, const std::string & name_, const std::list<Argument::Ptr> & arguments_, const Documentation & doc_) :
				function(function_),
				returnType(returnType_),
				name(name_),
				doc(doc_)
			{
				arguments.resize(arguments_.size());
				std::copy(arguments_.begin(), arguments_.end(), arguments.begin());
				buildArguments();
				updateHash();
			}

			Function::Ptr function;
			std::shared_ptr<Type> returnType;
			std::string name;
			std::string argumentHash, hash;
			std::vector<std::shared_ptr<Argument>> arguments, in_arguments, out_arguments;
			Documentation doc;

			void updateHash()
			{
				argumentHash = "ARG:";
				if (arguments.size())
				{
					std::map<std::string, Argument::Ptr> args;
					for (auto & a : arguments)
						args[a->name()] = a;

					bool is_first = true;
					for (auto & a : args)
					{
						if (!is_first)
							argumentHash += "|";
						else
							is_first = false;
						switch (a.second->direction())
						{
						case Argument::Direction::In:
							argumentHash += "in:"; break;
						case Argument::Direction::InOut:
							argumentHash += "inout:"; break;
						case Argument::Direction::Out:
							argumentHash += "out:"; break;
						}
						argumentHash +=  a.first;
					}
				}
				else
					argumentHash += "void";
				hash = name + "#" + argumentHash;
			}
		private:
			void buildArguments()
			{
				for (auto & a : arguments)
					switch (a->direction())
					{
					case Argument::Direction::In:
						in_arguments.push_back(a);
						break;
					case Argument::Direction::InOut:
						in_arguments.push_back(a);
						out_arguments.push_back(a);
						break;
					case Argument::Direction::Out:
						out_arguments.push_back(a);
						break;
					}
			}
		};

		FunctionVariant::FunctionVariant(const Function::Ptr & function, const Type::Ptr & returnType, const std::string & name, const std::vector<Argument::Ptr> & arguments, const Documentation & doc) :
			Definition(),
			priv(new Priv(function, returnType, name, arguments, doc))
		{ }

		FunctionVariant::FunctionVariant(const Function::Ptr & function, const Type::Ptr & returnType, const std::string & name, const std::list<Argument::Ptr> & arguments, const Documentation & doc) :
			Definition(),
			priv(new Priv(function, returnType, name, arguments, doc))
		{ }

		FunctionVariant::~FunctionVariant()
		{
			delete priv;
		}

		const char * FunctionVariant::name() const
		{
			return priv->name.c_str();
		}

		const Function::Ptr & FunctionVariant::function() const
		{
			return priv->function;
		}

		const std::string & FunctionVariant::variantId() const
		{
			return priv->argumentHash;
		}

		const std::vector<std::shared_ptr<Function::Variant::Argument>> & FunctionVariant::arguments() const
		{
			return priv->arguments;
		}

		const std::vector<std::shared_ptr<Function::Variant::Argument>> & FunctionVariant::in_arguments() const
		{
			return priv->in_arguments;
		}

		const std::vector<std::shared_ptr<Function::Variant::Argument>> & FunctionVariant::out_arguments() const
		{
			return priv->out_arguments;
		}

		std::shared_ptr<Type> FunctionVariant::returnType() const
		{
			return priv->returnType;
		}

		const FunctionVariant::Documentation & FunctionVariant::documentation() const
		{
			return priv->doc;
		}

		struct Function::Priv
		{
			Priv(const std::vector<std::string> & scope_, const std::string & name_) : 
				scope(scope_), name(name_)
			{ }

			std::vector<std::string> scope;
			std::string name;

			std::map<std::string /*variantId*/, Variant::Ptr> variants;
		};

		Function::Function(const std::vector<std::string> & scope, const std::string & name) : Definition(), priv(new Priv(scope, name))
		{ }

		Function::~Function()
		{
			delete priv;
		}

		const char * Function::name() const
		{
			return priv->name.c_str();
		}

		const std::vector<std::string> & Function::scope() const
		{
			return priv->scope;
		}

		std::map<std::string /*variantId*/, Function::Variant::Ptr> & Function::variants()
		{
			return priv->variants;
		}

		const std::map<std::string /*variantId*/, Function::Variant::Ptr> & Function::variants() const
		{
			return priv->variants;
		}


		struct TopLevel::Priv 
		{
			Priv(const Info & info_) : info(info_) { }
			Priv() { }

			Info info;
		};

		TopLevel::TopLevel(const Info & info) : priv(new Priv(info))
		{ }

		TopLevel::TopLevel() : priv(new Priv)
		{ }

		TopLevel::~TopLevel()
		{
			delete priv;
		}

		const std::vector<std::string> & TopLevel::scope() const
		{
			static const std::vector<std::string> empty;
			return empty;
		}

		void TopLevel::setInfo(const std::string & name, const std::string & value)
		{
			priv->info[name] = value;
		}

		void TopLevel::setInfo(const Info & info)
		{
			priv->info = info;
		}

		const TopLevel::Info & TopLevel::info() const
		{
			return priv->info;
		}


		// struct DefinitionProvider::Priv { };
		DefinitionProvider::DefinitionProvider() : priv(nullptr) { }
		DefinitionProvider::~DefinitionProvider() = default;


		struct Interface::Priv
		{
            Priv(const std::string & name_, const std::vector<Definition::Ptr> & definitions_, const std::vector<std::string> & scope_, const Documentation & doc_, const std::string & loggerName_) :
                name(name_), scope(scope_), definitions(definitions_), doc(doc_), loggerName(loggerName_)
			{
				scope.push_back(name);
			}

            Priv(const std::string & name_, const std::list<Definition::Ptr> & definitions_, const std::vector<std::string> & scope_, const Documentation & doc_, const std::string & loggerName_) :
                name(name_), scope(scope_), doc(doc_), loggerName(loggerName_)
			{ 
				definitions.resize(definitions_.size());
				std::copy(definitions_.begin(), definitions_.end(), definitions.begin());
			}

			std::string name;
			std::vector<std::string> scope;
			std::vector<std::shared_ptr<Definition>> definitions;
			Documentation doc;
            std::string loggerName;
		};

        Interface::Interface(const std::string & name, const std::vector<Definition::Ptr> & definitions, const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName) :
			TopLevel(),
			DefinitionProvider(),
			DocumentationProvider(),
            priv(new Priv(name, definitions, scope, doc, loggerName))
		{ }

        Interface::Interface(const std::string & name, const std::list<Definition::Ptr> & definitions, const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName) :
			TopLevel(),
			DefinitionProvider(),
			DocumentationProvider(),
            priv(new Priv(name, definitions, scope, doc, loggerName))
		{ }

		Interface::~Interface()
		{
			delete priv;
		}

		const std::vector<std::shared_ptr<Definition>> & Interface::definitions() const
		{
			return priv->definitions;
		}

		const char * Interface::name() const
		{
			return priv->name.c_str();
		}

		const std::vector<std::string> & Interface::scope() const
		{
			return priv->scope;
		}

		const Interface::Documentation & Interface::documentation() const
		{
			return priv->doc;
		}

        const std::string & Interface::loggerName() const
        {
            return priv->loggerName;
        }

		//struct MethodVariant::Priv { };

		MethodVariant::MethodVariant(const Method::Ptr & function, const Type::Ptr & returnType, const std::string & name, const std::vector<Argument::Ptr> & arguments, const Documentation & doc) :
			FunctionVariant(function, returnType, name, arguments, doc), priv(nullptr)
		{ }

		MethodVariant::MethodVariant(const Method::Ptr & function, const Type::Ptr & returnType, const std::string & name, const std::list<Argument::Ptr> & arguments, const Documentation & doc) :
			FunctionVariant(function, returnType, name, arguments, doc), priv(nullptr)
		{ }

		MethodVariant::~MethodVariant() = default;


		//struct Method::Priv { };

		Method::Method(const std::vector<std::string> & scope, const std::string & name) : Function(scope, name), priv(nullptr)
		{ }

		Method::~Method() = default;


		struct Property::Priv
		{
			Priv(const Type::Ptr & type_, const std::vector<std::string> & scope_, const std::string & name_, bool readOnly_, const Documentation & doc_) :
				type(type_), scope(scope_), name(name_), readOnly(readOnly_), doc(doc_)
			{ }

			Type::Ptr type;
			std::vector<std::string> scope;
			std::string name;
			bool readOnly;
			Documentation doc;
		};

		Property::Property(const Type::Ptr & type, const std::vector<std::string> & scope, const std::string & name, bool readOnly, const Documentation & doc) :
				Definition(), priv(new Priv(type, scope, name, readOnly, doc))
		{ }

		Property::~Property()
		{
			delete priv;
		}

		Type::Ptr Property::type() const
		{
			return priv->type;
		}

		const std::vector<std::string> & Property::scope() const
		{
			return priv->scope;
		}

		bool Property::readOnly() const
		{
			return priv->readOnly;
		}

		const char * Property::name() const
		{
			return priv->name.c_str();
		}

		const Property::Documentation & Property::documentation() const
		{
			return priv->doc;
		}


		struct Object::Priv
		{
            Priv(const std::string & name, const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName) :
                initialized(true), name(name), scope(scope), doc(doc), loggerName(loggerName)
			{ }

            Priv(const std::string & name) :
                initialized(false), name(name)
            { }

            bool initialized;
			std::string name;
			std::vector<std::string> scope;
			std::vector<std::shared_ptr<Definition>> definitions;
			Documentation doc;
            std::string loggerName;
		};

        Object::Object(const std::string & name, const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName) :
			Type(),
			DefinitionProvider(),
			Definition(),
			DocumentationProvider(),
            priv(new Priv(name, scope, doc, loggerName))
		{ }

        Object::Object(const std::string & name) :
            Type(),
            DefinitionProvider(),
            Definition(),
            DocumentationProvider(),
            priv(new Priv(name))
        { }

		Object::~Object()
		{
			delete priv;
		}

        void Object::init(const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName)
        {
            priv->scope = scope;
            priv->doc = doc;
            priv->loggerName = loggerName;
            priv->initialized = true;
        }

        bool Object::initialized() const
        {
            return priv->initialized;
        }

        void Object::setDefinitions(const std::vector<Definition::Ptr> & definitions)
        {
            priv->definitions = definitions;
        }

        void Object::setDefinitions(const std::list<Definition::Ptr> & definitions)
        {
            priv->definitions.resize(definitions.size());
            std::copy(definitions.begin(), definitions.end(), priv->definitions.begin());
        }

		const std::vector<std::shared_ptr<Definition>> & Object::definitions() const
		{
			return priv->definitions;
		}

		const char * Object::name() const
		{
			return priv->name.c_str();
		}

		const std::vector<std::string> & Object::scope() const
		{
			return priv->scope;
		}

		const Object::Documentation & Object::documentation() const
		{
			return priv->doc;
		}

        const std::string & Object::loggerName() const
        {
            return priv->loggerName;
        }

		struct Module::Priv
		{
			Priv(const std::string & name_, const std::vector<TopLevel::Ptr> & elements_, const Documentation & doc_) :
				name(name_), elements(elements_), doc(doc_)
			{ }

			std::string name;
			std::vector<TopLevel::Ptr> elements;
			Documentation doc;
		};

		Module::Module(const std::string & name, const std::vector<TopLevel::Ptr> & elements, const Documentation & doc) :
			TopLevel(),
			priv(new Priv(name, elements, doc))
		{ }

		Module::Module(const std::string & name, const std::vector<TopLevel::Ptr> & elements, const Documentation & doc, const Info & info) :
			TopLevel(info),
			priv(new Priv(name, elements, doc))
		{ }

		Module::~Module()
		{
			delete priv;
		}

		const char * Module::name() const
		{
			return priv->name.c_str();
		}

		const std::vector<TopLevel::Ptr> & Module::elements() const
		{
			return priv->elements;
		}

		const Module::Documentation & Module::documentation() const
		{
			return priv->doc;
		}

}}
