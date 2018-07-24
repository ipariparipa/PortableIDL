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
		Priv(const std::shared_ptr<std::ostream> & s) : o(s) { }

		static void addName(rapidjson::Document & doc, rapidjson::Value & r, const char * str)
		{
			JSONTools::addValue(doc, r, "name", str);
		}

		static void addNature(rapidjson::Document & doc, rapidjson::Value & r, const char * str)
		{
			JSONTools::addValue(doc, r, "nature", str);
		}

		static void addType(rapidjson::Document & doc, rapidjson::Value & r, const std::shared_ptr<Language::Type> & t)
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
					m.PushBack(e, doc.GetAllocator());
				}

				JSONTools::addValue(doc, v, "members", m);
				JSONTools::addValue(doc, r, "type", v);
			}
			else if (dynamic_cast<Language::Array*>(t.get()))
			{
				rapidjson::Value v(rapidjson::kObjectType);
				addName(doc, v, t->name());
				addType(doc, v, dynamic_cast<Language::Array*>(t.get())->type());
				JSONTools::addValue(doc, r, "type", v);
			}
			else if (dynamic_cast<Language::Nullable*>(t.get()))
			{
				rapidjson::Value v(rapidjson::kObjectType);
				addName(doc, v, t->name());
				addType(doc, v, dynamic_cast<Language::Nullable*>(t.get())->type());
				JSONTools::addValue(doc, r, "type", v);
			}
			else
				JSONTools::addValue(doc, r, "type", t->name());
		}

		static void addTypeDefinition(rapidjson::Document & doc, rapidjson::Value & v, const std::shared_ptr<Language::TypeDefinition> & t)
		{
			addNature(doc, v, "typedef");
			addName(doc, v, t->name());
			addType(doc, v, t->type());
		}

		static void addFunction(rapidjson::Document & doc, rapidjson::Value & v, const std::shared_ptr<Language::Function> & f)
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
				case Language::Function::Argument::Direction::In:
					JSONTools::addValue(doc, e, "direction", "in"); break;
				case Language::Function::Argument::Direction::Out:
					JSONTools::addValue(doc, e, "direction", "out"); break;
				case Language::Function::Argument::Direction::InOut:
					JSONTools::addValue(doc, e, "direction", "in-out"); break;
				}
				addName(doc, e, arg->name());
				addType(doc, e, arg->type());
				a.PushBack(e, doc.GetAllocator());
			}
			JSONTools::addValue(doc, v, "arguments", a);
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
				else if (dynamic_cast<Language::Function*>(d.get()))
				{
					rapidjson::Value e(rapidjson::kObjectType);
					addFunction(doc, e, std::dynamic_pointer_cast<Language::Function>(d));
					b.PushBack(e, doc.GetAllocator());
				}
			}

			JSONTools::addValue(doc, v, "body", b);
		}

		static void addModule(rapidjson::Document & doc, rapidjson::Value & v, const std::shared_ptr<Language::Module> & mod)
		{
			addNature(doc, v, "module");
			addName(doc, v, mod->name());

			rapidjson::Value b(rapidjson::kArrayType);

			for (auto & e : mod->elements())
			{
				rapidjson::Value t(rapidjson::kObjectType);
				addTopLevel(doc, t, e);
				b.PushBack(t, doc.GetAllocator());
			}
			JSONTools::addValue(doc, v, "body", b);
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
			for (auto & tl : topLevels)
			{
				rapidjson::Document doc;
				doc.SetObject();

				addTopLevel(doc, doc, tl);

				rapidjson::StringBuffer sb;
				rapidjson::PrettyWriter<rapidjson::StringBuffer> w(sb);
				doc.Accept(w);

				*o << sb.GetString() << std::endl;
			}

			return true;
		}

		std::shared_ptr<std::ostream> o;
	};

	JSONWriter::JSONWriter(const std::shared_ptr<std::ostream> & s) : priv(new Priv(s))
	{ }

	JSONWriter::~JSONWriter() = default;

	bool JSONWriter::write(Reader * reader, ErrorCollector & ec)
	{
		return priv->write(reader->topLevels(), ec);
	}

}