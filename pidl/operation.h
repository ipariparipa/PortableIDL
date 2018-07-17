/*
    This file is part of pidl.

    pidl is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidl.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef pidl__operation_h
#define pidl__operation_h

#include "config.h"

#include <vector>
#include <memory>

namespace PIDL {

	namespace Language {
		class TopLevel;
	}

	class ErrorCollector;
	class Writer;
	class Reader;

	class Operation
	{
		PIDL_COPY_PROTECTOR(Operation)
		struct Priv;
		Priv * priv;
	protected:
		Operation();
	public:
		virtual ~Operation();
		virtual bool run(ErrorCollector & ec) = 0;
	};

	class OperationGroup : public Operation
	{
		PIDL_COPY_PROTECTOR(OperationGroup)
		struct Priv;
		Priv * priv;
	public:
		OperationGroup(const std::vector<std::shared_ptr<Operation>> & ops);
		virtual ~OperationGroup();
		virtual bool run(ErrorCollector & ec) override;
	};

	class Read : public Operation
	{
		PIDL_COPY_PROTECTOR(Read)
		struct Priv;
		Priv * priv;
	public:
		Read(std::shared_ptr<Reader> & reader);
		virtual ~Read();
		virtual bool run(ErrorCollector & ec) override;
	};

	class Write : public Operation
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

#endif // pidl__operation_h
