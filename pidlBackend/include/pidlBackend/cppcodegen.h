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
#include "codegencontext.h"

namespace PIDL {
	
	namespace Language {

		class Element;
		class TopLevel;
		class Interface;
		class Function;
		class FunctionVariant;
		class Type;
		class Property;
		class Object;
		class Method;
		class DocumentationProvider;
	}

	class ErrorCollector;
	class CStyleDocumentation;

    enum class IncludeType
    {
        GLobal, Local
    };
    typedef std::pair<IncludeType, std::string> Include;

	class PIDL_BACKEND__CLASS CPPCodeGenContext : public CodeGenContext
	{
		PIDL_COPY_PROTECTOR(CPPCodeGenContext)
		struct Priv;
		Priv * priv;
	public:
		enum class Mode
		{
			Declaration, Implementatinon, AllInOne
		};

		CPPCodeGenContext(short tab_length, char tab_char, std::ostream & o, Role role, Mode mode);
		virtual ~CPPCodeGenContext();

		Mode mode() const;
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
        virtual std::vector<Include> includes() const = 0;
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

        virtual std::vector<Include> includes() const override;
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

#define PIDL_OBJECT_TYPE__CPP_CODEGEN_HELPER "cpp_codegen_helper"

	class PIDL_BACKEND__CLASS CPPCodeGenHelper : public Object
	{
		PIDL_COPY_PROTECTOR(CPPCodeGenHelper)
		struct Priv;
		Priv * priv;

    public:
		CPPCodeGenHelper();
		virtual ~CPPCodeGenHelper();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN_HELPER; }

		virtual std::shared_ptr<CPPCodeGenLogging> logging() const = 0;
		virtual std::shared_ptr<CStyleDocumentation> documentation() const = 0;

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
		virtual std::shared_ptr<CStyleDocumentation> documentation() const override;

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
		using Role = CPPCodeGenContext::Role;
		using Mode = CPPCodeGenContext::Mode;

		CPPCodeGen();
		virtual ~CPPCodeGen();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CPP_CODEGEN; }

		bool generateIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);
		bool generateCode(Language::TopLevel * topLevel, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);

		virtual CPPCodeGenContext * createContext(short tab_length, char tab_char, std::ostream & o, Role role, Mode mode) const;

	protected:
		virtual CPPCodeGenHelper * helper() const = 0;

        virtual bool writeInclude(short code_deepness, CPPCodeGenContext * ctx, const Include & include, ErrorCollector & ec);

		virtual bool writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);
		virtual bool writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);
		virtual bool writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);

		virtual bool writePrivateSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec);
		virtual bool writePublicSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec);
		virtual bool writeProtectedSection(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec);

		virtual bool writeIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;

		virtual bool writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writeFunctionBody(Language::Interface * intf, Language::FunctionVariant * function, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeConstructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeDestructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);

		virtual bool writeInvoke(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec) = 0;
		virtual bool writePrivateMembers(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec) = 0;
		virtual bool writePropertyGetterBody(Language::Interface * intf, Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writePropertySetterBody(Language::Interface * intf, Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeConstructorBody(Language::Interface * intf, Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeDestructorBody(Language::Interface * intf, Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);

		virtual bool writeObjectBase(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;

		bool writeType(Language::Type * type, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);
	};

}

#endif // pidlBackend__cppcodegen_h
