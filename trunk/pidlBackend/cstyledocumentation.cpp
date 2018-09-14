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

	//struct CStyleDocumentation::Priv { };
	CStyleDocumentation::CStyleDocumentation() : priv(nullptr) { }
	CStyleDocumentation::~CStyleDocumentation() = default;


	//struct CStyleVoidDocumentation::Priv { };
	CStyleVoidDocumentation::CStyleVoidDocumentation() : priv(nullptr) { }
	CStyleVoidDocumentation::~CStyleVoidDocumentation() = default;

	bool CStyleVoidDocumentation::write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
		return true;
	}


	//struct CStyleBasicDocumentation::Priv { };
	CStyleBasicDocumentation::CStyleBasicDocumentation() : priv(nullptr) { }
	CStyleBasicDocumentation::~CStyleBasicDocumentation() = default;

	bool CStyleBasicDocumentation::write(short code_deepness, CodeGenContext * ctx, Place place, Language::DocumentationProvider * docprov, ErrorCollector & ec)
	{
		auto & doc = docprov->documentation();

		if (!doc.brief.length() && !doc.details.size())
			return true;

		auto split = [](const std::string & src, char delimeter) -> std::list<std::string>
		{
			std::stringstream ss(src);
			std::string item;
			std::list<std::string> splittedStrings;
			while (std::getline(ss, item, delimeter))
				splittedStrings.push_back(item);
			return splittedStrings;
		};

		auto join = [](const std::list<std::string> & src, char delimeter) -> std::string
		{
			std::stringstream ss;
			bool is_first = true;
			for (auto & s : src)
			{
				if (is_first)
					is_first = false;
				else if (delimeter)
					ss << delimeter;
				ss << s;
			}
			return ss.str();
		};


		auto clean = [&](const std::string & str) -> std::string
		{
			return join(split(str, '\r'), '\0');
		};

		auto writeLines = [&](const std::string & title, const std::list<std::string> & lines)
		{
			if (title.length())
				ctx->writeTabs(code_deepness) << " * " << title << std::endl;
			for (auto & l : lines)
				ctx->writeTabs(code_deepness) << " * " << clean(l) << std::endl;
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
					writeLines("Description:", split(doc.details.at(Language::DocumentationProvider::Documentation::Description), '\n'));

				if (dynamic_cast<Language::Function*>(docprov) &&
					doc.details.count(Language::DocumentationProvider::Documentation::Return))
					writeLines("Return:", split(doc.details.at(Language::DocumentationProvider::Documentation::Return), '\n'));

				ctx->writeTabs(code_deepness) << " */" << std::endl;
			}
			break;
		case Place::After:
			if (dynamic_cast<Language::FunctionVariant::Argument*>(docprov))
			{
				*ctx << " // " << clean(join(split(doc.brief, '\n'), ' ')) << std::endl;
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
		auto & doc = docprov->documentation();

		if (!doc.brief.length() && !doc.details.size())
			return true;

		auto split = [](const std::string & src, char delimeter) -> std::list<std::string>
		{
			std::stringstream ss(src);
			std::string item;
			std::list<std::string> splittedStrings;
			while (std::getline(ss, item, delimeter))
				splittedStrings.push_back(item);
			return splittedStrings;
		};

		auto join = [](const std::list<std::string> & src, char delimeter) -> std::string
		{
			std::stringstream ss;
			bool is_first = true;
			for (auto & s : src)
			{
				if (is_first)
					is_first = false;
				else if (delimeter)
					ss << delimeter;
				ss << s;
			}
			return ss.str();
		};

		auto writeLines = [&](const std::list<std::string> & lines)
		{
			for (auto & l : lines)
				ctx->writeTabs(code_deepness) << "/// " << join(split(l, '\r'), '\0') << std::endl;
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
						if ((long)arg->documentation().brief.find("\n") == -1)
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
		auto & doc = docprov->documentation();

		if (!doc.brief.length() && !doc.details.size())
			return true;

		auto split = [](const std::string & src, char delimeter) -> std::list<std::string>
		{
			std::stringstream ss(src);
			std::string item;
			std::list<std::string> splittedStrings;
			while (std::getline(ss, item, delimeter))
				splittedStrings.push_back(item);
			return splittedStrings;
		};

		auto join = [](const std::list<std::string> & src, char delimeter) -> std::string
		{
			std::stringstream ss;
			bool is_first = true;
			for (auto & s : src)
			{
				if (is_first)
					is_first = false;
				else if (delimeter)
					ss << delimeter;
				ss << s;
			}
			return ss.str();
		};

		auto clean = [&](const std::string & str) -> std::string
		{
			return join(split(str, '\r'), '\0');
		};

		auto writeLine = [&](const std::string & line)
		{
			ctx->writeTabs(code_deepness) << " * " << clean(line) << std::endl;
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
				ctx->writeTabs(code_deepness) << "/*!" << std::endl;

				if (doc.brief.length())
					ctx->writeTabs(code_deepness) << " * @brief " << clean(join(split(doc.brief, '\n'), ' ')) << std::endl;

				if (doc.details.count(Language::DocumentationProvider::Documentation::Description))
					writeLines(split(doc.details.at(Language::DocumentationProvider::Documentation::Description), '\n'));

				if (dynamic_cast<Language::Function*>(docprov))
				{
					if (doc.details.count(Language::DocumentationProvider::Documentation::Return))
					{
						ctx->writeTabs(code_deepness) << " * @return " << clean(join(split(doc.details.at(Language::DocumentationProvider::Documentation::Return), '\n'), ' ')) << std::endl;
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
						*ctx << "] " << arg->name() << " " << clean(join(split(doc.brief, '\n'), ' ')) << std::endl;
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
