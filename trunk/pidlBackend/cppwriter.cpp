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

#include "include/pidlBackend/cppwriter.h"
#include "include/pidlBackend/cppcodegen.h"
#include "include/pidlBackend/reader.h"

#include <algorithm>
#include <ostream>

namespace PIDL {

	struct CPPWriter::Priv
	{
		Priv(Mode mode_, Role role_, const std::shared_ptr<CPPCodeGen> & codegen_, const std::shared_ptr<std::ostream> & o_, const std::string & name_) :
			mode(mode_),
			role(role_), 
			codegen(codegen_),
			o(o_),
			name(name_)
		{ }

		Mode mode;
		Role role;
		std::shared_ptr<CPPCodeGen> codegen;
		std::shared_ptr<std::ostream> o;
		std::string name;
	};

	CPPWriter::CPPWriter(Mode mode, Role role, const std::shared_ptr<CPPCodeGen> & codegen, const std::shared_ptr<std::ostream> & o, const std::string & name) :
		Writer(), 
		priv(new Priv(mode, role, codegen, o, name))
	{ }

	CPPWriter::~CPPWriter()
	{
		delete priv;
	}

	bool CPPWriter::write(Reader * reader, ErrorCollector & ec)
	{
		switch (priv->mode)
		{
		case Mode::Include:
			{
				auto guard = priv->name;
				std::replace(guard.begin(), guard.end(), '.', '_');
				std::replace(guard.begin(), guard.end(), '/', '_');
				std::replace(guard.begin(), guard.end(), '\\', '_');
				guard = "__" + guard + "__";
				(*priv->o) << "#ifndef " << guard << std::endl;
				(*priv->o) << "#define " << guard << std::endl;

				std::unique_ptr<CPPCodeGenContext> ctx(priv->codegen->createContext(1, '\t', (*priv->o), (CPPCodeGenContext::Role)priv->role, CPPCodeGenContext::Mode::Declaration));
				if (!priv->codegen->generateIncludes(0, ctx.get(), ec))
					return false;

				(*priv->o) << std::endl;

				for (const auto & top_level : reader->topLevels())
				{
					if (!priv->codegen->generateCode(top_level.get(), 0, ctx.get(), ec))
						return false;
				}

				(*priv->o) << "#endif // " << guard << std::endl;
			}
			break;
		case Mode::Source:
			{
				std::unique_ptr<CPPCodeGenContext> ctx(priv->codegen->createContext(1, '\t', (*priv->o), (CPPCodeGenContext::Role)priv->role, CPPCodeGenContext::Mode::Implementatinon));
				if (!priv->codegen->generateIncludes(0, ctx.get(), ec))
					return false;

				bool has_error = false;
				for (auto & top_level : reader->topLevels())
					if (!ctx->prebuild(top_level.get(), ec))
						has_error = true;

				if (has_error)
					return false;


				(*priv->o) << std::endl;

				for (const auto & top_level : reader->topLevels())
				{
					if (!priv->codegen->generateCode(top_level.get(), 0, ctx.get(), ec))
						return false;
				}
			}
			break;
		case Mode::Combo:
			{
				std::unique_ptr<CPPCodeGenContext> ctx(priv->codegen->createContext(1, '\t', (*priv->o), (CPPCodeGenContext::Role)priv->role, CPPCodeGenContext::Mode::AllInOne));
				if (!priv->codegen->generateIncludes(0, ctx.get(), ec))
					return false;

				(*priv->o) << std::endl;

				for (const auto & top_level : reader->topLevels())
				{
					if (!priv->codegen->generateCode(top_level.get(), 0, ctx.get(), ec))
						return false;
				}
				break;
			}
		}

		return true;
	}
	
}
