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

#ifndef pidlBackend__language_h
#define pidlBackend__language_h

#include "config.h"

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <map>

namespace PIDL { namespace Language {

	class PIDL_BACKEND__CLASS Element
	{
		PIDL_COPY_PROTECTOR(Element)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Element> Ptr;
		Element();
		virtual ~Element();
	};

	class PIDL_BACKEND__CLASS DocumentationProvider
	{
	public:
		struct Documentation
		{
			enum Detail
			{
				Description,
				Return
			};

			std::string brief;
			std::map<Detail, std::string> details;
		};

		virtual const Documentation & documentation() const = 0;
	};

	class PIDL_BACKEND__CLASS Definition : public Element
	{
		PIDL_COPY_PROTECTOR(Definition)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Definition> Ptr;
		Definition();
		virtual ~Definition();
	};

	class PIDL_BACKEND__CLASS Type : public Element
	{
		PIDL_COPY_PROTECTOR(Type)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Type> Ptr;
		Type();
		virtual ~Type();
		virtual Type::Ptr finalType() const;
		virtual const char * name() const = 0;
		virtual const std::vector<std::string> & scope() const;
	};

	class PIDL_BACKEND__CLASS Variable : public Element
	{
		PIDL_COPY_PROTECTOR(Variable)
		struct Priv;
		Priv * priv;
	public:
		Variable(const std::shared_ptr<Type> & type, const std::string & name);
		virtual ~Variable();

		virtual const char * name() const;
		Type::Ptr type() const;
	};

	class PIDL_BACKEND__CLASS TypeDefinition : public Type, public Definition, public DocumentationProvider
	{
		PIDL_COPY_PROTECTOR(TypeDefinition)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<TypeDefinition> Ptr;
		TypeDefinition(const std::string & name, const std::shared_ptr<Type> & type, const std::vector<std::string> & scope, const Documentation & doc);
		virtual ~TypeDefinition();

		virtual const char * name() const override;
		virtual Type::Ptr type() const;
		virtual Type::Ptr finalType() const override;
		virtual const std::vector<std::string> & scope() const override;

		virtual const Documentation & documentation() const override;
	};

	class PIDL_BACKEND__CLASS ComplexType : public Type
	{
		PIDL_COPY_PROTECTOR(ComplexType)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<ComplexType> Ptr;
		ComplexType();
		virtual ~ComplexType();
	};

	class PIDL_BACKEND__CLASS Generic : public ComplexType
	{
		PIDL_COPY_PROTECTOR(Generic)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Generic> Ptr;
		virtual ~Generic();
		std::vector<Type::Ptr> types() const;
	protected:
		Generic(const std::vector<Type::Ptr> & types);
		Generic(const Type::Ptr & type);
	};

	class PIDL_BACKEND__CLASS Nullable : public Generic
	{
		PIDL_COPY_PROTECTOR(Nullable)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Nullable> Ptr;
		Nullable(const Type::Ptr & type);
		virtual ~Nullable();
		virtual const char * name() const override { return "nullable"; }
	};

	class PIDL_BACKEND__CLASS Array : public Generic
	{
		PIDL_COPY_PROTECTOR(Array)
		struct Priv;
		Priv * priv;
	public:
		Array(const Type::Ptr & type);
		virtual ~Array();
		virtual const char * name() const override { return "array"; }
	};

	class PIDL_BACKEND__CLASS Tuple : public Generic
	{
		PIDL_COPY_PROTECTOR(Tuple)
		struct Priv;
		Priv * priv;
	public:
		Tuple(const std::vector<Type::Ptr> & type);
		virtual ~Tuple();
		virtual const char * name() const override { return "tuple"; }
	};

	class PIDL_BACKEND__CLASS Structure : public ComplexType
	{
		PIDL_COPY_PROTECTOR(Structure)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Structure> Ptr;
		class Member : public Variable, public DocumentationProvider
		{
			PIDL_COPY_PROTECTOR(Member)
			struct Priv;
			Priv * priv;
		public:
			typedef std::shared_ptr<Member> Ptr;
			Member(const Type::Ptr & type, const std::string & name, const Documentation & doc);
			virtual ~Member();

			virtual const Documentation & documentation() const override;
		};

		Structure(const std::vector<Member::Ptr> & members);
		Structure(const std::list<Member::Ptr> & members);
		virtual ~Structure();

		virtual const char * name() const override { return "structure"; }
		const std::vector<Member::Ptr> & members() const;
	};

	class PIDL_BACKEND__CLASS NativeType : public Type
	{
		PIDL_COPY_PROTECTOR(NativeType)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<NativeType> Ptr;
		NativeType();
		virtual ~NativeType();
	};

	class PIDL_BACKEND__CLASS Integer : public NativeType
	{
		PIDL_COPY_PROTECTOR(Integer)
		struct Priv;
		Priv * priv;
	public:
		Integer();
		virtual ~Integer();

		virtual const char * name() const override { return "integer"; }
	};

	class PIDL_BACKEND__CLASS Float : public NativeType
	{
		PIDL_COPY_PROTECTOR(Float)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Float> Ptr;
		Float();
		virtual ~Float();

		virtual const char * name() const override { return "float"; }
	};

	class PIDL_BACKEND__CLASS Boolean : public NativeType
	{
		PIDL_COPY_PROTECTOR(Boolean)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Boolean> Ptr;
		Boolean();
		virtual ~Boolean();

		virtual const char * name() const override { return "boolean"; }
	};

	class PIDL_BACKEND__CLASS EmbeddedType : public ComplexType
	{
		PIDL_COPY_PROTECTOR(EmbeddedType)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<EmbeddedType> Ptr;
		EmbeddedType();
		virtual ~EmbeddedType();
	};

	class PIDL_BACKEND__CLASS String : public EmbeddedType
	{
		PIDL_COPY_PROTECTOR(String)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<String> Ptr;
		String();
		virtual ~String();

		virtual const char * name() const override { return "string"; }
	};

	class PIDL_BACKEND__CLASS DateTime : public EmbeddedType
	{
		PIDL_COPY_PROTECTOR(DateTime)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<DateTime> Ptr;
		DateTime();
		virtual ~DateTime();

		virtual const char * name() const override { return "datetime"; }
	};

	class PIDL_BACKEND__CLASS Blob : public EmbeddedType
	{
		PIDL_COPY_PROTECTOR(Blob)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Blob> Ptr;
		Blob();
		virtual ~Blob();
		virtual const char * name() const override { return "blob"; }
	};

	class PIDL_BACKEND__CLASS Void : public Type
	{
		PIDL_COPY_PROTECTOR(Void)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Void> Ptr;
		Void();
		virtual ~Void();
		virtual const char * name() const override { return "void"; }
	};

	class Function;

	class PIDL_BACKEND__CLASS FunctionVariant : public Definition, public DocumentationProvider
	{
		PIDL_COPY_PROTECTOR(FunctionVariant)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<FunctionVariant> Ptr;

		class PIDL_BACKEND__CLASS Argument : public Variable, public DocumentationProvider
		{
			PIDL_COPY_PROTECTOR(Argument)
			struct Priv;
			Priv * priv;
		public:
			typedef std::shared_ptr<Argument> Ptr;
			enum class Direction {
				In, Out, InOut
			};

			Argument(const Type::Ptr & type, const std::string & name, Direction direction, const Documentation & doc);
			virtual ~Argument();

			Direction direction() const;

			virtual const Documentation & documentation() const override;
		};

		FunctionVariant(const std::shared_ptr<Function> & function, const Type::Ptr & returnType, const std::string & name, const std::vector<Argument::Ptr> & arguments, const Documentation & doc);
		FunctionVariant(const std::shared_ptr<Function> & function, const Type::Ptr & returnType, const std::string & name, const std::list<Argument::Ptr> & arguments, const Documentation & doc);
		virtual ~FunctionVariant();

		virtual const char * name() const;
		const std::shared_ptr<Function> & function() const;
		const std::string & variantId() const;
		const std::vector<Argument::Ptr> & arguments() const;
		const std::vector<Argument::Ptr> & in_arguments() const;
		const std::vector<Argument::Ptr> & out_arguments() const;
		Type::Ptr returnType() const;

		virtual const Documentation & documentation() const override;
	};

	class PIDL_BACKEND__CLASS Function : public Definition
	{
		PIDL_COPY_PROTECTOR(Function)
		struct Priv;
		Priv * priv;
	public:
		typedef FunctionVariant Variant;
		typedef std::shared_ptr<Function> Ptr;

		Function(const std::vector<std::string> & scope, const std::string & name);
		virtual ~Function();

		virtual const char * name() const;
		const std::vector<std::string> & scope() const;

		std::map<std::string /*variantId*/, Variant::Ptr> & variants();
		const std::map<std::string /*variantId*/, Variant::Ptr> & variants() const;
	};

	class PIDL_BACKEND__CLASS TopLevel : public Element
	{
		PIDL_COPY_PROTECTOR(TopLevel)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<TopLevel> Ptr;
		typedef std::map<std::string /*name*/, std::string /*value*/> Info;
 
		TopLevel();
		TopLevel(const Info & info);
		virtual ~TopLevel();
		virtual const char * name() const = 0;
		virtual const std::vector<std::string> & scope() const;

		void setInfo(const std::string & name, const std::string & value);
		void setInfo(const Info & info);
		const Info & info() const;
	};

	class PIDL_BACKEND__CLASS DefinitionProvider : public Element
	{
		PIDL_COPY_PROTECTOR(DefinitionProvider)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<DefinitionProvider> Ptr;

		DefinitionProvider();
		virtual ~DefinitionProvider();
		virtual const std::vector<Definition::Ptr> & definitions() const = 0;
	};

	class PIDL_BACKEND__CLASS Interface : public TopLevel, public DefinitionProvider, public DocumentationProvider
	{
		PIDL_COPY_PROTECTOR(Interface)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Interface> Ptr;
        Interface(const std::string & name, const std::vector<std::shared_ptr<Definition>> & definitions, const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName);
        Interface(const std::string & name, const std::list<std::shared_ptr<Definition>> & definitions, const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName);
		virtual ~Interface();
		const std::vector<Definition::Ptr> & definitions() const override;
		virtual const char * name() const override;
		virtual const std::vector<std::string> & scope() const override;
        virtual const Documentation & documentation() const override;
        const std::string & loggerName() const;
    };

	class Method;

	class PIDL_BACKEND__CLASS MethodVariant : public FunctionVariant
	{
		PIDL_COPY_PROTECTOR(MethodVariant)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<MethodVariant> Ptr;
		MethodVariant(const std::shared_ptr<Method> & function, const Type::Ptr & returnType, const std::string & name, const std::vector<Argument::Ptr> & arguments, const Documentation & doc);
		MethodVariant(const std::shared_ptr<Method> & function, const Type::Ptr & returnType, const std::string & name, const std::list<Argument::Ptr> & arguments, const Documentation & doc);
		virtual ~MethodVariant();
	};

	class PIDL_BACKEND__CLASS Method : public Function
	{
		PIDL_COPY_PROTECTOR(Method)
		struct Priv;
		Priv * priv;
	public:
		typedef MethodVariant Variant;
		typedef std::shared_ptr<Method> Ptr;

		Method(const std::vector<std::string> & scope, const std::string & name);
		virtual ~Method();
	};

	class PIDL_BACKEND__CLASS Property : public Definition, public DocumentationProvider
	{
		PIDL_COPY_PROTECTOR(Property)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Property> Ptr;
		Property(const Type::Ptr & type, const std::vector<std::string> & scope, const std::string & name, bool readOnly, const Documentation & doc);
		virtual ~Property();
		Type::Ptr type() const;
		const std::vector<std::string> & scope() const;
		bool readOnly() const;
		virtual const char * name() const;

		virtual const Documentation & documentation() const override;
	};

	class PIDL_BACKEND__CLASS Object : public Type, public DefinitionProvider, public Definition, public DocumentationProvider
	{
		PIDL_COPY_PROTECTOR(Object)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Object> Ptr;

        Object(const std::string & name, const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName);
        Object(const std::string & name);
        virtual ~Object() override;
        void init(const std::vector<std::string> & scope, const Documentation & doc, const std::string & loggerName);
        bool initialized() const;
        void setDefinitions(const std::vector<Definition::Ptr> & definitions);
        void setDefinitions(const std::list<Definition::Ptr> & definitions);
        const std::vector<Definition::Ptr> & definitions() const override;
		virtual const char * name() const override;
		virtual const std::vector<std::string> & scope() const override;
		virtual const Documentation & documentation() const override;
        const std::string & loggerName() const;
    };

	class PIDL_BACKEND__CLASS Module : public TopLevel, public DocumentationProvider
	{
		PIDL_COPY_PROTECTOR(Module)
		struct Priv;
		Priv * priv;
	public:
		typedef std::shared_ptr<Module> Ptr;
		Module(const std::string & name, const std::vector<TopLevel::Ptr> & elements, const Documentation & doc, const Info & info);
		Module(const std::string & name, const std::vector<TopLevel::Ptr> & elements, const Documentation & doc);
		virtual ~Module();
		virtual const char * name() const override;
		const std::vector<TopLevel::Ptr> & elements() const;

		virtual const Documentation & documentation() const override;
	};

}}

#endif // pidlBackend__language_h
