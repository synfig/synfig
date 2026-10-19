/* === S Y N F I G ========================================================= */
/*!	\file xmlpp_compat.h
**	\brief Compatibility layer between libxml++ 5.0 and libxml++ 2.6
**
**	\legal
**	Copyright (c) 2025 Synfig Contributors
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

#ifndef SYNFIG_XMLPP_COMPAT_H
#define SYNFIG_XMLPP_COMPAT_H

/* === H E A D E R S ======================================================= */

#include <string>
#include <utility>

#include <libxml++/libxml++.h>

/* === M A C R O S ========================================================= */

/*! libxml++ 5.0 renamed a few DOM methods that Synfig relies on, so it and
**  libxml++ 2.6 do not share the same API for them. Rather than guarding every
**  call site with a version check, the affected methods are reached through the
**  free functions below, which forward to whichever method the available
**  libxml++ provides. They are named after the libxml++ 5.0 API, so dropping
**  libxml++ 2.6 support only means turning them back into member calls.
**
**  @c libxml++config.h is installed by libxml++ 2.42 and later (5.0 included),
**  so an undefined @c LIBXMLXX_MAJOR_VERSION means a 2.6 predating the renames.
*/
#if defined(LIBXMLXX_MAJOR_VERSION) && LIBXMLXX_MAJOR_VERSION >= 5
#	define SYNFIG_LIBXMLPP5 1
#endif

/* === F U N C T I O N S =================================================== */

namespace synfig
{

/** Appends a new child element to @a element.
 *
 * @param element the element to append the child to
 * @param args the remaining arguments of the libxml++ method
 * @returns the new child element
 */
template<typename... Args>
xmlpp::Element*
add_child_element(xmlpp::Element* element, Args&&... args)
{
#ifdef SYNFIG_LIBXMLPP5
	return element->add_child_element(std::forward<Args>(args)...);
#else
	return element->add_child(std::forward<Args>(args)...);
#endif
}

/** Sets the content of the first text child of @a element, adding a text child
 *  if the element does not have one yet.
 *
 * @param element the element to set the text of
 * @param text the new text content
 */
inline void
set_first_child_text(xmlpp::Element* element, const std::string& text)
{
#ifdef SYNFIG_LIBXMLPP5
	element->set_first_child_text(text);
#else
	element->set_child_text(text);
#endif
}

/** Returns the first text child of @a element.
 *
 * @param element the element to get the text child of
 * @returns the first text child, or @c nullptr when there is none
 */
inline xmlpp::TextNode*
get_first_child_text(xmlpp::Element* element)
{
#ifdef SYNFIG_LIBXMLPP5
	return element->get_first_child_text();
#else
	return element->get_child_text();
#endif
}

/** Returns the first text child of @a element.
 *
 * @param element the element to get the text child of
 * @returns the first text child, or @c nullptr when there is none
 */
inline const xmlpp::TextNode*
get_first_child_text(const xmlpp::Element* element)
{
#ifdef SYNFIG_LIBXMLPP5
	return element->get_first_child_text();
#else
	return element->get_child_text();
#endif
}

/** Detaches @a node from its parent and destroys it, along with its descendants.
 *
 * @param node the node to remove; it is unusable after this call
 */
inline void
remove_node(xmlpp::Node* node)
{
#ifdef SYNFIG_LIBXMLPP5
	xmlpp::Node::remove_node(node);
#else
	node->get_parent()->remove_child(node);
#endif
}

} // END of namespace synfig

/* === E N D =============================================================== */

#endif // SYNFIG_XMLPP_COMPAT_H