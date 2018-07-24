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

		std::shared_ptr<Type> Type::finalType() const
		{
			return std::shared_ptr<Type>((Type*)this, [](void*){});
		}


		struct Variable::Priv
		{
			Priv(const std::shared_ptr<Type> & t, const std::string & n) : type(t), name(n)
			{ }

			std::shared_ptr<Type> type;
			std::string name;
		};
	
		Variable::Variable(const std::shared_ptr<Type> & type, const std::string & name) :
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
			Priv(const std::string & n, const std::shared_ptr<Type> & t) : name(n), type(t)
			{ }

			std::string name;
			std::shared_ptr<Type> type;
		};

		TypeDefinition::TypeDefinition(const std::string & name, const std::shared_ptr<Type> & type) :
			Type(),
			priv(new Priv(name, type))
		{ }

		TypeDefinition::~TypeDefinition()
		{
			delete priv;
		}

		const char * TypeDefinition::name() const
		{
			return priv->name.c_str();
		}

		std::shared_ptr<Type> TypeDefinition::type() const
		{
			return priv->type;
		}

		std::shared_ptr<Type> TypeDefinition::finalType() const
		{
			return priv->type->finalType();
		}


		//struct ComplexType::Priv { };
		ComplexType::ComplexType() : priv(nullptr) { }
		ComplexType::~ComplexType() = default;


		//struct Blob::Priv { };
		Blob::Blob() : priv(nullptr) { }
		Blob::~Blob() = default;


		struct Generic::Priv
		{
			Priv(const std::shared_ptr<Type> & t) : type(t)
			{ }

			std::shared_ptr<Type> type;
		};

		Generic::Generic(const std::shared_ptr<Type> & type) :
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
		Nullable::Nullable(const std::shared_ptr<Type> & type) : Generic(type), priv(nullptr) { }
		Nullable::~Nullable() = default;

		//struct Array::Priv { };
		Array::Array(const std::shared_ptr<Type> & type) : Generic(type), priv(nullptr) { }
		Array::~Array() = default;

		//struct Structure::Member::Priv { };
		Structure::Member::Member(const std::shared_ptr<Type> & type, const std::string & name) : Variable(type, name), priv(nullptr) { }
		Structure::Member:: ~Member() = default;


		struct Structure::Priv
		{
			Priv(const std::vector<std::shared_ptr<Member>> & m) : members(m)
			{ }

			Priv(const std::list<std::shared_ptr<Member>> & m)
			{
				members.resize(m.size());
				std::copy(m.begin(), m.end(), members.begin());
			}

			std::vector<std::shared_ptr<Member>> members;
		};

		Structure::Structure(const std::vector<std::shared_ptr<Member>> & members) :
			ComplexType(),
			priv(new Priv(members))
		{ }

		Structure::Structure(const std::list<std::shared_ptr<Member>> & members) :
			ComplexType(),
			priv(new Priv(members))
		{ }

		Structure::~Structure()
		{
			delete priv;
		}

		const std::vector<std::shared_ptr<Structure::Member>> & Structure::members() const
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
			Priv(Direction direction_) : direction(direction_)
			{ }

			Direction direction;
		};

		Function::Argument::Argument(const std::shared_ptr<Type> & type, const std::string & name, Direction direction) :
			Variable(type, name),
			priv(new Priv(direction))
		{ }

		Function::Argument::~Argument()
		{
			delete priv;
		}

		Function::Argument::Direction Function::Argument::direction() const
		{
			return priv->direction;
		}



		struct Function::Priv
		{
			Priv(const std::shared_ptr<Type> & returnType_, const std::vector<std::string> & scope_, const std::string & name_, const std::vector<std::shared_ptr<Argument>> & arguments_) :
				returnType(returnType_), 
				scope(scope_),
				name(name_), 
				arguments(arguments_)
			{
				buildArguments();
			}

			Priv(const std::shared_ptr<Type> & returnType_, const std::vector<std::string> & scope_, const std::string & name_, const std::list<std::shared_ptr<Argument>> & arguments_) :
				returnType(returnType_),
				scope(scope_),
				name(name_)
			{
				arguments.resize(arguments_.size());
				std::copy(arguments_.begin(), arguments_.end(), arguments.begin());
				buildArguments();
			}

			std::shared_ptr<Type> returnType;
			std::vector<std::string> scope;
			std::string name;
			std::vector<std::shared_ptr<Argument>> arguments, in_arguments, out_arguments;

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

		Function::Function(const std::shared_ptr<Type> & returnType, const std::vector<std::string> & scope, const std::string & name, const std::vector<std::shared_ptr<Argument>> & arguments) :
			Definition(),
			priv(new Priv(returnType, scope, name, arguments))
		{ }

		Function::Function(const std::shared_ptr<Type> & returnType, const std::vector<std::string> & scope, const std::string & name, const std::list<std::shared_ptr<Argument>> & arguments) :
			Definition(),
			priv(new Priv(returnType, scope, name, arguments))
		{ }

		Function::~Function()
		{
			delete priv;
		}

		const char * Function::name() const
		{
			return priv->name.c_str();
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


		//struct TopLevel::Priv { };
		TopLevel::TopLevel() : priv(nullptr) { }
		TopLevel::~TopLevel() = default;


		struct Interface::Priv
		{
			Priv(const std::string & name_, const std::vector<std::shared_ptr<Definition>> & definitions_) :
				name(name_), definitions(definitions_)
			{ }

			Priv(const std::string & name_, const std::list<std::shared_ptr<Definition>> & definitions_) :
				name(name_)
			{ 
				definitions.resize(definitions_.size());
				std::copy(definitions_.begin(), definitions_.end(), definitions.begin());
			}

			std::string name;
			std::vector<std::shared_ptr<Definition>> definitions;
		};

		Interface::Interface(const std::string & name, const std::vector<std::shared_ptr<Definition>> & definitions) :
			TopLevel(),
			priv(new Priv(name, definitions))
		{ }

		Interface::Interface(const std::string & name, const std::list<std::shared_ptr<Definition>> & definitions) :
			TopLevel(),
			priv(new Priv(name, definitions))
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


		struct Module::Priv
		{
			Priv(const std::string & name_, const std::vector<std::shared_ptr<TopLevel>> & elements_) :
				name(name_), elements(elements_)
			{ }

			std::string name;
			std::vector<std::shared_ptr<TopLevel>> elements;
		};

		Module::Module(const std::string &name, const std::vector<std::shared_ptr<TopLevel>> & elements) :
			TopLevel(),
			priv(new Priv(name, elements))
		{ }

		Module::~Module()
		{
			delete priv;
		}

		const char * Module::name() const
		{
			return priv->name.c_str();
		}

		const std::vector<std::shared_ptr<TopLevel>> & Module::elements() const
		{
			return priv->elements;
		}

}}
