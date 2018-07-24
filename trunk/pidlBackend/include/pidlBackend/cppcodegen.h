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

namespace PIDL {
	
	namespace Language {

		class TopLevel;
		class Interface;
		class Function;
		class Type;
	}

	class ErrorCollector;

	class PIDL_BACKEND__CLASS CPPCodeGenContext
	{
		PIDL_COPY_PROTECTOR(CPPCodeGenContext)
		struct Priv;
		Priv * priv;
	public:
		enum class Nature
		{
			Declaration, Implementatinon, AllInOne
		};

		enum class Role
		{
			Server, Client
		};

		CPPCodeGenContext(std::ostream & o, Nature nature, Role role);
		~CPPCodeGenContext();

		template<typename T>
		std::ostream & operator << (const T & v) const
		{
			stream() << v;
			return stream();
		}

		std::ostream & stream() const;
		Nature nature() const;
		Role role() const;
	};

	class PIDL_BACKEND__CLASS CPPCodeGenLogging
	{
		PIDL_COPY_PROTECTOR(CPPCodeGenLogging)
		struct Priv;
		Priv * priv;

	protected:
		CPPCodeGenLogging();
		virtual ~CPPCodeGenLogging();

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

	class PIDL_BACKEND__CLASS CPPCodeGenHelper
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

		virtual short tabDefinition(char & ch) const = 0;
		virtual std::shared_ptr<CPPCodeGenLogging> logging() const = 0;

		virtual std::vector<Include> includes() const = 0;

		virtual Include coreIncludePath() const = 0;

		virtual std::string getName(const Language::TopLevel * t) const = 0;
	};

	class PIDL_BACKEND__CLASS CPPBasicCodegenHelper : public CPPCodeGenHelper
	{
		PIDL_COPY_PROTECTOR(CPPBasicCodegenHelper)
		struct Priv;
		Priv * priv;
	public:
		CPPBasicCodegenHelper(const std::vector<Include> & customIncludes = std::vector<Include>());
		virtual ~CPPBasicCodegenHelper();

		virtual short tabDefinition(char & ch) const override;
		virtual std::shared_ptr<CPPCodeGenLogging> logging() const override;

		virtual std::vector<Include> includes() const override;

		virtual Include coreIncludePath() const override;

		virtual std::string getName(const Language::TopLevel * t) const override;

	};

	class PIDL_BACKEND__CLASS CPPCodeGen
	{
		PIDL_COPY_PROTECTOR(CPPCodeGen)
		struct Priv;
		Priv * priv;
	public:
		CPPCodeGen();
		virtual ~CPPCodeGen();

		bool generateIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);
		bool generateCode(Language::TopLevel * topLevel, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);

	protected:
		using Role = CPPCodeGenContext::Role;
		using Nature = CPPCodeGenContext::Nature;

		virtual CPPCodeGenHelper * helper() const = 0;

		virtual bool writeInclude(short code_deepness, CPPCodeGenContext * ctx, const CPPCodeGenHelper::Include & include, ErrorCollector & ec);
		virtual bool writePrivateSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);
		virtual bool writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);
		virtual bool writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec);

		virtual bool writeIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writeFunctionBody(Language::Function * function, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeConstructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) = 0;
		bool writeType(Language::Type * type, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec);
	};

}

#endif // pidlBackend__cppcodegen_h
