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

#ifndef pidlBackend__cppcodegen_h
#define pidlBackend__cppcodegen_h

#include "config.h"
#include <memory>
#include <ostream>
#include <vector>
#include <string>

#include "object.h"

namespace PIDL {
	
	namespace Language {

		class Element;
		class TopLevel;
		class Interface;
		class Function;
		class Type;
		class Property;
		class Object;
		class Method;
		class DocumentationProvider;
	}

	class ErrorCollector;

	class PIDL_BACKEND__CLASS CPPCodeGenContext
	{
		PIDL_COPY_PROTECTOR(CPPCodeGenContext)
		struct Priv;
		Priv * priv;
	public:
		enum class Mode
		{
			Declaration, Implementatinon, AllInOne
		};

		enum class Role
		{
			Server, Client
		};

		CPPCodeGenContext(short tab_length, char tab_char, std::ostream & o, Mode mode, Role role);
		~CPPCodeGenContext();

		template<typename T>
		std::ostream & operator << (const T & v) const
		{
			stream() << v;
			return stream();
		}

		std::ostream & operator * () const;

		std::ostream & stream() const;
		Mode mode() const;
		Role role() const;

		std::ostream & writeTabs(short code_deepness);
	};

#define PIDL_OBJECT_TYPE__CPP_CODEGEN_LOGGING "cpp_codegen_logging"
	class PIDL_BACKEND__CLASS CPPCodeGenLogging : public Object
	{
		PIDL_COPY_PROTECTOR(CPPCodeGenLogging)
		struct Priv;
		Priv * priv;

	protected:
		CPPCodeGenLogging();
		virtual ~CPPCodeGenLogging();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN_LOGGING; }

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

	class PIDL_BACKEND__CLASS CPPVoidLogging : public CPPCodeGenLogging
	{
		PIDL_COPY_PROTECTOR(CPPVoidLogging)
		struct Priv;
		Priv * priv;
	public:
		CPPVoidLogging();
		virtual ~CPPVoidLogging();

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


#define PIDL_OBJECT_TYPE__CPP_CODEGEN_DOCUMENTATION "cpp_codegen_documentation"
	class PIDL_BACKEND__CLASS CPPCodeGenDocumentation : public Object
	{
		PIDL_COPY_PROTECTOR(CPPCodeGenDocumentation)
		struct Priv;
		Priv * priv;

	protected:
		CPPCodeGenDocumentation();
		virtual ~CPPCodeGenDocumentation();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN_DOCUMENTATION; }

	public:
		enum Place
		{
			Before,
			After
		};
		virtual bool write(short code_deepness, CPPCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) = 0;
	};

	class PIDL_BACKEND__CLASS CPPVoidDocumentation : public CPPCodeGenDocumentation
	{
		PIDL_COPY_PROTECTOR(CPPVoidDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CPPVoidDocumentation();
		virtual ~CPPVoidDocumentation();

		virtual bool write(short code_deepness, CPPCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};

	class PIDL_BACKEND__CLASS CPPBasicDocumentation : public CPPCodeGenDocumentation
	{
		PIDL_COPY_PROTECTOR(CPPBasicDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CPPBasicDocumentation();
		virtual ~CPPBasicDocumentation();

		virtual bool write(short code_deepness, CPPCodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};


#define PIDL_OBJECT_TYPE__CPP_CODEGEN_HELPER "cpp_codegen_helper"

	class PIDL_BACKEND__CLASS CPPCodeGenHelper : public Object
	{
		PIDL_COPY_PROTECTOR(CPPCodeGenHelper)
		struct Priv;
		Priv * priv;

	public:
		enum class IncludeType
		{
			GLobal, Local
		};
		typedef std::pair<IncludeType, std::string> Include;

		CPPCodeGenHelper();
		virtual ~CPPCodeGenHelper();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN_HELPER; }

		virtual std::shared_ptr<CPPCodeGenLogging> logging() const = 0;
		virtual std::shared_ptr<CPPCodeGenDocumentation> documentation() const = 0;

		virtual std::vector<Include> includes() const = 0;

		virtual Include coreIncludePath() const = 0;

		virtual std::string getName(const Language::TopLevel * e) const = 0;
	};

	class PIDL_BACKEND__CLASS CPPBasicCodeGenHelper : public CPPCodeGenHelper
	{
		PIDL_COPY_PROTECTOR(CPPBasicCodeGenHelper)
		struct Priv;
		Priv * priv;
	public:
		CPPBasicCodeGenHelper(const std::vector<Include> & customIncludes = std::vector<Include>());
		virtual ~CPPBasicCodeGenHelper();

		virtual std::shared_ptr<CPPCodeGenLogging> logging() const override;
		virtual std::shared_ptr<CPPCodeGenDocumentation> documentation() const override;

		virtual std::vector<Include> includes() const override;

		virtual Include coreIncludePath() const override;

		virtual std::string getName(const Language::TopLevel * t) const override;

	};

#define PIDL_OBJECT_TYPE__CPP_CODEGEN "cpp_codegen"

	class PIDL_BACKEND__CLASS CPPCodeGen : public Object
	{
		PIDL_COPY_PROTECTOR(CPPCodeGen)
		struct Priv;
		Priv * priv;
	public:
		CPPCodeGen();
		virtual ~CPPCodeGen();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN; }

		bool generateIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);
		bool generateCode(Language::TopLevel * topLevel, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);

	protected:
		using Role = CPPCodeGenContext::Role;
		using Mode = CPPCodeGenContext::Mode;

		virtual CPPCodeGenHelper * helper() const = 0;

		virtual bool writeInclude(short code_deepness, CPPCodeGenContext * ctx, const CPPCodeGenHelper::Include & include, ErrorCollector & ec);

		virtual bool writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);
		virtual bool writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);
		virtual bool writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);

		virtual bool writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec);
		virtual bool writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec);
		virtual bool writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec);

		virtual bool writeIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;

		virtual bool writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writeFunctionBody(Language::Function * function, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeConstructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeDestructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);

		virtual bool writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec) = 0;
		virtual bool writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec) = 0;
		virtual bool writePropertyGetterBody(Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writePropertySetterBody(Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeConstructorBody(Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeDestructorBody(Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);

		virtual bool writeObjectBase(Language::Interface * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;

		bool writeType(Language::Type * type, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);
	};

}

#endif // pidlBackend__cppcodegen_h
