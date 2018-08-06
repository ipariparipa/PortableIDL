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

#ifndef pidlBackend__cscodegen_h
#define pidlBackend__cscodegen_h

#include "config.h"

#include <ostream>
#include <memory>

#include "object.h"

namespace PIDL {

	namespace Language {

		class TopLevel;
		class Interface;
		class Function;
		class Type;
		class Object;
		class Method;
		class Property;
		class DocumentationProvider;	
	}

	class ErrorCollector;

	class PIDL_BACKEND__CLASS CSCodeGenContext
	{
		PIDL_COPY_PROTECTOR(CSCodeGenContext)
		struct Priv;
		Priv * priv;
	public:
		enum class Role
		{
			Server, Client
		};

		CSCodeGenContext(short tab_length, char tab_char, std::ostream & o, Role role);
		~CSCodeGenContext();

		template<typename T>
		std::ostream & operator << (const T & v) const
		{
			stream() << v;
			return stream();
		}

		std::ostream & operator * () const;

		std::ostream & stream() const;
		Role role() const;

		std::ostream & writeTabs(short code_deepness);
	};

#define PIDL_OBJECT_TYPE__CS_CODEGEN_LOGGING "cs_codegen_logging"

	class PIDL_BACKEND__CLASS CSCodeGenLogging : public Object
	{
		PIDL_COPY_PROTECTOR(CSCodeGenLogging)
		struct Priv;
		Priv * priv;

	protected:
		CSCodeGenLogging();
		virtual ~CSCodeGenLogging();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CS_CODEGEN_LOGGING; }

	public:
		virtual std::string initLogger(const std::string & scope) const = 0;
		virtual std::string loggerType() const = 0;
		virtual std::string loggingStart(const std::string & logger) const = 0;
		virtual std::string loggingAssert(const std::string & logger, const std::string & expression, const std::string & message) const = 0;
		virtual std::string loggingTrace(const std::string & logger, const std::string & message) const = 0;
		virtual std::string loggingDebug(const std::string & logger, const std::string & message) const = 0;
		virtual std::string loggingInfo(const std::string & logger, const std::string & message) const = 0;
		virtual std::string loggingWarning(const std::string & logger, const std::string & message) const = 0;
		virtual std::string loggingError(const std::string & logger, const std::string & message) const = 0;
		virtual std::string loggingFatal(const std::string & logger, const std::string & message) const = 0;
	};


	class PIDL_BACKEND__CLASS CSVoidLogging : public CSCodeGenLogging
	{
		PIDL_COPY_PROTECTOR(CSVoidLogging)
		struct Priv;
		Priv * priv;
	public:
		CSVoidLogging();
		virtual ~CSVoidLogging();

		virtual std::string initLogger(const std::string & scope) const override;
		virtual std::string loggerType() const override;
		virtual std::string loggingStart(const std::string & logger) const override;
		virtual std::string loggingAssert(const std::string & logger, const std::string & expression, const std::string & message) const override;
		virtual std::string loggingTrace(const std::string & logger, const std::string & message) const override;
		virtual std::string loggingDebug(const std::string & logger, const std::string & message) const override;
		virtual std::string loggingInfo(const std::string & logger, const std::string & message) const override;
		virtual std::string loggingWarning(const std::string & logger, const std::string & message) const override;
		virtual std::string loggingError(const std::string & logger, const std::string & message) const override;
		virtual std::string loggingFatal(const std::string & logger, const std::string & message) const override;
	};


#define PIDL_OBJECT_TYPE__CS_CODEGEN_DOCUMENTATION "cs_codegen_documentation"
	class PIDL_BACKEND__CLASS CSCodeGenDocumentation : public Object
	{
		PIDL_COPY_PROTECTOR(CSCodeGenDocumentation)
		struct Priv;
		Priv * priv;

	protected:
		CSCodeGenDocumentation();
		virtual ~CSCodeGenDocumentation();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CS_CODEGEN_DOCUMENTATION; }

	public:
		enum Place
		{
			Before,
			After
		};
		virtual bool write(short code_deepness, CSCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) = 0;
	};

	class PIDL_BACKEND__CLASS CSVoidDocumentation : public CSCodeGenDocumentation
	{
		PIDL_COPY_PROTECTOR(CSVoidDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CSVoidDocumentation();
		virtual ~CSVoidDocumentation();

		virtual bool write(short code_deepness, CSCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};

	class PIDL_BACKEND__CLASS CSBasicDocumentation : public CSCodeGenDocumentation
	{
		PIDL_COPY_PROTECTOR(CSBasicDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CSBasicDocumentation();
		virtual ~CSBasicDocumentation();

		virtual bool write(short code_deepness, CSCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};

	class PIDL_BACKEND__CLASS CSdotNetDocumentation : public CSCodeGenDocumentation
	{
		PIDL_COPY_PROTECTOR(CSdotNetDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CSdotNetDocumentation();
		virtual ~CSdotNetDocumentation();

		virtual bool write(short code_deepness, CSCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};


#define PIDL_OBJECT_TYPE__CS_CODEGEN_HELPER "cs_codegen_helper"

	class PIDL_BACKEND__CLASS CSCodeGenHelper : public Object
	{
		PIDL_COPY_PROTECTOR(CSCodeGenHelper)
		struct Priv;
		Priv * priv;

	public:
		CSCodeGenHelper();
		virtual ~CSCodeGenHelper();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CS_CODEGEN_HELPER; }

		virtual std::shared_ptr<CSCodeGenLogging> logging() const = 0;
		virtual std::shared_ptr<CSCodeGenDocumentation> documentation() const = 0;

		virtual std::string getName(const Language::TopLevel * t) const = 0;
	};

	class PIDL_BACKEND__CLASS CSBasicCodeGenHelper : public CSCodeGenHelper
	{
		PIDL_COPY_PROTECTOR(CSBasicCodeGenHelper)
		struct Priv;
		Priv * priv;
	public:
		CSBasicCodeGenHelper();
		virtual ~CSBasicCodeGenHelper();

		virtual std::shared_ptr<CSCodeGenLogging> logging() const override;
		virtual std::shared_ptr<CSCodeGenDocumentation> documentation() const override;

		virtual std::string getName(const Language::TopLevel * t) const override;
	};

#define PIDL_OBJECT_TYPE__CS_CODEGEN "cs_codegen"

	class PIDL_BACKEND__CLASS CSCodeGen : public Object
	{
		PIDL_COPY_PROTECTOR(CSCodeGen)
		struct Priv;
		Priv * priv;
	public:

		CSCodeGen();
		virtual ~CSCodeGen();

		bool generateUsings(short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec);
		bool generateCode(Language::TopLevel * topLevel, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec);

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CS_CODEGEN; }

	protected:
		using Role = CSCodeGenContext::Role;

		virtual CSCodeGenHelper * helper() const = 0;

		virtual bool writeUsings(short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writeFunctionBody(Language::Function * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeConstructorBody(Language::Interface * intf, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;

		virtual bool writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec) = 0;
		virtual bool writeFunctionBody(Language::Method * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeConstructorBody(Language::Object * obj, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeDestructorBody(Language::Object * obj, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Object * intf, ErrorCollector & ec) = 0;
		virtual bool writePropertyGetterBody(Language::Property * prop, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writePropertySetterBody(Language::Property * prop, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;

		bool writeType(Language::Type * type, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec);
	};
}

#endif // pidlBackend__cscodegen_h
