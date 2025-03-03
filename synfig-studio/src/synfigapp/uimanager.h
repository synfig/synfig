/* === S Y N F I G ========================================================= */
/*!	\file uimanager.h
**	\brief User Interface Manager Class
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
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

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_APP_UIMANAGER_H
#define __SYNFIG_APP_UIMANAGER_H

/* === H E A D E R S ======================================================= */

#include <ETL/handle>
#include <sigc++/sigc++.h>

#include <synfig/string.h>
#include <synfig/progresscallback.h>

/* === M A C R O S ========================================================= */

/* === T Y P E D E F S ===================================================== */

/* === C L A S S E S & S T R U C T S ======================================= */

namespace synfigapp {

class UIInterface : public etl::shared_object, public synfig::ProgressCallback, public sigc::trackable
{
public:
	enum Response
	{
		RESPONSE_NONE = -9, /*<< Shouldn't be used as response */
		RESPONSE_CANCEL = -1,
		RESPONSE_NO = 0,
		RESPONSE_YES = 1,
		RESPONSE_OK = 2
	};
	virtual ~UIInterface() { }
	virtual Response confirmation(
				const std::string &message,
				const std::string &details,
				const std::string &confirm,
				const std::string &cancel,
				Response dflt = RESPONSE_OK
	) = 0;


	virtual Response yes_no_cancel(
				const std::string &message,
				const std::string &details,
				const std::string &button1,
				const std::string &button2,
				const std::string &button3,
				Response destructive_response = RESPONSE_NONE, /**< If UI should highlight the correspondent action is dangerous (data loss) */
				Response dflt = RESPONSE_YES
	) = 0;
};

class DefaultUIInterface : public UIInterface
{
public:
	Response confirmation(
			const std::string &/*message*/,
			const std::string &/*details*/,
			const std::string &/*confirm*/,
			const std::string &/*cancel*/,
			Response dflt
	) override
	{ return dflt; }


	Response yes_no_cancel(
			const std::string &/*message*/,
			const std::string &/*details*/,
			const std::string &/*button1*/,
			const std::string &/*button2*/,
			const std::string &/*button3*/,
			Response /*destructive_response*/,
			Response dflt
	) override
	{ return dflt; }


	bool task(const std::string &/*task*/) override
		{ return true; }
	bool error(const std::string &/*task*/) override
		{ return true; }
	bool warning(const std::string &/*task*/) override
		{ return true; }
	bool amount_complete(int /*current*/, int /*total*/) override
		{ return true; }
};

class ConfidentUIInterface : public UIInterface
{
public:
	Response confirmation(
			const std::string &/*message*/,
			const std::string &/*details*/,
			const std::string &/*confirm*/,
			const std::string &/*cancel*/,
			Response /*dflt*/
	) override
	{ return RESPONSE_OK; }


	Response yes_no_cancel(
			const std::string &/*message*/,
			const std::string &/*details*/,
			const std::string &/*button1*/,
			const std::string &/*button2*/,
			const std::string &/*button3*/,
			Response /*destructive_response*/,
			Response /*dflt*/
	) override
	{ return RESPONSE_YES; }


	bool task(const std::string &/*task*/) override
		{ return true; }
	bool error(const std::string &/*task*/) override
		{ return true; }
	bool warning(const std::string &/*task*/) override
		{ return true; }
	bool amount_complete(int /*current*/, int /*total*/) override
		{ return true; }
};

class ConsoleUIInterface : public UIInterface
{
public:
	Response confirmation(
			const std::string &message,
			const std::string &details,
			const std::string &confirm,
			const std::string &cancel,
			Response dflt
	) override;


	Response yes_no_cancel(
			const std::string &message,
			const std::string &details,
			const std::string &button1,
			const std::string &button2,
			const std::string &button3,
			Response destructive_response,
			Response dflt
	) override;


	bool task(const std::string &task) override;
	bool error(const std::string &task) override;
	bool warning(const std::string &task) override;
	bool amount_complete(int current, int total) override;
};

}; // END of namespace synfigapp

/* === E N D =============================================================== */

#endif
