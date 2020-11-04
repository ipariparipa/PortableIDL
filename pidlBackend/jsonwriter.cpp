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

#include "include/pidlBackend/jsonwriter.h"
#include "include/pidlBackend/reader.h"
#include "include/pidlBackend/language.h"

#include <pidlCore/errorcollector.h>
#include <pidlCore/jsontools.h>

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

namespace PIDL
{

	struct JSONWriter::Priv
	{
		Priv(const std::shared_ptr<std::ostream> & s, bool hr_) : o(s), hr(hr_) { }

		static void addName(rapidjson::Document & doc, rapidjson::Value & r, const char * str)
		{
			JSONTools::addValue(doc, r, "name", str);
		}

		static void addNature(rapidjson::Document & doc, rapidjson::Value & r, const char * str)
		{
			JSONTools::addValue(doc, r, "nature", str);
		}

		static void addDocumentation(rapidjson::Document & doc, rapidjson::Value & r, const Language::DocumentationProvider::Documentation & d)
		{
			if (!d.brief.length() && !d.details.size())
				return;

			if (!d.details.size() && (long)d.brief.find('\n') == -1)
				JSONTools::addValue(doc, r, "documentation", d.brief);
			else
			{
				rapidjson::Value docu_v(rapidjson::kObjectType);
				JSONTools::addValue(doc, docu_v, "brief", d.brief);
				for (auto & det : d.details)
				{
					switch (det.first)
					{
					case Language::DocumentationProvider::Documentation::Description:
						JSONTools::addValue(doc, docu_v, "description", det.second);
						break;
					case Language::DocumentationProvider::Documentation::Return:
						JSONTools::addValue(doc, docu_v, "return", det.second);
						break;
					}
				}

				JSONTools::addValue(doc, r, "documentation", docu_v);
			}
		}

		static void addType(rapidjson::Document & doc, rapidjson::Value & r, const std::vector<Language::Type::Ptr> & types)
		{
			rapidjson::Value v(rapidjson::kArrayType);
			for (auto & t : types)
				v.PushBack(createType(doc, t), doc.GetAllocator());
			JSONTools::addValue(doc, r, "types", v);
		}

		static void addType(rapidjson::Document & doc, rapidjson::Value & r, const Language::Type::Ptr & t)
		{
			auto v = createType(doc, t);
			JSONTools::addValue(doc, r, "type", v);
		}

		static rapidjson::Value createType(rapidjson::Document & doc, const Language::Type::Ptr & t)
		{
			if (dynamic_cast<Language::Structure*>(t.get()))
			{
				rapidjson::Value v(rapidjson::kObjectType);
				addName(doc, v, t->name());

				rapidjson::Value m(rapidjson::kArrayType);
				for (auto & memb : dynamic_cast<Language::Structure*>(t.get())->members())
				{
					rapidjson::Value e(rapidjson::kObjectType);
					addName(doc, e, memb->name());
					addType(doc, e, memb->type());
					addDocumentation(doc, e, memb->documentation());
					m.PushBack(e, doc.GetAllocator());
				}

				JSONTools::addValue(doc, v, "members", m);
				return v;
			}
			else if (dynamic_cast<Language::Array*>(t.get()))
			{
				rapidjson::Value v(rapidjson::kObjectType);
				addName(doc, v, t->name());
				addType(doc, v, dynamic_cast<Language::Array*>(t.get())->types().front());
				return v;
			}
			else if (dynamic_cast<Language::Nullable*>(t.get()))
			{
				rapidjson::Value v(rapidjson::kObjectType);
				addName(doc, v, t->name());
				addType(doc, v, dynamic_cast<Language::Nullable*>(t.get())->types().front());
				return v;
			}
			else if (dynamic_cast<Language::Tuple*>(t.get()))
			{
				rapidjson::Value v(rapidjson::kObjectType);
				addName(doc, v, t->name());
				addType(doc, v, dynamic_cast<Language::Tuple*>(t.get())->types());
				return v;
			}

			rapidjson::Value v(rapidjson::kStringType);
			v.SetString(t->name(), doc.GetAllocator());
			return v;
		}

		static void addTypeDefinition(rapidjson::Document & doc, rapidjson::Value & v, const Language::TypeDefinition::Ptr & t)
		{
			addNature(doc, v, "typedef");
			addName(doc, v, t->name());
			addType(doc, v, t->type());
			addDocumentation(doc, v, t->documentation());
		}

		static void addFunction(rapidjson::Document & doc, rapidjson::Value & v, const Language::Function::Variant::Ptr & f)
		{
			addNature(doc, v, "function");
			addName(doc, v, f->name());
			addType(doc, v, f->returnType());
			rapidjson::Value a(rapidjson::kArrayType);
			for (auto & arg : f->arguments())
			{
				rapidjson::Value e(rapidjson::kObjectType);
				switch (arg->direction())
				{
				case Language::Function::Variant::Argument::Direction::In:
					JSONTools::addValue(doc, e, "direction", "in"); break;
				case Language::Function::Variant::Argument::Direction::Out:
					JSONTools::addValue(doc, e, "direction", "out"); break;
				case Language::Function::Variant::Argument::Direction::InOut:
					JSONTools::addValue(doc, e, "direction", "in-out"); break;
				}
				addName(doc, e, arg->name());
				addType(doc, e, arg->type());
				addDocumentation(doc, e, arg->documentation());
				a.PushBack(e, doc.GetAllocator());
			}
			JSONTools::addValue(doc, v, "arguments", a);
			addDocumentation(doc, v, f->documentation());
		}

		static void addMethod(rapidjson::Document & doc, rapidjson::Value & v, const Language::Method::Variant::Ptr & f)
		{
			addNature(doc, v, "method");
			addName(doc, v, f->name());
			addType(doc, v, f->returnType());
			rapidjson::Value a(rapidjson::kArrayType);
			for (auto & arg : f->arguments())
			{
				rapidjson::Value e(rapidjson::kObjectType);
				switch (arg->direction())
				{
				case Language::Method::Variant::Argument::Direction::In:
					JSONTools::addValue(doc, e, "direction", "in"); break;
				case Language::Method::Variant::Argument::Direction::Out:
					JSONTools::addValue(doc, e, "direction", "out"); break;
				case Language::Method::Variant::Argument::Direction::InOut:
					JSONTools::addValue(doc, e, "direction", "in-out"); break;
				}
				addName(doc, e, arg->name());
				addType(doc, e, arg->type());
				addDocumentation(doc, e, arg->documentation());
				a.PushBack(e, doc.GetAllocator());
			}
			JSONTools::addValue(doc, v, "arguments", a);
			addDocumentation(doc, v, f->documentation());
		}

		static void addProperty(rapidjson::Document & doc, rapidjson::Value & v, const std::shared_ptr<Language::Property> & p)
		{
			addNature(doc, v, "property");
			addName(doc, v, p->name());
			addType(doc, v, p->type());
			JSONTools::addValue(doc, v, "readonly", p->readOnly());
			addDocumentation(doc, v, p->documentation());
		}

		static void addObject(rapidjson::Document & doc, rapidjson::Value & v, const std::shared_ptr<Language::Object> & o)
		{
			addNature(doc, v, "object");
			addName(doc, v, o->name());

			rapidjson::Value b(rapidjson::kArrayType);

			for (auto & d : o->definitions())
			{
                if (dynamic_cast<Language::TypeDefinition*>(d.get()))
                {
                    rapidjson::Value e(rapidjson::kObjectType);
                    addTypeDefinition(doc, e, std::dynamic_pointer_cast<Language::TypeDefinition>(d));
                    b.PushBack(e, doc.GetAllocator());
                }
                else if (dynamic_cast<Language::Object*>(d.get()))
                {
                    rapidjson::Value e(rapidjson::kObjectType);
                    addObject(doc, e, std::dynamic_pointer_cast<Language::Object>(d));
                    b.PushBack(e, doc.GetAllocator());
                }
                else if (dynamic_cast<Language::MethodVariant*>(d.get()))
				{
					rapidjson::Value e(rapidjson::kObjectType);
					addMethod(doc, e, std::dynamic_pointer_cast<Language::MethodVariant>(d));
					b.PushBack(e, doc.GetAllocator());
				}
				else if (dynamic_cast<Language::Property*>(d.get()))
				{
					rapidjson::Value e(rapidjson::kObjectType);
					addProperty(doc, e, std::dynamic_pointer_cast<Language::Property>(d));
					b.PushBack(e, doc.GetAllocator());
				}
			}
			JSONTools::addValue(doc, v, "body", b);
			addDocumentation(doc, v, o->documentation());
		}

		static void addInterface(rapidjson::Document & doc, rapidjson::Value & v, const std::shared_ptr<Language::Interface> & intf)
		{
			addNature(doc, v, "interface");
			addName(doc, v, intf->name());

			rapidjson::Value b(rapidjson::kArrayType);

			for (auto & d : intf->definitions())
			{
				if (dynamic_cast<Language::TypeDefinition*>(d.get()))
				{
					rapidjson::Value e(rapidjson::kObjectType);
					addTypeDefinition(doc, e, std::dynamic_pointer_cast<Language::TypeDefinition>(d));
					b.PushBack(e, doc.GetAllocator());
				}
				else if (dynamic_cast<Language::FunctionVariant*>(d.get()))
				{
					rapidjson::Value e(rapidjson::kObjectType);
					addFunction(doc, e, std::dynamic_pointer_cast<Language::FunctionVariant>(d));
					b.PushBack(e, doc.GetAllocator());
				}
				else if (dynamic_cast<Language::Object*>(d.get()))
				{
					rapidjson::Value e(rapidjson::kObjectType);
					addObject(doc, e, std::dynamic_pointer_cast<Language::Object>(d));
					b.PushBack(e, doc.GetAllocator());
				}
			}

			JSONTools::addValue(doc, v, "body", b);
			addDocumentation(doc, v, intf->documentation());
		}

		static void addModule(rapidjson::Document & doc, rapidjson::Value & v, const std::shared_ptr<Language::Module> & mod)
		{
			addNature(doc, v, "module");
			addName(doc, v, mod->name());

			if (mod->info().size())
			{
				rapidjson::Value i(rapidjson::kArrayType);
				for (auto & e : mod->info())
				{
					rapidjson::Value t(rapidjson::kObjectType);
					JSONTools::addValue(doc, t, "name", e.first);
					if (e.first.length() && e.first.front() != '_')
						JSONTools::addValue(doc, t, "value", e.second);
					i.PushBack(t, doc.GetAllocator());
				}
				JSONTools::addValue(doc, v, "info", i);
				addDocumentation(doc, v, mod->documentation());
			}

			rapidjson::Value b(rapidjson::kArrayType);

			for (auto & e : mod->elements())
			{
				rapidjson::Value t(rapidjson::kObjectType);
				addTopLevel(doc, t, e);
				b.PushBack(t, doc.GetAllocator());
			}
			JSONTools::addValue(doc, v, "body", b);
			addDocumentation(doc, v, mod->documentation());
		}

		static void addTopLevel(rapidjson::Document & doc, rapidjson::Value & v, const std::shared_ptr<Language::TopLevel> & tl)
		{
			rapidjson::Value b(rapidjson::kArrayType);
			if (dynamic_cast<Language::Interface*>(tl.get()))
				addInterface(doc, v, std::dynamic_pointer_cast<Language::Interface>(tl));
			else if (dynamic_cast<Language::Module*>(tl.get()))
				addModule(doc, v, std::dynamic_pointer_cast<Language::Module>(tl));
		}

		bool write(const std::vector<std::shared_ptr<Language::TopLevel>> & topLevels, ErrorCollector & ec)
		{
            (void)ec;

			for (auto & tl : topLevels)
			{
				rapidjson::Document doc;
				doc.SetObject();

				addTopLevel(doc, doc, tl);

				rapidjson::StringBuffer sb;
				if (hr)
				{
					rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
					doc.Accept(w);
				}
				else
				{
					rapidjson::Writer<rapidjson::StringBuffer> w(sb);
					doc.Accept(w);
				}

				*o << sb.GetString() << std::endl;
			}

			return true;
		}

		std::shared_ptr<std::ostream> o;
		bool hr;
	};

	JSONWriter::JSONWriter(const std::shared_ptr<std::ostream> & s, bool hr) : priv(new Priv(s, hr))
	{ }

	JSONWriter::~JSONWriter() = default;

	bool JSONWriter::write(Reader * reader, ErrorCollector & ec)
	{
		return priv->write(reader->topLevels(), ec);
	}

}
