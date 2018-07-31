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

#ifndef pidlBackend__json_cscodegen_h
#define pidlBackend__json_cscodegen_h

#include "cscodegen.h"

namespace PIDL
{
	class PIDL_BACKEND__CLASS JSON_CSCodeGen : public CSCodeGen
	{
		PIDL_COPY_PROTECTOR(JSON_CSCodeGen)
		struct Priv;
		Priv * priv;
	public:
		JSON_CSCodeGen(const std::shared_ptr<CSCodeGenHelper> & helper);
		JSON_CSCodeGen();
		virtual ~JSON_CSCodeGen();

	protected:
		virtual CSCodeGenHelper * helper() const override;
		virtual bool writeUsings(short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) override;
		virtual bool writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Interface * intf, ErrorCollector & ec) override;
		virtual bool writeFunctionBody(Language::Function * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeConstructorBody(Language::Interface * intf, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) override;

		virtual bool writeMembers(short code_deepness, CSCodeGenContext * ctx, Language::Object * obj, ErrorCollector & ec) override;
		virtual bool writeFunctionBody(Language::Method * function, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeConstructorBody(Language::Object * obj, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeDestructorBody(Language::Object * obj, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writeInvoke(short code_deepness, CSCodeGenContext * ctx, Language::Object * intf, ErrorCollector & ec) override;
		virtual bool writePropertyGetterBody(Language::Property * prop, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) override;
		virtual bool writePropertySetterBody(Language::Property * prop, short code_deepness, CSCodeGenContext * ctx, ErrorCollector & ec) override;
	};

}

#endif // pidlBackend__json_cscodegen_h
