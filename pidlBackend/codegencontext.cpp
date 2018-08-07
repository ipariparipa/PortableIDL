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

#include "include/pidlBackend/codegencontext.h"

namespace PIDL
{

	struct CodeGenContext::Priv
	{
		Priv(short tab_length_, char tab_char_, std::ostream & o_, Role role_) :
			o(o_),
			role(role_),
			tab_length(tab_length_),
			tab_char(tab_char_)
		{ }

		std::ostream & o;
		Role role;

		short tab_length;
		char tab_char;
	};

	CodeGenContext::CodeGenContext(short tab_length, char tab_char, std::ostream & o, Role role) :
		priv(new Priv(tab_length, tab_char, o, role))
	{ }

	CodeGenContext::~CodeGenContext()
	{
		delete priv;
	}

	std::ostream & CodeGenContext::operator * () const
	{
		return priv->o;
	}

	std::ostream & CodeGenContext::stream() const
	{
		return priv->o;
	}

	CodeGenContext::Role CodeGenContext::role() const
	{
		return priv->role;
	}

	std::ostream & CodeGenContext::writeTabs(short code_deepness)
	{
		for (short i = 0, l = code_deepness * priv->tab_length; i < l; ++i)
			priv->o << priv->tab_char;
		return priv->o;
	}

}
