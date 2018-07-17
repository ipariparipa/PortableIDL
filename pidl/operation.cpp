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

#include "operation.h"

#include <pidlBackend/writer.h>
#include <pidlBackend/reader.h>

namespace PIDL {

	//struct Operation::Priv { }
	Operation::Operation() : priv(nullptr) { }
	Operation::~Operation() = default;


	struct OperationGroup::Priv 
	{
		Priv(const std::vector<std::shared_ptr<Operation>> & ops_) : ops(ops_)
		{ }

		std::vector<std::shared_ptr<Operation>> ops;
	};

	OperationGroup::OperationGroup(const std::vector<std::shared_ptr<Operation>> & ops) :
		Operation(),
		priv(new Priv(ops))
	{ }

	OperationGroup::~OperationGroup()
	{
		delete priv;
	}

	bool OperationGroup::run(ErrorCollector & ec)
	{
		for (auto & o : priv->ops)
			if (!o->run(ec))
				return false;

		return true;
	}


	struct Read::Priv
	{
		Priv(std::shared_ptr<Reader> & reader_) : reader(reader_)
		{ }

		std::shared_ptr<Reader> reader;
	};

	Read::Read(std::shared_ptr<Reader> & reader) :
		Operation(),
		priv(new Priv(reader))
	{ }

	Read::~Read()
	{
		delete priv;
	}

	bool Read::run(ErrorCollector & ec)
	{
		return priv->reader->read(ec);
	}


	struct Write::Priv
	{
		Priv(const std::shared_ptr<Reader> & reader_, const std::shared_ptr<Read> & read_op_, const std::shared_ptr<Writer> & writer_) : 
			reader(reader_), 
			read_op(read_op_), 
			writer(writer_)
		{ }

		std::shared_ptr<Reader> reader;
		std::shared_ptr<Read> read_op;
		std::shared_ptr<Writer> writer;
	};

	Write::Write(const std::shared_ptr<Reader> & reader, const std::shared_ptr<Read> & read_op, const std::shared_ptr<Writer> & writer) :
		Operation(),
		priv(new Priv(reader, read_op, writer))
	{ }

	Write::~Write()
	{
		delete priv;
	}

	bool Write::run(ErrorCollector & ec)
	{
		if (priv->read_op && !priv->read_op->run(ec))
			return false;

		return priv->writer->write(priv->reader.get(), ec);
	}

}
