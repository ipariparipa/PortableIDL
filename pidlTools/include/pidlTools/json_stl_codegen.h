/*
    This file is part of pidlTools.

    pidlTools is free software: you can redistribute it and/or modify
    it under the terms of the Lesser GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    pidlTools is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with pidlTools.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef pidlTools__json_stl_codegen_H
#define pidlTools__json_stl_codegen_H

#include "cppcodegen.h"

namespace PIDL
{
	class JSON_STL_CodeGen : public CPPCodeGen
	{
		PIDL_COPY_PROTECTOR(JSON_STL_CodeGen)
		struct Priv;
		Priv * priv;
	public:
		JSON_STL_CodeGen(const std::shared_ptr<CPPCodeGenHelper> & helper);
		virtual ~JSON_STL_CodeGen();

	protected:
		virtual CPPCodeGenHelper * helper() const override;
		virtual bool writeIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) override;
		virtual bool writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) override;
		virtual bool writeFunctionBody(Language::Function * function, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeConstructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
	};

}

#endif // pidlTools__json_stl_codegen_H

