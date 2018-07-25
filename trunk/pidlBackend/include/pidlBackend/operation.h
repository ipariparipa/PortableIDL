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

#ifndef pidlBackend__operation_h
#define pidlBackend__operation_h

#include "config.h"

#include "object.h"

#include <vector>
#include <memory>

namespace PIDL {

	namespace Language {
		class TopLevel;
	}

	class ErrorCollector;
	class Writer;
	class Reader;

#define PIDL_OBJECT_TYPE__OPERATION "operation"

	class PIDL_BACKEND__CLASS Operation : public Object
	{
		PIDL_COPY_PROTECTOR(Operation)
		struct Priv;
		Priv * priv;
	protected:
		Operation();
	public:
		virtual ~Operation();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__OPERATION; }

		virtual bool run(ErrorCollector & ec) = 0;
	};

	class PIDL_BACKEND__CLASS OperationGroup : public Operation
	{
		PIDL_COPY_PROTECTOR(OperationGroup)
		struct Priv;
		Priv * priv;
	public:
		OperationGroup(const std::vector<std::shared_ptr<Operation>> & ops);

		virtual ~OperationGroup();

		virtual bool run(ErrorCollector & ec) override;
	};

	class PIDL_BACKEND__CLASS Read : public Operation
	{
		PIDL_COPY_PROTECTOR(Read)
		struct Priv;
		Priv * priv;
	public:
		Read(std::shared_ptr<Reader> & reader);

		virtual ~Read();

		std::shared_ptr<Reader> reader() const;

		virtual bool run(ErrorCollector & ec) override;
	};

	class PIDL_BACKEND__CLASS Write : public Operation
	{
		PIDL_COPY_PROTECTOR(Write)
		struct Priv;
		Priv * priv;
	public:
		Write(const std::shared_ptr<Reader> & reader, const std::shared_ptr<Read> & read_op, const std::shared_ptr<Writer> & writer);

		virtual ~Write();

		virtual bool run(ErrorCollector & ec) override;
	};

}

#endif // pidlBackend__operation_h
