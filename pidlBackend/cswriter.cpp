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

#include "include/pidlBackend/cswriter.h"
#include "include/pidlBackend/cscodegen.h"
#include "include/pidlBackend/reader.h"

#include <algorithm>
#include <ostream>

namespace PIDL {

	struct CSWriter::Priv
	{
		Priv(Role role_, const std::shared_ptr<CSCodeGen> & codegen_, const std::shared_ptr<std::ostream> & o_) :
			role(role_),
			codegen(codegen_),
			o(o_)
		{ }

		Role role;
		std::shared_ptr<CSCodeGen> codegen;
		std::shared_ptr<std::ostream> o;
	};

	CSWriter::CSWriter(Role role, const std::shared_ptr<CSCodeGen> & codegen, const std::shared_ptr<std::ostream> & o) :
		Writer(),
		priv(new Priv(role, codegen, o))
	{ }

	CSWriter::~CSWriter()
	{
		delete priv;
	}

	bool CSWriter::write(Reader * reader, ErrorCollector & ec)
	{
		CSCodeGenContext ctx(1, '\t', (*priv->o), (CSCodeGenContext::Role)priv->role);
		if (!priv->codegen->generateUsings(0, &ctx, ec))
			return false;

		(*priv->o) << std::endl;

		for (const auto & top_level : reader->topLevels())
		{
			if (!priv->codegen->generateCode(top_level.get(), 0, &ctx, ec))
				return false;
		}

		return true;
	}

}
