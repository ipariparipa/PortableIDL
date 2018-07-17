
#include "include/pidlTools/cppwriter.h"
#include "include/pidlTools/cppcodegen.h"
#include "include/pidlTools/reader.h"

#include <algorithm>
#include <ostream>

namespace PIDL {

	struct CPPWriter::Priv
	{
		Priv(Mode mode_, Role role_, const std::shared_ptr<CPPCodeGen> & codegen_, std::ostream & o_, const std::string & name_) :
			mode(mode_),
			role(role_), 
			codegen(codegen_),
			o(o_),
			name(name_)
		{ }

		Mode mode;
		Role role;
		std::shared_ptr<CPPCodeGen> codegen;
		std::ostream & o;
		std::string name;
	};

	CPPWriter::CPPWriter(Mode mode, Role role, const std::shared_ptr<CPPCodeGen> & codegen, std::ostream & o, const std::string & name) :
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
				priv->o << "#ifndef " << guard << std::endl;
				priv->o << "#define " << guard << std::endl;

				CPPCodeGenContext ctx(priv->o, CPPCodeGenContext::Nature::Declaration, (CPPCodeGenContext::Role)priv->role);
				if (!priv->codegen->generateIncludes(0, &ctx, ec))
					return false;

				priv->o << std::endl;

				for (const auto & top_level : reader->topLevels())
				{
					if (!priv->codegen->generateCode(top_level.get(), 0, &ctx, ec))
						return false;
				}

				priv->o << "#endif // " << guard << std::endl;
			}
			break;
		case Mode::Source:
			{
				CPPCodeGenContext ctx(priv->o, CPPCodeGenContext::Nature::Implementatinon, (CPPCodeGenContext::Role)priv->role);
				if (!priv->codegen->generateIncludes(0, &ctx, ec))
					return false;

				priv->o << std::endl;

				for (const auto & top_level : reader->topLevels())
				{
					if (!priv->codegen->generateCode(top_level.get(), 0, &ctx, ec))
						return false;
				}
			}
			break;
		case Mode::Combo:
			{
				CPPCodeGenContext ctx(priv->o, CPPCodeGenContext::Nature::AllInOne, (CPPCodeGenContext::Role)priv->role);
				if (!priv->codegen->generateIncludes(0, &ctx, ec))
					return false;

				priv->o << std::endl;

				for (const auto & top_level : reader->topLevels())
				{
					if (!priv->codegen->generateCode(top_level.get(), 0, &ctx, ec))
						return false;
				}
				break;
			}
		}

		return true;
	}
	
}
