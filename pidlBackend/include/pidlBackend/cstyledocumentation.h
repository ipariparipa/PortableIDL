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

#ifndef pidlBackend__cstyledocumentation_h
#define pidlBackend__cstyledocumentation_h

#include "config.h"
#include <memory>

#include "object.h"

namespace PIDL {

	namespace Language {
		class DocumentationProvider;
	}

	class ErrorCollector;
	class CodeGenContext;

#define PIDL_OBJECT_TYPE__CSTYLE_DOCUMENTATION "cstyle_documentation"
	class PIDL_BACKEND__CLASS CStyleDocumentation : public Object
	{
		PIDL_COPY_PROTECTOR(CStyleDocumentation)
		struct Priv;
		Priv * priv;

	protected:
		CStyleDocumentation();
		virtual ~CStyleDocumentation();

		virtual const char * type() const override { return PIDL_OBJECT_TYPE__CSTYLE_DOCUMENTATION; }

	public:
		enum Place
		{
			Before,
			After
		};
		virtual bool write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) = 0;
	};

	class PIDL_BACKEND__CLASS CStyleVoidDocumentation : public CStyleDocumentation
	{
		PIDL_COPY_PROTECTOR(CStyleVoidDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CStyleVoidDocumentation();
		virtual ~CStyleVoidDocumentation();

		virtual bool write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};

	class PIDL_BACKEND__CLASS CStyleBasicDocumentation : public CStyleDocumentation
	{
		PIDL_COPY_PROTECTOR(CStyleBasicDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CStyleBasicDocumentation();
		virtual ~CStyleBasicDocumentation();

		virtual bool write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};

	class PIDL_BACKEND__CLASS CStyleDotNetDocumentation : public CStyleDocumentation
	{
		PIDL_COPY_PROTECTOR(CStyleDotNetDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CStyleDotNetDocumentation();
		virtual ~CStyleDotNetDocumentation();

		virtual bool write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};

	class PIDL_BACKEND__CLASS CStyleDoxygenDocumentation : public CStyleDocumentation
	{
		PIDL_COPY_PROTECTOR(CStyleDoxygenDocumentation)
		struct Priv;
		Priv * priv;

	public:
		CStyleDoxygenDocumentation();
		virtual ~CStyleDoxygenDocumentation();

		virtual bool write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec) override;
	};

}

#endif // pidlBackend__cstyledocumentation_h
