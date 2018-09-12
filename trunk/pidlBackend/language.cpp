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
			Priv(const Type::Ptr & t) : type(t)
			{ }

			std::shared_ptr<Type> type;
		};

		Generic::Generic(const Type::Ptr & type) :
			ComplexType(),
			priv(new Priv(type))
		{ }

		Generic::~Generic()
		{
			delete priv;
		}

		std::shared_ptr<Type> Generic::type() const
		{
			return priv->type;
		}


		//struct Nullable::Priv { };
		Nullable::Nullable(const Type::Ptr & type) : Generic(type), priv(nullptr) { }
		Nullable::~Nullable() = default;

		//struct Array::Priv { };
		Array::Array(const Type::Ptr & type) : Generic(type), priv(nullptr) { }
		Array::~Array() = default;


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


		struct Function::Argument::Priv
		{
			Priv(Direction direction_, const Documentation & doc_) : direction(direction_), doc(doc_)
			{ }

			Direction direction;
			Documentation doc;
		};

		Function::Argument::Argument(const Type::Ptr & type, const std::string & name, Direction direction, const Documentation & doc) :
			Variable(type, name),
			priv(new Priv(direction, doc))
		{ }

		Function::Argument::~Argument()
		{
			delete priv;
		}

		Function::Argument::Direction Function::Argument::direction() const
		{
			return priv->direction;
		}

		const Function::Argument::Documentation & Function::Argument::documentation() const
		{
			return priv->doc;
		}



		struct Function::Priv
		{
			Priv(const Type::Ptr & returnType_, const std::vector<std::string> & scope_, const std::string & name_, const std::vector<Argument::Ptr> & arguments_, const Documentation & doc_) :
				returnType(returnType_), 
				scope(scope_),
				name(name_), 
				arguments(arguments_),
				doc(doc_)
			{
				buildArguments();
				updateHash();
			}

			Priv(const Type::Ptr & returnType_, const std::vector<std::string> & scope_, const std::string & name_, const std::list<Argument::Ptr> & arguments_, const Documentation & doc_) :
				returnType(returnType_),
				scope(scope_),
				name(name_),
				doc(doc_)
			{
				arguments.resize(arguments_.size());
				std::copy(arguments_.begin(), arguments_.end(), arguments.begin());
				buildArguments();
				updateHash();
			}

			std::shared_ptr<Type> returnType;
			std::vector<std::string> scope;
			std::string name;
			std::string hash;
			std::vector<std::shared_ptr<Argument>> arguments, in_arguments, out_arguments;
			Documentation doc;

			void updateHash()
			{
				hash = name;
				for (auto & a : arguments)
					hash +=std::string("|") + a->name();
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

		Function::Function(const Type::Ptr & returnType, const std::vector<std::string> & scope, const std::string & name, const std::vector<Argument::Ptr> & arguments, const Documentation & doc) :
			Definition(),
			priv(new Priv(returnType, scope, name, arguments, doc))
		{ }

		Function::Function(const Type::Ptr & returnType, const std::vector<std::string> & scope, const std::string & name, const std::list<Argument::Ptr> & arguments, const Documentation & doc) :
			Definition(),
			priv(new Priv(returnType, scope, name, arguments, doc))
		{ }

		Function::~Function()
		{
			delete priv;
		}

		const char * Function::name() const
		{
			return priv->name.c_str();
		}

		const std::string & Function::hash() const
		{
			return priv->hash;
		}

		const std::vector<std::shared_ptr<Function::Argument>> & Function::arguments() const
		{
			return priv->arguments;
		}

		const std::vector<std::shared_ptr<Function::Argument>> & Function::in_arguments() const
		{
			return priv->in_arguments;
		}

		const std::vector<std::shared_ptr<Function::Argument>> & Function::out_arguments() const
		{
			return priv->out_arguments;
		}

		std::shared_ptr<Type> Function::returnType() const
		{
			return priv->returnType;
		}

		const std::vector<std::string> & Function::scope() const
		{
			return priv->scope;
		}

		const Function::Documentation & Function::documentation() const
		{
			return priv->doc;
		}



		//struct TopLevel::Priv { };
		TopLevel::TopLevel() : priv(nullptr) { }
		TopLevel::~TopLevel() = default;
		const std::vector<std::string> & TopLevel::scope() const
		{
			static const std::vector<std::string> empty;
			return empty;
		}


		struct Interface::Priv
		{
			Priv(const std::string & name_, const std::vector<Definition::Ptr> & definitions_, const std::vector<std::string> & scope_, const Documentation & doc_) :
				name(name_), scope(scope_), definitions(definitions_), doc(doc_)
			{
				scope.push_back(name);
			}

			Priv(const std::string & name_, const std::list<Definition::Ptr> & definitions_, const std::vector<std::string> & scope_, const Documentation & doc_) :
				name(name_), scope(scope_), doc(doc_)
			{ 
				definitions.resize(definitions_.size());
				std::copy(definitions_.begin(), definitions_.end(), definitions.begin());
			}

			std::string name;
			std::vector<std::string> scope;
			std::vector<std::shared_ptr<Definition>> definitions;
			Documentation doc;
		};

		Interface::Interface(const std::string & name, const std::vector<Definition::Ptr> & definitions, const std::vector<std::string> & scope, const Documentation & doc) :
			TopLevel(),
			priv(new Priv(name, definitions, scope, doc))
		{ }

		Interface::Interface(const std::string & name, const std::list<Definition::Ptr> & definitions, const std::vector<std::string> & scope, const Documentation & doc) :
			TopLevel(),
			priv(new Priv(name, definitions, scope, doc))
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


		//struct Method::Priv { };

		Method::Method(const std::shared_ptr<Type> & returnType, const std::vector<std::string> & scope, const std::string & name, const std::vector<Argument::Ptr> & arguments, const Documentation & doc) :
				Function(returnType, scope, name, arguments, doc), priv(nullptr)
		{ }

		Method::Method(const std::shared_ptr<Type> & returnType, const std::vector<std::string> & scope, const std::string & name, const std::list<Argument::Ptr> & arguments, const Documentation & doc) :
						Function(returnType, scope, name, arguments, doc), priv(nullptr)
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
			Priv(const std::string & name_, const std::vector<Definition::Ptr> & definitions_, const std::vector<std::string> & scope_, const Documentation & doc_) :
				name(name_), scope(scope_), definitions(definitions_), doc(doc_)
			{ }

			Priv(const std::string & name_, const std::list<Definition::Ptr> & definitions_, const std::vector<std::string> & scope_, const Documentation & doc_) :
				name(name_), scope(scope_), doc(doc_)
			{
				definitions.resize(definitions_.size());
				std::copy(definitions_.begin(), definitions_.end(), definitions.begin());
			}

			std::string name;
			std::vector<std::string> scope;
			std::vector<std::shared_ptr<Definition>> definitions;
			Documentation doc;
		};

		Object::Object(const std::string & name, const std::vector<Definition::Ptr> & definitions, const std::vector<std::string> & scope, const Documentation & doc) :
			Definition(),
			priv(new Priv(name, definitions, scope, doc))
		{ }

		Object::Object(const std::string & name, const std::list<Definition::Ptr> & definitions, const std::vector<std::string> & scope, const Documentation & doc) :
			Definition(),
			priv(new Priv(name, definitions, scope, doc))
		{ }

		Object::~Object()
		{
			delete priv;
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
