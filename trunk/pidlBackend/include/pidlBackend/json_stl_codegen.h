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

#ifndef pidlBackend__json_stl_codegen_H
#define pidlBackend__json_stl_codegen_H

#include "cppcodegen.h"

namespace PIDL
{
	class PIDL_BACKEND__CLASS JSON_STL_CodeGen : public CPPCodeGen
	{
		PIDL_COPY_PROTECTOR(JSON_STL_CodeGen)
		struct Priv;
		Priv * priv;
	public:
		JSON_STL_CodeGen(const std::shared_ptr<CPPCodeGenHelper> & helper);
		JSON_STL_CodeGen();
		virtual ~JSON_STL_CodeGen();

	protected:
		using Role = CPPCodeGenContext::Role;
		using Mode = CPPCodeGenContext::Mode;

		virtual CPPCodeGenHelper * helper() const override;

		virtual bool writePublicSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) override;
		virtual bool writeProtectedSection(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) override;

		virtual bool writeIncludes(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeAliases(short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) override;
		virtual bool writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) override;
		virtual bool writeFunctionBody(Language::FunctionVariant * function, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeConstructorBody(Language::Interface * intf, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;

		virtual bool writeInvoke(short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec) override;
		virtual bool writePrivateMembers(short code_deepness, CPPCodeGenContext * ctx, Language::Object * object, ErrorCollector & ec) override;
		virtual bool writePropertyGetterBody(Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writePropertySetterBody(Language::Property * property, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeConstructorBody(Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeDestructorBody(Language::Object * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;

		virtual bool writeObjectBase(Language::Interface * object, short code_deepness, CPPCodeGenContext * ctx, ErrorCollector & ec) override;
	};

}

#endif // pidlBackend__json_stl_codegen_H

