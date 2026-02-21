/* === S Y N F I G ========================================================= */
/*! \file gui/updatechecker.cpp
**  \brief Online update checker
**
**  \legal
**	Copyright (c) 2026 036006
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
#  include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gui/updatechecker.h"

#include <autorevision.h>
#include <gui/app.h>
#include <gui/localization.h>
#include <gui/mainwindow.h>

#include <synfig/general.h>

#include <giomm/file.h>
#include <glibmm/main.h>
#include <gtkmm/messagedialog.h>

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <cctype>
#include <string>
#include <thread>
#include <vector>

#endif

/* === U S I N G =========================================================== */

using namespace studio;
using namespace synfig;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

namespace {

#if defined(DEVELOPMENT_SNAPSHOT) && DEVELOPMENT_SNAPSHOT
static constexpr bool is_development_snapshot = true;
#else
static constexpr bool is_development_snapshot = false;
#endif

std::atomic<bool> update_checker_started(false);

} // anonymous namespace

namespace studio { namespace update_checker {

const std::string check_url_dev = "https://synfig.org/download/latest-dev.txt";
const std::string landing_url_dev = "https://www.synfig.org/download-development/";
const std::string check_url_stable = "https://synfig.org/download/latest-stable.txt";
const std::string landing_url_stable = "https://www.synfig.org/download-stable/";

} } // namespace studio::update_checker

/* === P R O C E D U R E S ================================================= */

namespace {

std::vector<int> version_parts(const std::string& value)
{
	std::vector<int> result;
	int current = -1;
	for (char ch : value) {
		if (std::isdigit(static_cast<unsigned char>(ch))) {
			if (current < 0) current = 0;
			current = current * 10 + (ch - '0');
		} else if (current >= 0) {
			result.push_back(current);
			current = -1;
		}
	}
	if (current >= 0)
		result.push_back(current);
	return result;
}

int compare_versions(const std::string& lhs, const std::string& rhs)
{
	const std::vector<int> lparts = version_parts(lhs);
	const std::vector<int> rparts = version_parts(rhs);
	const size_t count = std::max(lparts.size(), rparts.size());
	for (size_t i = 0; i < count; ++i) {
		const int l = i < lparts.size() ? lparts[i] : 0;
		const int r = i < rparts.size() ? rparts[i] : 0;
		if (l < r) return -1;
		if (l > r) return 1;
	}
	return 0;
}

std::string fetch_uri_content(const std::string& uri)
{
	try {
		auto file = Gio::File::create_for_uri(uri);
		auto stream = file->read();
		if (!stream)
			return std::string();
		std::string buffer;
		char chunk[4096];
		while (true) {
			const gssize read_bytes = stream->read(chunk, sizeof(chunk));
			if (read_bytes <= 0) break;
			buffer.append(chunk, static_cast<size_t>(read_bytes));
		}
		return buffer;
	} catch (const Glib::Error& err) {
		synfig::warning("Update check failed: %s", err.what().c_str());
	} catch (const std::exception& ex) {
		synfig::warning("Update check exception: %s", ex.what());
	}
	return std::string();
}

std::string extract_plain_version(const std::string& body)
{
	const auto pos = body.find_first_not_of(" \t\r\n");
	if (pos == std::string::npos)
		return std::string();
	const auto end = body.find_first_of("\r\n", pos);
	return body.substr(pos, end == std::string::npos ? body.size() - pos : end - pos);
}

using namespace update_checker;

bool ensure_update_check_consent()
{
	if (App::update_check_consent != UPDATE_CHECK_CONSENT_UNKNOWN)
		return App::update_check_consent == UPDATE_CHECK_CONSENT_ALLOWED;

	if (!App::main_window)
		return false;

	Gtk::MessageDialog dialog(
		*App::main_window,
		"",
		false,
		Gtk::MESSAGE_QUESTION,
		Gtk::BUTTONS_NONE,
		true);
	dialog.set_title(_("Check for Updates"));
	dialog.get_action_area()->set_layout(Gtk::BUTTONBOX_SPREAD);
	dialog.set_secondary_text(_(
		"Would you like to enable check for new versions of the application at startup?"
		"\n\n"
		"You can always change this setting in Preferences > System > Update check."));
	dialog.add_button(_("Enable"), 1);
	dialog.add_button(_("Disable"), 0);

	const int response = dialog.run();
	if (response == 1)
	{
		App::update_check_consent = UPDATE_CHECK_CONSENT_ALLOWED;
		App::enable_update_check = true;
		App::save_settings();
		return true;
	}
	if (response == 0)
	{
		App::update_check_consent = UPDATE_CHECK_CONSENT_DENIED;
		App::enable_update_check = false;
		App::save_settings();
		return false;
	}

	return false;
}

} // anonymous namespace

namespace studio { namespace update_checker {

void start_async()
{
	// Early exit if already running — avoids re-showing the consent dialog
	if (update_checker_started.load())
		return;

	if (!ensure_update_check_consent())
		return;

	// User previously consented but may have later disabled checks in Preferences
	if (!App::enable_update_check)
		return;

	// Atomic acquire: guarantees only one thread proceeds even under a race
	if (update_checker_started.exchange(true))
		return;

	std::thread([]() {
		const std::string check_url = is_development_snapshot ? check_url_dev : check_url_stable;
		const std::string landing_url = is_development_snapshot ? landing_url_dev : landing_url_stable;

		const std::string body = fetch_uri_content(check_url);
		if (body.empty())
			return;

		// Plain-text format: first non-empty line is version string
		const std::string remote_version = extract_plain_version(body);
		if (remote_version.empty())
			return;

		const std::string local_version = VERSION;
		if (compare_versions(local_version, remote_version) >= 0)
			return;

		// Don't notify if user chose to skip this version
		if (!App::skipped_update_version.empty()
			&& compare_versions(App::skipped_update_version, remote_version) >= 0)
			return;

		Glib::signal_idle().connect_once([remote_version, landing_url]() {
			if (!App::main_window)
				return;
			App::main_window->show_update_notification(landing_url, remote_version);
		});
	}).detach();
}

} } // namespace studio::update_checker
