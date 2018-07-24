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

namespace PIDL {

	namespace Language {

		class TopLevel;
		class Interface;
		class Function;
		class Type;
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

		CSCodeGenContext(std::ostream & o, Role role);
		~CSCodeGenContext();

		template<typename T>
		std::ostream & operator << (const T & v) const
		{
			stream() << v;
			return stream();
		}

		std::ostream & stream() const;
		Role role() const;
	};

	class PIDL_BACKEND__CLASS CSCodeGenLogging
	{
		PIDL_COPY_PROTECTOR(CSCodeGenLogging)
		struct Priv;
		Priv * priv;

	protected:
		CSCodeGenLogging();
		virtual ~CSCodeGenLogging();

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

	class PIDL_BACKEND__CLASS CSCodeGenHelper
	{
		PIDL_COPY_PROTECTOR(CSCodeGenHelper)
		struct Priv;
		Priv * priv;

	public:
		CSCodeGenHelper();
		virtual ~CSCodeGenHelper();

		virtual short tabDefinition(char & ch) const = 0;
		virtual std::shared_ptr<CSCodeGenLogging> logging() const = 0;

		virtual std::string getName(const Language::TopLevel * t) const = 0;
	};

	class PIDL_BACKEND__CLASS CSBasicCodegenHelper : public CSCodeGenHelper
	{
		PIDL_COPY_PROTECTOR(CSBasicCodegenHelper)
		struct Priv;
		Priv * priv;
	public:
		CSBasicCodegenHelper();
		virtual ~CSBasicCodegenHelper();

		virtual short tabDefinition(char & ch) const override;
		virtual std::shared_ptr<CSCodeGenLogging> logging() const override;

		virtual std::string getName(const Language::TopLevel * t) const override;
	};

	class PIDL_BACKEND__CLASS CSCodeGen
	{
		PIDL_COPY_PROTECTOR(CSCodeGen)
		struct Priv;
		Priv * priv;
	public:

		CSCodeGen();
		virtual ~CSCodeGen();

		bool generateUsings(short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec);
		bool generateCode(Language::TopLevel * topLevel, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec);

	protected:
		using Role = CSCodeGenContext::Role;

		virtual CSCodeGenHelper * helper() const = 0;

		virtual bool writeUsings(short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) = 0;
		virtual bool writeFunctionBody(Language::Function * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;
		virtual bool writeConstructorBody(Language::Interface * intf, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) = 0;

		bool writeType(Language::Type * type, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec);
	};
}

#endif // pidlBackend__cscodegen_h
