/* === S Y N F I G ========================================================= */
/*!	\file token.cpp
**	\brief Token File
**
**	$Id$
**
**	\legal
**	......... ... 2018 Ivan Mahonin
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <synfig/localization.h>
#include <synfig/general.h>

#include "token.h"

#endif

/* === U S I N G =========================================================== */

using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === M E T H O D S ======================================================= */

bool Token::ready_ = false;
bool Token::root_exists_ = false;
Token *Token::first_ = 0;
Token *Token::last_ = 0;

Token Token::token;

Token::Token(const Token&):
	previous_(), next_(), in_process_(), prepared_() { }

Token::Token():
	previous_(), next_(), in_process_(), prepared_()
{ root_exists_ = true; init(); }

Token::Token(const Handle &parent):
	previous_(),
	next_(),
	in_process_(),
	prepared_()
{
	parents_.insert(parent);
	init();
}

Token::Token(const Set &parents):
	previous_(),
	next_(),
	in_process_(),
	prepared_(),
	parents_(parents)
{ init(); }

void
Token::init()
{
	ready_ = false;
	previous_ = last_;
	(previous_ ? previous_->next_ : first_) = this;
	last_ = this;
}

Token::~Token()
{
	ready_ = false;
	(previous_ ? previous_->next_ : first_) = next_;
	(next_     ? next_->previous_ : last_ ) = previous_;

	// remove all references
	for(Token* token = first_; token; token = token->next_)
		token->parents_.erase(handle());
}

void
Token::fill_all_parents()
{
	if (parents_.empty() || !all_parents_.empty())
		return;

	if (in_process_)
	{
		error(String(_("Loop in hierarchy of tokens detected")));
		assert(false);
		return;
	}

	in_process_ = true;
	for(Set::iterator i = parents_.begin(); i != parents_.end(); ++i)
	{
		Token &t = (*i)->cast_const();
		all_parents_.insert( t.handle() );
		t.fill_all_parents();
		all_parents_.insert( t.all_parents_.begin(), t.all_parents_.end() );
	}
	in_process_ = false;
}

void
Token::prepare()
{
	if (prepared_) return;
	if (in_process_)
	{
		error(String(_("Loop detected while preparing tokens")));
		assert(false);
		return;
	}

	prepare_vfunc();

	in_process_ = true;
	prepared_ = true;
	in_process_ = false;
}

void
Token::rebuild()
{
	if (ready_) return;
	assert(root_exists_);

	// clear
	for(Token* t = first_; t; t = t->next_)
	{
		t->unprepare_vfunc();
		t->prepared_ = false;
		t->children_.clear();
		t->all_parents_.clear();
		t->all_children_.clear();
	}

	// add root
	if (root_exists_)
		for(Token* t = first_; t; t = t->next_)
			if ( t->parents_.empty()
			  && t->handle() != token.handle() )
				t->parents_.insert( Token::token.handle() );

	// fill parents
	for(Token* t = first_; t; t = t->next_)
		t->fill_all_parents();

	// fill children
	for(Token* t = first_; t; t = t->next_)
	{
		for(Set::iterator i = t->parents_.begin(); i != t->parents_.end(); ++i)
			(*i)->cast_const().children_.insert( t->handle() );
		for(Set::iterator i = t->all_parents_.begin(); i != t->all_parents_.end(); ++i)
			(*i)->cast_const().all_children_.insert( t->handle() );
	}

	// prepare
	for(Token* t = first_; t; t = t->next_)
		t->prepare();

	ready_ = true;
}
