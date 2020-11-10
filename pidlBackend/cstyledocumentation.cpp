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

#include "include/pidlBackend/cstyledocumentation.h"
#include "include/pidlBackend/language.h"
#include "include/pidlBackend/codegencontext.h"

#include <assert.h>

#include <sstream>
#include <algorithm>

namespace PIDL
{
    namespace  {
        std::list<std::string> split(const std::string & src, char delimeter)
        {
            std::stringstream ss(src);
            std::string item;
            std::list<std::string> splittedStrings;
            while (std::getline(ss, item, delimeter))
                splittedStrings.push_back(item);
            return splittedStrings;
        };

        std::string join(const std::list<std::string> & src, char delimiter)

        {
            std::stringstream ss;
            bool is_first = true;
            for (auto & s : src)
            {
                if (is_first)
                    is_first = false;
                else if (delimeter)
                    ss << delimiter;

                ss << s;
            }
            return ss.str();
        };

        std::string replace(const std::string & str, char before, char after)
        {
            return join(split(str, before), after);
        }

        std::string trim(const std::string & str, char sp)
        {
            auto first = str.find_first_not_of(sp);
            auto last = str.find_last_not_of(sp);
            if(first == std::string::npos)
                first = 0;
            if(last == std::string::npos)
                last = str.length() - 1;
            return str.substr(first, (last-first+1));
        }

        std::string clear(const std::string & str)
        {
            return trim(trim(replace(str, '\r', '\0'), '\t'), ' ');
        }
    }

	//struct CStyleDocumentation::Priv { };
	CStyleDocumentation::CStyleDocumentation() : priv(nullptr) { }
	CStyleDocumentation::~CStyleDocumentation() = default;


	//struct CStyleVoidDocumentation::Priv { };
	CStyleVoidDocumentation::CStyleVoidDocumentation() : priv(nullptr) { }
	CStyleVoidDocumentation::~CStyleVoidDocumentation() = default;

	bool CStyleVoidDocumentation::write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
        (void)code_deepness;
        (void)ctx;
        (void)place;
        (void)docprov;
        (void)ec;
		return true;
	}


	//struct CStyleBasicDocumentation::Priv { };
	CStyleBasicDocumentation::CStyleBasicDocumentation() : priv(nullptr) { }
	CStyleBasicDocumentation::~CStyleBasicDocumentation() = default;

	bool CStyleBasicDocumentation::write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
        (void)ec;
		auto & doc = docprov->documentation();

		if (!doc.brief.length() && !doc.details.size())
			return true;

		auto writeLines = [&](const std::string & title, const std::list<std::string> & lines)
		{
			if (title.length())
				ctx->writeTabs(code_deepness) << " * " << title << std::endl;
			for (auto & l : lines)
                ctx->writeTabs(code_deepness) << " * " << clear(l) << std::endl;
		};

		switch (place)
		{
		case Place::Before:
			if (!dynamic_cast<Language::FunctionVariant::Argument*>(docprov))
			{
				**ctx << std::endl;
				ctx->writeTabs(code_deepness) << "/*" << std::endl;

				if (doc.brief.size())
					writeLines(std::string(), split(doc.brief, '\n'));

				if (doc.details.count(Language::DocumentationProvider::Documentation::Description))
                    writeLines("\n", split(doc.details.at(Language::DocumentationProvider::Documentation::Description), '\n'));

				if (dynamic_cast<Language::Function*>(docprov) &&
					doc.details.count(Language::DocumentationProvider::Documentation::Return))
                    writeLines("\nReturn:", split(doc.details.at(Language::DocumentationProvider::Documentation::Return), '\n'));

                // Language::DocumentationProvider::Documentation::Group is not handled here

				ctx->writeTabs(code_deepness) << " */" << std::endl;
			}
			break;
		case Place::After:
			if (dynamic_cast<Language::FunctionVariant::Argument*>(docprov))
			{
                *ctx << " // " << clear(replace(doc.brief, '\n', ' ')) << std::endl;
				ctx->writeTabs(code_deepness + 1);
			}
			break;
		}

		return true;
	}

	//struct CStyleDotNetDocumentation::Priv { };
	CStyleDotNetDocumentation::CStyleDotNetDocumentation() : priv(nullptr) { }
	CStyleDotNetDocumentation::~CStyleDotNetDocumentation() = default;

	bool CStyleDotNetDocumentation::write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
        (void)ec;
		auto & doc = docprov->documentation();

		if (!doc.brief.length() && !doc.details.size())
			return true;

		auto writeLines = [&](const std::list<std::string> & lines)
		{
			for (auto & l : lines)
                ctx->writeTabs(code_deepness) << "/// " << clear(l) << std::endl;
		};

		switch (place)
		{
		case Place::Before:
			if (!dynamic_cast<Language::FunctionVariant::Argument*>(docprov))
			{
				**ctx << std::endl;

				if (doc.brief.length())
				{
					ctx->writeTabs(code_deepness) << "/// <summary>" << std::endl;
					writeLines(split(doc.brief, '\n'));
					ctx->writeTabs(code_deepness) << "/// </summary>" << std::endl;
				}

				if (doc.details.count(Language::DocumentationProvider::Documentation::Description))
				{
					ctx->writeTabs(code_deepness) << "/// <remarks>" << std::endl;
					writeLines(split(doc.details.at(Language::DocumentationProvider::Documentation::Description), '\n'));
					ctx->writeTabs(code_deepness) << "/// </remarks>" << std::endl;
				}

				if (dynamic_cast<Language::FunctionVariant*>(docprov))
				{
					if (doc.details.count(Language::DocumentationProvider::Documentation::Return))
					{
						ctx->writeTabs(code_deepness) << "/// <returns>" << std::endl;
						writeLines(split(doc.details.at(Language::DocumentationProvider::Documentation::Return), '\n'));
						ctx->writeTabs(code_deepness) << "/// </returns>" << std::endl;
					}

					for (auto & arg : dynamic_cast<Language::FunctionVariant*>(docprov)->arguments())
					{
                        if (static_cast<long>(arg->documentation().brief.find("\n")) == -1)
						{
							ctx->writeTabs(code_deepness) << "/// <param name=\"" << arg->name() << "\">";
							*ctx << arg->documentation().brief;
							*ctx << "</param>" << std::endl;
						}
						else
						{
							ctx->writeTabs(code_deepness) << "/// <param name=\"" << arg->name() << "\">" << std::endl;
							writeLines(split(arg->documentation().brief, '\n'));
							ctx->writeTabs(code_deepness) << "/// </param>" << std::endl;
						}
					}
				}

                // Language::DocumentationProvider::Documentation::Group is not handled here

			}
			break;
		case Place::After:
			break;
		}

		return true;
	}

	//struct CStyleDoxygenDocumentation::Priv { };
	CStyleDoxygenDocumentation::CStyleDoxygenDocumentation() : priv(nullptr) { }
	CStyleDoxygenDocumentation::~CStyleDoxygenDocumentation() = default;

	bool CStyleDoxygenDocumentation::write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
        (void)ec;
		auto & doc = docprov->documentation();

		if (!doc.brief.length() && !doc.details.size())
			return true;

		auto writeLine = [&](const std::string & line)
		{
            ctx->writeTabs(code_deepness) << " * " << clear(line) << std::endl;
		};

		auto writeLines = [&](const std::list<std::string> & lines)
		{
			for (auto & l : lines)
				writeLine(l);
		};

		switch (place)
		{
		case Place::Before:
			if (!dynamic_cast<Language::FunctionVariant::Argument*>(docprov))
			{
				**ctx << std::endl;
                ctx->writeTabs(code_deepness) << "/*!";

                if (doc.details.count(Language::DocumentationProvider::Documentation::Group))
                    **ctx << " @ingroup " << doc.details.at(Language::DocumentationProvider::Documentation::Group);

                **ctx << std::endl;

				if (doc.brief.length())
                    ctx->writeTabs(code_deepness) << " * @brief " << clear(replace(doc.brief, '\n', ' ')) << std::endl;

				if (doc.details.count(Language::DocumentationProvider::Documentation::Description))
					writeLines(split(doc.details.at(Language::DocumentationProvider::Documentation::Description), '\n'));

                if (dynamic_cast<Language::FunctionVariant*>(docprov))
				{
					if (doc.details.count(Language::DocumentationProvider::Documentation::Return))
					{
                        ctx->writeTabs(code_deepness) << " * @return " << clear(replace(doc.details.at(Language::DocumentationProvider::Documentation::Return), '\n', ' ')) << std::endl;
					}

					for (auto & arg : dynamic_cast<Language::FunctionVariant*>(docprov)->arguments())
					{
						auto & doc = arg->documentation();
						ctx->writeTabs(code_deepness) << " * @param[";
						switch (arg->direction())
						{
						case Language::FunctionVariant::Argument::Direction::In:
							*ctx << "in"; break;
						case Language::FunctionVariant::Argument::Direction::Out:
							*ctx << "out"; break;
						case Language::FunctionVariant::Argument::Direction::InOut:
							*ctx << "in,out"; break;
						}
                        *ctx << "] " << arg->name() << " " << clear(replace(doc.brief, '\n', ' ')) << std::endl;
					}
				}
				ctx->writeTabs(code_deepness) << " */" << std::endl;
			}
			break;
		case Place::After:
			break;
		}

		return true;
	}
}
