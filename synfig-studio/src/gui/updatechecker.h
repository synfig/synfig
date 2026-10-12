/* === S Y N F I G ========================================================= */
/*! \file gui/updatechecker.h
**  \brief Online update checker interface
**
**  \legal
**	Copyright (c) 2026 036006
**
**  This file is part of Synfig.
**
**  Synfig is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  Synfig is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**  \endlegal
**
** ========================================================================= */

/* === S T A R T =========================================================== */

#ifndef __SYNFIG_STUDIO_UPDATECHECKER_H
#define __SYNFIG_STUDIO_UPDATECHECKER_H

/* === H E A D E R S ======================================================= */

#include <string>

/* === F U N C T I O N S =================================================== */

namespace studio { namespace update_checker {

enum UpdateCheckConsent {
	UPDATE_CHECK_CONSENT_UNKNOWN  = -1,
	UPDATE_CHECK_CONSENT_DENIED   =  0,
	UPDATE_CHECK_CONSENT_ALLOWED  =  1,
};

extern const std::string check_url_dev;
extern const std::string landing_url_dev;
extern const std::string check_url_stable;
extern const std::string landing_url_stable;

// Starts asynchronous online update check (no-op if already started)
void start_async();

} } // namespace studio::update_checker

/* === E N D =============================================================== */

#endif
