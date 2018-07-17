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

namespace PIDL { namespace Language {

	class PIDL_BACKEND__CLASS Element
	{
		PIDL_COPY_PROTECTOR(Element)
		struct Priv;
		Priv * priv;
	public:
		Element();
		virtual ~Element();

		virtual const char * name() const = 0;
	};

	class PIDL_BACKEND__CLASS Definition : public Element
	{
		PIDL_COPY_PROTECTOR(Definition)
		struct Priv;
		Priv * priv;
	public:
		Definition();
		virtual ~Definition();
	};

	class PIDL_BACKEND__CLASS Type : public Element
	{
		PIDL_COPY_PROTECTOR(Type)
		struct Priv;
		Priv * priv;
	public:
		Type();
		virtual ~Type();
	};

	class PIDL_BACKEND__CLASS Variable : public Element
	{
		PIDL_COPY_PROTECTOR(Variable)
		struct Priv;
		Priv * priv;
	public:
		Variable(const std::shared_ptr<Type> & type, const std::string & name);
		virtual ~Variable();

		virtual const char * name() const override;
		std::shared_ptr<Type> type() const;
	};

	class PIDL_BACKEND__CLASS TypeDefinition : public Type, public Definition
	{
		PIDL_COPY_PROTECTOR(TypeDefinition)
		struct Priv;
		Priv * priv;
	public:
		TypeDefinition(const std::string & name, const std::shared_ptr<Type> & type);
		virtual ~TypeDefinition();

		virtual const char * name() const override;
		virtual std::shared_ptr<Type> type() const;
	};

	class PIDL_BACKEND__CLASS ComplexType : public Type
	{
		PIDL_COPY_PROTECTOR(ComplexType)
		struct Priv;
		Priv * priv;
	public:
		ComplexType();
		virtual ~ComplexType();
	};

	class PIDL_BACKEND__CLASS Blob : public ComplexType
	{
		PIDL_COPY_PROTECTOR(Blob)
		struct Priv;
		Priv * priv;
	public:
		Blob();
		virtual ~Blob();
		virtual const char * name() const override { return "blob"; }
	};

	class PIDL_BACKEND__CLASS Generic : public ComplexType
	{
		PIDL_COPY_PROTECTOR(Generic)
		struct Priv;
		Priv * priv;
	public:
		Generic(const std::shared_ptr<Type> & type);
		virtual ~Generic();
		std::shared_ptr<Type> type() const;
	};

	class PIDL_BACKEND__CLASS Nullable : public Generic
	{
		PIDL_COPY_PROTECTOR(Nullable)
		struct Priv;
		Priv * priv;
	public:
		Nullable(const std::shared_ptr<Type> & type);
		virtual ~Nullable();
		virtual const char * name() const override { return "nullable"; }
	};

	class PIDL_BACKEND__CLASS Array : public Generic
	{
		PIDL_COPY_PROTECTOR(Array)
		struct Priv;
		Priv * priv;
	public:
		Array(const std::shared_ptr<Type> & type);
		virtual ~Array();
		virtual const char * name() const override { return "array"; }
	};

	class PIDL_BACKEND__CLASS Structure : public ComplexType
	{
		PIDL_COPY_PROTECTOR(Structure)
		struct Priv;
		Priv * priv;
	public:
		class Member : public Variable
		{
			PIDL_COPY_PROTECTOR(Member)
			struct Priv;
			Priv * priv;
		public:
			Member(const std::shared_ptr<Type> & type, const std::string & name);
			virtual ~Member();
		};

		Structure(const std::vector<std::shared_ptr<Member>> & members);
		Structure(const std::list<std::shared_ptr<Member>> & members);
		virtual ~Structure();

		virtual const char * name() const override { return "structure"; }
		const std::vector<std::shared_ptr<Member>> & members() const;
	};

	class PIDL_BACKEND__CLASS NativeType : public Type
	{
		PIDL_COPY_PROTECTOR(NativeType)
		struct Priv;
		Priv * priv;
	public:
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
		EmbeddedType();
		virtual ~EmbeddedType();
	};

	class PIDL_BACKEND__CLASS String : public EmbeddedType
	{
		PIDL_COPY_PROTECTOR(String)
		struct Priv;
		Priv * priv;
	public:
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
		DateTime();
		virtual ~DateTime();

		virtual const char * name() const override { return "datetime"; }
	};

	class PIDL_BACKEND__CLASS Void : public Type
	{
		PIDL_COPY_PROTECTOR(Void)
		struct Priv;
		Priv * priv;
	public:
		Void();
		virtual ~Void();

		virtual const char * name() const override { return "void"; }
	};

	class PIDL_BACKEND__CLASS Function : public Definition
	{
		PIDL_COPY_PROTECTOR(Function)
		struct Priv;
		Priv * priv;
	public:

		class PIDL_BACKEND__CLASS Argument : public Variable
		{
			PIDL_COPY_PROTECTOR(Argument)
			struct Priv;
			Priv * priv;
		public:
			enum class Direction {
				In, Out, InOut
			};

			Argument(const std::shared_ptr<Type> & type, const std::string & name, Direction direction);
			virtual ~Argument();

			Direction direction() const;
		};

		Function(const std::shared_ptr<Type> & returnType, const std::vector<std::string> & scope, const std::string & name, const std::vector<std::shared_ptr<Argument>> & arguments);
		Function(const std::shared_ptr<Type> & returnType, const std::vector<std::string> & scope, const std::string & name, const std::list<std::shared_ptr<Argument>> & arguments);
		virtual ~Function();

		virtual const char * name() const override;
		const std::vector<std::string> & scope() const;
		const std::vector<std::shared_ptr<Argument>> & arguments() const;
		std::shared_ptr<Type> returnType() const;
	};

	class PIDL_BACKEND__CLASS TopLevel : public Element
	{
		PIDL_COPY_PROTECTOR(TopLevel)
		struct Priv;
		Priv * priv;
	public:
		TopLevel();
		virtual ~TopLevel();
	};

	class PIDL_BACKEND__CLASS Interface : public TopLevel
	{
		PIDL_COPY_PROTECTOR(Interface)
		struct Priv;
		Priv * priv;
	public:
		Interface(const std::string & name, const std::vector<std::shared_ptr<Definition>> & definitions);
		Interface(const std::string & name, const std::list<std::shared_ptr<Definition>> & definitions);
		virtual ~Interface();
		const std::vector<std::shared_ptr<Definition>> & definitions() const;
		virtual const char * name() const override;
	};

	class PIDL_BACKEND__CLASS Module : public TopLevel
	{
		PIDL_COPY_PROTECTOR(Module)
		struct Priv;
		Priv * priv;
	public:
		Module(const std::string &name, const std::vector<std::shared_ptr<TopLevel>> & elements);
		virtual ~Module();
		virtual const char * name() const override;
		const std::vector<std::shared_ptr<TopLevel>> & elements() const;
	};

}}

#endif // pidlBackend__language_h
