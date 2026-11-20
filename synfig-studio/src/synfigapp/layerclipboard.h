// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef SYNFIGAPP_LAYERCLIPBOARD_H
#define SYNFIGAPP_LAYERCLIPBOARD_H

#include <synfig/canvas.h>
#include <synfig/layer.h>
#include <list>

namespace synfigapp {

// Repair bone references in nodes cloned with this operation's GUID.
// Also usable after Canvas::clone when embedding an imported canvas.
void relink_cloned_bones(const std::list<synfig::Layer::Handle>& source_layers,
	const synfig::GUID& guid);

// Clone one selection as a unit so dependent layers follow the cloned bones.
std::list<synfig::Layer::Handle> clone_layers_for_clipboard(
	const std::list<synfig::Layer::Handle>& layers,
	synfig::Canvas::LooseHandle canvas,
	const synfig::GUID& guid);

}
#endif
