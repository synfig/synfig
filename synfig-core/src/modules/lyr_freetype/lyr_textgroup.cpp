#include "synfig/layer.h"
#include "synfig/real.h"
#include "synfig/value.h"
#include "synfig/animshare.h"
#include "synfig/valuenodes/valuenode_angle.h"
#include "synfig/valuenodes/valuenode_animsharelist.h"
#ifdef USING_PCH
#include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "synfig/paramdesc.h"
#include <hb-ft.h>
#ifdef WITH_FONTCONFIG
#include <fontconfig/fontconfig.h>
#endif
#include <set>
#include "lyr_textgroup.h"

#include <synfig/canvas.h>
#include <synfig/context.h>
#include <synfig/general.h>
#include <synfig/localization.h>
#include <glibmm/ustring.h>
#include <synfig/valuenode.h>
#include <synfig/valuenodes/valuenode_timeoffset.h>
#include <synfig/valuenodes/valuenode_composite.h>
#include <synfig/layers/layer_shape.h>
#include <synfig/rendering/primitive/contour.h>
#include "lyr_freetype.h"
#include "text_processing.h"
#include <synfig/rendering/common/task/tasktransformation.h>
#include <random>
#include <algorithm>
#include <glibmm/main.h>
#endif

using namespace synfig;

extern FT_Library ft_library;

/// NL/LF, VT, FF, CR, NEL, LS and PS
static const std::vector<uint32_t> line_endings{'\n',	'\v',	'\f',  '\r',
												0x0085, 0x2028, 0x2029};

SYNFIG_LAYER_INIT(Layer_TextGroup);
SYNFIG_LAYER_SET_NAME(Layer_TextGroup, "text_group");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_TextGroup, N_("Text Group"));
SYNFIG_LAYER_SET_CATEGORY(Layer_TextGroup, N_("Other"));
SYNFIG_LAYER_SET_VERSION(Layer_TextGroup, "0.1");

SYNFIG_LAYER_INIT(Layer_GlyphShape);
SYNFIG_LAYER_SET_NAME(Layer_GlyphShape, "glyph_shape");
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_GlyphShape, N_("Glyph"));
SYNFIG_LAYER_SET_CATEGORY(Layer_GlyphShape, CATEGORY_DO_NOT_USE);
SYNFIG_LAYER_SET_VERSION(Layer_GlyphShape, "0.1");

Layer_GlyphShape::Layer_GlyphShape()
	: param_scale(ValueBase(Vector(1.0, 1.0))),
	  param_rotation(ValueBase(Angle::zero())),
	  param_anim_offset(ValueBase(Vector(0.0, 0.0)))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_GlyphShape::~Layer_GlyphShape()
{
}

String
Layer_GlyphShape::get_local_name() const
{
	return _("Glyph");
}

void
Layer_GlyphShape::set_glyph_chunks(const rendering::Contour::ChunkList& chunks)
{
    std::lock_guard<std::recursive_mutex> lock(params_mutex_);
	stored_chunks = chunks;
	force_sync();
}

void
Layer_GlyphShape::sync_vfunc()
{
	clear();
	if (stored_chunks.empty())
		return;
	add(stored_chunks);
}

void
Layer_GlyphShape::set_time_vfunc(IndependentContext context, Time time) const
{
	std::lock_guard<std::recursive_mutex> lock(params_mutex_);
	Layer_Shape::set_time_vfunc(context, time);
}

void
Layer_TextGroup::on_canvas_set()
{
    Layer_PasteCanvas::on_canvas_set();

    if (dynamic_param_list().count("share_target"))
        disconnect_dynamic_param("share_target");

    auto it = dynamic_param_list().find("share_animations");

    if (it != dynamic_param_list().end() && it->second)
    {
        rebuild_shared_entries_from_valuenode(it->second);
    }
    else
    {
        rebuild_shared_entries_from_param();
    }

    pending_shared_rebuild_ = true;
}

Layer_TextGroup::Layer_TextGroup()
	: param_text(ValueBase(std::string())),
	  param_family(ValueBase(std::string("Sans Serif"))),
	  param_style(ValueBase(TEXT_STYLE_NORMAL)),
	  param_weight(ValueBase(TEXT_WEIGHT_NORMAL)),
	  param_size(ValueBase(Vector(0.25, 0.25))),
	  param_compress(ValueBase(Real(1.0))),
	  param_vcompress(ValueBase(Real(1.0))),
	  param_orient(ValueBase(Vector(0.5, 0.5))),
	  param_use_kerning(ValueBase(true)), param_grid_fit(ValueBase(false)),
	  param_direction(ValueBase(0)), param_stagger_delay(ValueBase(Time(0.0))),
	  param_font(ValueBase(std::string())),
	  param_color(ValueBase(Color::black())),
	  param_stagger_order(ValueBase(int(StaggerOrder::STAGGER_ORDER_FORWARD))),
	  param_stagger_seed(ValueBase(int(0))),
	  param_share_target(ValueBase(int(SHARE_TARGET_NONE))),
	  param_share_animations(ValueBase(std::vector<AnimShare>()))
{
	SET_INTERPOLATION_DEFAULTS();
	SET_STATIC_DEFAULTS();
}

Layer_TextGroup::~Layer_TextGroup()
{
	destructing_ = true;
	for (auto& entry : shared_entries_)
	{
		if (entry.deleted_conn.connected())
			entry.deleted_conn.disconnect();
	}
}

String
Layer_TextGroup::get_local_name() const
{
	return _("Text Group");
}

void
Layer_TextGroup::request_full_resync()
{
	if (get_canvas())
		get_canvas()->get_root()->signal_force_refresh()();
	sync_glyphs();
}

void
Layer_TextGroup::perform_share_action_deferred(ShareAction act)
{
    switch (act.mode)
    {
        case ShareMode::SHARE:
        {
            Time delay = param_stagger_delay.get(Time());
            int order = param_stagger_order.get(int());
            if (!share_param(act.param, delay, order))
                synfig::warning("Share Animation: '%s' is not "
                                "an animated glyph parameter",
                                act.param.c_str());
            else if (get_canvas())
                get_canvas()->get_root()->signal_force_refresh()();
            break;
        }
        case ShareMode::UNSHARE:
        {
            if (unshare_param(act.param))
            {
                if (get_canvas())
                    get_canvas()->get_root()->signal_force_refresh()();
            }
            else
                synfig::warning("Share Animation: '%s' is not "
                                "currently shared",
                                act.param.c_str());
            break;
        }
    }
}

bool
Layer_TextGroup::set_param(const String& param, const ValueBase& value)
{
	// Structural params: change glyph shapes, count, or layout — need full
	// resync.
	IMPORT_VALUE_PLUS(param_text, request_full_resync());
	IMPORT_VALUE_PLUS(param_family, request_full_resync());
	IMPORT_VALUE_PLUS(param_style, request_full_resync());
	IMPORT_VALUE_PLUS(param_weight, request_full_resync());
	IMPORT_VALUE_PLUS(param_size, request_full_resync());
	IMPORT_VALUE_PLUS(param_direction, request_full_resync());
	IMPORT_VALUE_PLUS(param_compress, request_full_resync());
	IMPORT_VALUE_PLUS(param_vcompress, request_full_resync());
	IMPORT_VALUE_PLUS(param_orient, request_full_resync());
	IMPORT_VALUE_PLUS(param_font, request_full_resync());

	// Glyph rebuild needed but no viewport force-refresh:
	IMPORT_VALUE_PLUS(param_use_kerning, sync_glyphs());
	IMPORT_VALUE_PLUS(param_grid_fit, sync_glyphs());

	// Cosmetic params:no rebuild, no force_refresh.
	IMPORT_VALUE_PLUS(param_color, {
		Canvas::Handle canvas = get_sub_canvas();
		if (canvas)
		{
			Color color = param_color.get(Color());
			for (auto iter = canvas->begin(); iter != canvas->end(); ++iter)
				(*iter)->set_param("color", ValueBase(color));
		}
		changed();
	});

	// Stagger Delay/Order are *staging* values only — they do nothing on
	// their own. They're read at the moment a param name is committed via
	// "Add Shared Animation", which stamps them
	// onto that one SharedEntry. This keeps entries independent: rotation
	// can stagger at one rate, scale at another, and touching these sliders
	// never silently changes an entry you're not currently committing.
	IMPORT_VALUE_PLUS(param_stagger_delay, {
		if (get_canvas())
			get_canvas()->get_root()->signal_force_refresh()();
	});
	IMPORT_VALUE_PLUS(param_stagger_order, {
		if (get_canvas())
			get_canvas()->get_root()->signal_force_refresh()();
	});
	// Unlike delay/order, the seed isn't staged-then-stamped onto a
	// SharedEntry — stagger_perm_ is live, shared data that any
	// RANDOM-order entry reads immediately, so a seed change has to
	// rebuild it right away rather than waiting for the next glyph resync.
	IMPORT_VALUE_PLUS(param_stagger_seed, {
		rebuild_stagger_permutation();
		if (get_canvas())
			get_canvas()->get_root()->signal_force_refresh()();
	});

	IMPORT_VALUE_PLUS(
    param_share_target,
    (
        [&]()
        {
            if (dynamic_param_list().count("share_target"))
                pending_dynamic_cleanup_.insert("share_target");

            int action_idx = param_share_target.get(int());

            param_share_target = ValueBase(int(SHARE_TARGET_NONE));

            if (action_idx > 0 &&
                action_idx < (int)last_share_actions_.size())
            {
                ShareAction act = last_share_actions_[action_idx];

                // Defer the actual share/unshare + panel refresh until GTK
                // has finished emitting the combo box's "changed" signal.
                // Running it inline frees the combo box mid-callback -> SIGSEGV.
                Glib::signal_idle().connect_once(
                    sigc::bind(
                        sigc::mem_fun(*this,
                            &Layer_TextGroup::perform_share_action_deferred),
                        act));
            }
        })());

	IMPORT_VALUE_PLUS(param_share_animations, {
    	// If a dynamic AnimShareList is already connected, that's the source
    	// // of truth — connect_dynamic_param()/on_canvas_set() already rebuild
    	// // shared_entries_ from it. Don't fight that here.
    	if (!dynamic_param_list().count("share_animations")) {
        	rebuild_shared_entries_from_param();
    	}
	});

	return Layer_PasteCanvas::set_param(param, value);
}

bool
Layer_GlyphShape::set_param(const String& param, const ValueBase& value)
{
    std::lock_guard<std::recursive_mutex> lock(params_mutex_);
	IMPORT_VALUE(param_rotation);
	IMPORT_VALUE(param_scale);
	IMPORT_VALUE(param_anim_offset);

	return Layer_Shape::set_param(param, value);
}

ValueBase
Layer_TextGroup::get_param(const String& param) const
{
	EXPORT_VALUE(param_text);
	EXPORT_VALUE(param_family);
	EXPORT_VALUE(param_style);
	EXPORT_VALUE(param_weight);
	EXPORT_VALUE(param_size);
	EXPORT_VALUE(param_direction);
	EXPORT_VALUE(param_compress);
	EXPORT_VALUE(param_vcompress);
	EXPORT_VALUE(param_orient);
	EXPORT_VALUE(param_use_kerning);
	EXPORT_VALUE(param_grid_fit);
	EXPORT_VALUE(param_color);
	EXPORT_VALUE(param_font);
	EXPORT_VALUE(param_stagger_delay);
	EXPORT_VALUE(param_stagger_order);
	EXPORT_VALUE(param_stagger_seed);
	EXPORT_VALUE(param_share_target);
	EXPORT_VALUE(param_share_animations);
	EXPORT_NAME();
	EXPORT_VERSION();
	return Layer_PasteCanvas::get_param(param);
}

ValueBase
Layer_GlyphShape::get_param(const String& param) const
{
	EXPORT_VALUE(param_rotation);
	EXPORT_VALUE(param_scale);
	EXPORT_VALUE(param_anim_offset);
	EXPORT_NAME();
	EXPORT_VERSION();
	return Layer_Shape::get_param(param);
}

Layer::Vocab
Layer_TextGroup::get_param_vocab() const
{
	Layer::Vocab ret(Layer_PasteCanvas::get_param_vocab());
	ret.push_back(ParamDesc("text")
					  .set_local_name(_("Text"))
					  .set_description(
						  _("The text to decompose into per-character layers"))
					  .set_hint("paragraph"));

	ret.push_back(ParamDesc("family")
					  .set_local_name(_("Font Family"))
					  .set_description(_("Name of the font family"))
					  .set_hint("font_family"));

	ret.push_back(
		ParamDesc("style")
			.set_local_name(_("Style"))
			.set_hint("enum")
			.set_static(true)
			.add_enum_value(TEXT_STYLE_NORMAL, "normal", _("Normal"))
			.add_enum_value(TEXT_STYLE_OBLIQUE, "oblique", _("Oblique"))
			.add_enum_value(TEXT_STYLE_ITALIC, "italic", _("Italic")));

	ret.push_back(
		ParamDesc("weight")
			.set_local_name(_("Weight"))
			.set_hint("enum")
			.set_static(true)
			.add_enum_value(TEXT_WEIGHT_THIN, "thin", _("Thin"))
			.add_enum_value(TEXT_WEIGHT_ULTRALIGHT, "ultralight",
							_("Ultralight"))
			.add_enum_value(TEXT_WEIGHT_LIGHT, "light", _("Light"))
			.add_enum_value(TEXT_WEIGHT_BOOK, "book", _("Book"))
			.add_enum_value(TEXT_WEIGHT_NORMAL, "normal", _("Normal"))
			.add_enum_value(TEXT_WEIGHT_MEDIUM, "medium", _("Medium"))
			.add_enum_value(TEXT_WEIGHT_BOLD, "bold", _("Bold"))
			.add_enum_value(TEXT_WEIGHT_ULTRABOLD, "ultrabold", _("Ultrabold"))
			.add_enum_value(TEXT_WEIGHT_HEAVY, "heavy", _("Heavy"))
			.add_enum_value(TEXT_WEIGHT_ULTRAHEAVY, "ultraheavy",
							_("Ultraheavy")));

	ret.push_back(
		ParamDesc("direction")
			.set_local_name(_("Direction"))
			.set_description(
				_("The text direction: left-to-right or right-to-left"))
			.set_hint("enum")
			.set_static(true)
			.add_enum_value(TEXT_DIRECTION_AUTO, "auto", _("Automatic"))
			.add_enum_value(TEXT_DIRECTION_LTR, "ltr", _("LTR"))
			.add_enum_value(TEXT_DIRECTION_RTL, "rtl", _("RTL")));

	ret.push_back(ParamDesc("compress")
					  .set_local_name(_("Horizontal Spacing"))
					  .set_description(
						  _("Defines how close the glyphs are horizontally")));

	ret.push_back(ParamDesc("vcompress")
					  .set_local_name(_("Vertical Spacing"))
					  .set_description(_(
						  "Defines how close the text lines are vertically")));

	ret.push_back(ParamDesc("size")
					  .set_local_name(_("Size"))
					  .set_description(_("Size of the text"))
					  .set_hint("size")
					  .set_origin("origin")
					  .set_is_distance());

	ret.push_back(ParamDesc("orient")
					  .set_local_name(_("Orientation"))
					  .set_description(_("Text Orientation"))
					  .set_invisible_duck());

	ret.push_back(ParamDesc("font")
					  .set_local_name(_("Font"))
					  .set_description(_("Filename of the font to use"))
					  .set_hint("filename")
					  .not_critical()
					  .hidden());

	ret.push_back(ParamDesc("use_kerning")
					  .set_local_name(_("Kerning"))
					  .set_description(_("When checked, enables font kerning "
										 "(If the font supports it)")));

	ret.push_back(ParamDesc("color")
					  .set_local_name(_("Color"))
					  .set_description(_("Color for the Text")));

	ret.push_back(ParamDesc("grid_fit")
					  .set_local_name(_("Grid Fit"))
					  .set_description(_("Use grid fitting")));
	ret.push_back(ParamDesc("stagger_delay")
					  .set_local_name(_("Stagger Delay"))
					  .set_description(
						  _("Time offset between consecutive glyph animations"))
					  .set_hint("time"));
	ret.push_back(
		ParamDesc("stagger_order")
			.set_local_name(_("Stagger Order"))
			.set_description(_("Order in which glyphs are staggered in time"))
			.set_hint("enum")
			.set_static(true)
			.add_enum_value(static_cast<int>(StaggerOrder::STAGGER_ORDER_FORWARD), "forward", _("Forward"))
			.add_enum_value(static_cast<int>(StaggerOrder::STAGGER_ORDER_REVERSE), "reverse", _("Reverse"))
			.add_enum_value(static_cast<int>(StaggerOrder::STAGGER_ORDER_CENTER_OUT), "center_out",	_("Center Out"))
			.add_enum_value(static_cast<int>(StaggerOrder::STAGGER_ORDER_RANDOM), "random", _("Random")));
		{
			last_share_actions_.clear();
			last_share_actions_.push_back(ShareAction{String(), ShareMode::SHARE});

			ParamDesc share_desc("share_target");
			share_desc.set_local_name(_("Add / Update / Remove Shared Animation"))
			.set_description(
				_("Select a glyph parameter to share (using the current "
				  "Stagger Delay/Order), re-time it, or unshare it"))
			.set_hint("enum")
			.set_static(true)
			.add_enum_value(SHARE_TARGET_NONE, "none", _("— Select —"));

		for (const auto& c : build_share_choices())
		{
			int share_idx = (int)last_share_actions_.size();
			last_share_actions_.push_back(
				ShareAction{c.param, ShareMode::SHARE});
			String share_label =
				c.already_shared
					? strprintf(
						  _("Share/Update: %s  (currently %.3fs, order %d)"),
						  c.param.c_str(), (double)c.cur_delay, c.cur_order)
					: (_("Share: ") + c.param);
			share_desc.add_enum_value(share_idx, "share_" + c.param,
									  share_label);

			int unshare_idx = (int)last_share_actions_.size();
			last_share_actions_.push_back(
				ShareAction{c.param, ShareMode::UNSHARE});
			share_desc.add_enum_value(unshare_idx, "unshare_" + c.param,
									  _("Unshare: ") + c.param);
		}
		ret.push_back(share_desc);
	}
	ret.push_back(
	ParamDesc("stagger_seed")
		.set_local_name(_("Stagger Random Seed"))
		.set_description(
			_("Seed used to generate the Random stagger order. Has no "
			  "effect unless Stagger Order is set to Random, but "
			  "remains visible/editable regardless so a seed can be "
			  "chosen in advance"))
		.set_hint("int"));

	ret.push_back(
		ParamDesc("share_animations")
			.set_local_name(_("Share Animation"))
			.set_description(_("Glyph parameters shared across all glyphs, "
							   "each with its own stagger delay/order"))
			.set_static(true));
			
	return ret;
}

Layer::Vocab
Layer_GlyphShape::get_param_vocab() const
{
	Layer::Vocab ret(Layer_Shape::get_param_vocab());
	ret.push_back(ParamDesc("rotation")
					  .set_local_name(_("Rotation"))
					  .set_description(_("Per-glyph rotation")));
	ret.push_back(ParamDesc("scale")
					  .set_local_name(_("Scale"))
					  .set_description(_("Per-glyph scale"))
					  .set_is_distance());
	ret.push_back(ParamDesc("anim_offset")
					  .set_local_name(_("Animation Offset"))
					  .set_origin("origin")
					  .set_description(_("Per-glyph animated position offset"))
					  .set_is_distance());

	return ret;
}

rendering::Task::Handle
Layer_GlyphShape::build_composite_task_vfunc(ContextParams context_params) const
{
    std::lock_guard<std::recursive_mutex> lock(params_mutex_);
	rendering::Task::Handle task =
		Layer_Shape::build_composite_task_vfunc(context_params);

	Angle rotation = param_rotation.get(Angle());
	Vector scale = param_scale.get(Vector());

	Vector anim_offset = param_anim_offset.get(Vector());

	if (anim_offset != Vector())
	{
		auto translate = new rendering::TaskTransformationAffine();

		translate->transformation->matrix = Matrix().set_translate(anim_offset);

		translate->sub_task() = task;

		task = translate;
	}

	if (rotation != Angle::zero() || scale != Vector(1.0, 1.0))
	{
		Vector pivot;
		Matrix matrix =
			Matrix().set_translate(pivot) * Matrix().set_rotate(rotation) *
			Matrix().set_scale(scale) * Matrix().set_translate(-pivot);

		rendering::TaskTransformationAffine::Handle task_transform(
			new rendering::TaskTransformationAffine());
		task_transform->transformation->matrix = matrix;
		task_transform->sub_task() = task;
		task = task_transform;
	}
	return task;
}

void
Layer_TextGroup::detach_shared_param(const SharedEntry& entry)
{
    const String& param = entry.target_param;

    Canvas::Handle canvas = get_sub_canvas();
    if (canvas)
    {
        std::set<Layer*> retimed;
        Layer_GlyphShape::Handle source = find_source_glyph();

        for (auto iter = canvas->begin(); iter != canvas->end(); ++iter)
        {
            Layer_GlyphShape::Handle g =
                Layer_GlyphShape::Handle::cast_dynamic(*iter);
            if (!g)
                continue;

            if (!g->dynamic_param_list().count(param))
                continue;

            if (g == source && entry.node)
            {
                g->connect_dynamic_param(param,ValueNode::Handle(entry.node.get()));
            }
            else
            {
                auto pre = entry.pre_share_nodes.find(g.get());
                if (pre != entry.pre_share_nodes.end() && pre->second)
                    g->connect_dynamic_param(param, ValueNode::Handle(pre->second.get()));
                else
                    g->disconnect_dynamic_param(param);
            }
            retimed.insert(g.get());
        }

        attach_shared_entries();

        if (!retimed.empty())
        {
            Time now = canvas->get_time();
            IndependentContext ctx(canvas->end());
            for (Layer* l : retimed)
                l->set_time(ctx, now);
        }
    }
    else
    {
        attach_shared_entries();
    }

    changed();
}

Layer_GlyphShape::Handle
Layer_TextGroup::find_source_glyph() const
{
	Canvas::Handle canvas = get_sub_canvas();
	if (!canvas)
		return nullptr;

	Layer_GlyphShape::Handle source_glyph;
	size_t i = 0;
	for (auto iter = canvas->begin(); iter != canvas->end(); ++iter, ++i)
	{
		Layer_GlyphShape::Handle g =
			Layer_GlyphShape::Handle::cast_dynamic(*iter);

		if (!g)
			continue;
		if (i == source_glyph_index_)
			return g;
		if (!source_glyph)
			source_glyph = g; // fallback: first glyph seen
	}
	return source_glyph;
}

std::vector<String>
Layer_TextGroup::get_shareable_params() const
{
	std::vector<String> params;
	if (auto g = find_source_glyph())
		for (const auto& kv : g->dynamic_param_list())
			if (kv.second)
				params.push_back(kv.first);
	return params;
}

std::vector<Layer_TextGroup::ShareChoice>
Layer_TextGroup::build_share_choices() const
{
	std::vector<ShareChoice> choices;
	for (const auto& p : get_shareable_params())
	{
		ShareChoice c;
		c.param = p;
		for (const auto& e : shared_entries_)
		{
			if (e.valid && e.target_param == p)
			{
				c.already_shared = true;
				c.cur_delay = e.delay;
				c.cur_order = e.order;
				break;
			}
		}
		choices.push_back(c);
	}
	return choices;
}

int
Layer_TextGroup::ordinal_for_entry(const SharedEntry& entry, int index,
								   int count) const
{
	if (count <= 0)
		return index;
	StaggerOrder stagger_order = static_cast<StaggerOrder>(entry.order);
	switch (stagger_order)
	{
		case StaggerOrder::STAGGER_ORDER_REVERSE:
			return (count - 1) - index;
		case StaggerOrder::STAGGER_ORDER_CENTER_OUT:
			return (int)std::floor(std::fabs(index - (count - 1) / 2.0));
		case StaggerOrder::STAGGER_ORDER_RANDOM:
			return (index < (int)stagger_perm_.size()) ? stagger_perm_[index]
													   : index;
		default:
			return index;
	}
}

void
Layer_TextGroup::attach_shared_entries()
{
	if (in_attach_shared_)
		return;
	in_attach_shared_ = true;

	if (Canvas::Handle canvas = get_sub_canvas())
	{
		int count = 0;
		for (auto it = canvas->begin(); it != canvas->end(); ++it)
			if (dynamic_cast<Layer_GlyphShape*>(it->get()))
				++count;

		std::set<Layer*>
			touched; // glyphs that got a newly-wired wrapper this pass

		for (auto& entry : shared_entries_)
		{
    		if (!entry.valid || !entry.node)
        		continue;

    		std::map<Layer*, std::pair<Time, ValueNode::Handle>> next_cache;

    		int i = 0;
    		for (auto iter = canvas->begin(); iter != canvas->end(); ++iter)
    		{
        		auto g = Layer_GlyphShape::Handle::cast_dynamic(*iter);
        		if (!g)
            		continue;

        		Time off(ordinal_for_entry(entry, i, count) *
                 	(double)entry.delay);

        		auto cached = entry.wrapper_cache.find(g.get());

        		ValueNode::Handle wrapper;

        		if (cached != entry.wrapper_cache.end() &&
            		cached->second.first == off)
        		{
            		wrapper = cached->second.second;
        		}
        		else
        		{
            		// Preserve the glyph's original animation before replacing it
            		// with the shared wrapper.
            		auto& dpl = g->dynamic_param_list();
            		auto existing = dpl.find(entry.target_param);

            		if (existing != dpl.end() &&
                		existing->second &&
                		!entry.pre_share_nodes.count(g.get()))
            		{
                		auto existing_wrapper = ValueNode_TimeOffset::Handle::cast_dynamic(existing->second);
    					bool already_is_share_wrapper = existing_wrapper &&
    						existing_wrapper->get_link("link").get() == entry.node.get();

    					if (!already_is_share_wrapper)

               			entry.pre_share_nodes[g.get()] = existing->second;
            		}

            		wrapper = ValueNode::Handle(
                		ValueNode_TimeOffset::create_with_offset(
                    		entry.node.get(), off));
            		}

        		next_cache[g.get()] = {off, wrapper};
        		auto& dpl = g->dynamic_param_list();
				auto current = dpl.find(entry.target_param);

				if (current == dpl.end() ||
 				   current->second != wrapper)
				{
    				g->connect_dynamic_param(entry.target_param,wrapper);
					touched.insert(g.get());
				}
        		++i;
    		}

    		entry.wrapper_cache = std::move(next_cache);
		}

		if (!touched.empty())
		{
			Time now = canvas->get_time();
			IndependentContext ctx(canvas->end());
			for (Layer* l : touched)
				l->set_time(ctx, now);
		}
	}

	in_attach_shared_ = false;
}

ValueNode::Handle
Layer_TextGroup::find_wired_shared_node(const String& target_param) const
{
	Canvas::Handle canvas = get_sub_canvas();
	if (!canvas)
		return nullptr;

	for (auto iter = canvas->begin(); iter != canvas->end(); ++iter)
	{
		auto g = Layer_GlyphShape::Handle::cast_dynamic(*iter);
		if (!g)
			continue;

		const DynamicParamList& dpl = g->dynamic_param_list();
		auto it = dpl.find(target_param);
		if (it == dpl.end() || !it->second)
			continue;

		auto wrapper = ValueNode_TimeOffset::Handle::cast_dynamic(it->second);
		if (!wrapper)
			continue;

		ValueNode::Handle inner = wrapper->get_link("link");
		if (inner && !inner->get_id().empty())
			return inner;
	}
	return nullptr;
}

bool
Layer_TextGroup::resolve_and_export_node(SharedEntry& entry)
{
	Canvas::Handle canvas = get_sub_canvas();
	if (!canvas)
		return false;

	// Already wired up (freshly loaded from file, or attached earlier
	// this session)? Just adopt it.
	ValueNode::Handle node = find_wired_shared_node(entry.target_param);

	if (!node)
	{
		// Genuinely new share: nothing connected yet. Export the source
		// glyph's own animated node under a fresh id. The string itself
		// never needs to be reproducible later — once it's written into
		// the file, every future load resolves it via the saved ":id"
		// reference (step 1 above), not by recomputing this string.
		Layer_GlyphShape::Handle source_glyph = find_source_glyph();
		if (!source_glyph)
		{
			synfig::error("Share: no glyph layers found");
			return false;
		}

		const DynamicParamList& dpl = source_glyph->dynamic_param_list();
		auto it = dpl.find(entry.target_param);
		if (it == dpl.end() || !it->second)
			return false;

		node = it->second;

		if (node->get_id().empty())
		{
			String id =
				"textgroup_" + GUID().get_string() + "_" + entry.target_param;
			try
			{
				canvas->add_value_node(node, id);
			}
			catch (const std::exception&)
			{
				synfig::error("Share: failed to export shared graph '%s'",
							  entry.target_param.c_str());
				return false;
			}
		}
	}

	// wrapper_cache is keyed only on (glyph, Time offset) — if the resolved
	// node differs from what this entry had before (e.g. it was deleted
	// and just got re-exported under a new id), any cached wrapper still
	// points at the old node. Drop the cache so attach_shared_entries()
	// is forced to rebuild every wrapper against the new node.
	if (entry.node.get() != node.get())
		entry.wrapper_cache.clear();

	entry.node = ValueNode::RHandle(node.get());
	entry.valid = true;

	entry.deleted_conn.disconnect();
	entry.deleted_conn = node->signal_deleted().connect(sigc::bind(
		sigc::mem_fun(*this, &Layer_TextGroup::on_shared_node_deleted),
		entry.target_param));

	return true;
}
bool
Layer_TextGroup::share_param(const String& param, Time delay, int order)
{
	// Already shared: treat this as "update this entry's stagger", not a
	// no-op. Only the one entry matching `param` is touched — every other
	// entry keeps whatever delay/order it was last committed with.
	for (auto& e : shared_entries_)
	{
		if (e.target_param == param)
		{
			if (e.delay != delay || e.order != order)
			{
				e.delay = delay;
				e.order = order;
				push_shared_animations_param();
				attach_shared_entries();
			}
			return true;
		}
	}

	std::vector<String> valid = get_shareable_params();
	if (std::find(valid.begin(), valid.end(), param) == valid.end())
		return false;

	SharedEntry entry;
	entry.target_param = param;
	entry.delay = delay;
	entry.order = order;

	if (!resolve_and_export_node(entry))
		return false;

	shared_entries_.push_back(entry);
	push_shared_animations_param();
	attach_shared_entries();
	return true;
}

bool
Layer_TextGroup::unshare_param(const String& param)
{
	auto it = std::find_if(shared_entries_.begin(), shared_entries_.end(),
						   [&](const SharedEntry& e)
						   { return e.target_param == param; });
	if (it == shared_entries_.end())
		return false;

	// Drop the bookkeeping entry before severing the glyph wiring:
	// detach_shared_param() ends with its own attach_shared_entries() call,
	// which would immediately re-wire this param if it were still present
	// in shared_entries_ at that point.
	SharedEntry entry = *it;
	shared_entries_.erase(it);
	detach_shared_param(entry);
	Glib::signal_idle().connect_once(
        sigc::mem_fun(*this, &Layer_TextGroup::push_shared_animations_param));
	return true;
}

void
Layer_TextGroup::on_shared_node_deleted(String target_param)
{
	if (destructing_)
		return;
	if (!get_canvas())
		return; // detached — likely an undo/redo snapshot being torn down, not
				// a live edit

	bool changed_any = false;
	for (auto& e : shared_entries_)
	{
		if (e.target_param == target_param)
		{
			e.valid = false;
			e.node = nullptr;
			if (e.deleted_conn.connected())
				e.deleted_conn.disconnect();
			changed_any = true;
		}
	}
	if (changed_any)
		push_shared_animations_param();
}

void
Layer_TextGroup::push_shared_animations_param()
{
	if (destructing_)
		return;

	auto existing = dynamic_param_list().find("share_animations");
	ValueNode_AnimShareList::Handle list_node = existing != dynamic_param_list().end()
		? ValueNode_AnimShareList::Handle::cast_dynamic(existing->second)
		: ValueNode_AnimShareList::Handle();

	if (!list_node)
	{
		std::vector<AnimShare> items;
		for (auto& e : shared_entries_)
			items.emplace_back(e.target_param, e.delay, e.order);

		list_node = ValueNode_AnimShareList::create(ValueBase(items), get_canvas());
		if (!list_node)
			return;

		connect_dynamic_param("share_animations", ValueNode::Handle(list_node));
	}
	else
	{
		list_node->clear();
		for (const auto& e : shared_entries_)
		{
			AnimShare item(e.target_param, e.delay, e.order);
			list_node->add(ValueNode::Handle(ValueNode_Composite::create(item)));
		}
	}

	changed();
	signal_dynamic_param_changed()("share_animations");
}

void
Layer_TextGroup::apply_shared_entries_from_items(const std::vector<AnimShare>& items)
{
	std::vector<SharedEntry> parsed;
	std::set<String> seen_params;

	for (const auto& item : items)
	{
		SharedEntry e;
		e.target_param = item.get_param();
		e.delay        = item.get_delay();
		e.order        = item.get_order();

		if (!seen_params.insert(e.target_param).second)
			continue;

		bool reused = false;
		for (auto& old_entry : shared_entries_)
		{
			if (old_entry.target_param == e.target_param)
			{
				e.node            = old_entry.node;
				e.valid           = old_entry.valid;
				e.deleted_conn    = old_entry.deleted_conn;
				e.wrapper_cache   = old_entry.wrapper_cache;
				e.pre_share_nodes = old_entry.pre_share_nodes;
				reused = true;
				break;
			}
		}
		if (!reused && !resolve_and_export_node(e))
			e.valid = false;

		parsed.push_back(std::move(e));
	}

	// Anything that dropped out of the incoming list needs its glyphs
	// detached — otherwise they stay wired to the old shared wrapper
	// forever, invisible to shared_entries_ from this point on.
	std::vector<SharedEntry> removed;
	for (auto& old_entry : shared_entries_)
	{
    	if (!seen_params.count(old_entry.target_param))
        	removed.push_back(std::move(old_entry));
	}

	shared_entries_ = std::move(parsed);

	for (auto& entry : removed)
    	detach_shared_param(entry);// disconnects that param + re-runs attach_shared_entries()

	attach_shared_entries();
}

void
Layer_TextGroup::rebuild_shared_entries_from_param()
{
	// The real persisted value is the dynamic AnimShareList — see the
	// comment on param_share_animations in the header for why we don't
	// read the static param here.
	auto existing = dynamic_param_list().find("share_animations");
	auto list_node = existing != dynamic_param_list().end()
		? ValueNode_AnimShareList::Handle::cast_dynamic(existing->second)
		: ValueNode_AnimShareList::Handle();

	if (!list_node)
	{
		shared_entries_.clear();
		attach_shared_entries();
		return;
	}

	std::vector<AnimShare> items;
	for (const auto& entry : list_node->list)
	{
		if (!entry.value_node)
			continue;

		auto composite = ValueNode_Composite::Handle::cast_dynamic(entry.value_node);
		if (!composite)
			continue;

		ValueBase value = (*composite)(Time(0));
		if (value.get_type() != type_anim_share)
			continue;

		items.push_back(value.get(AnimShare()));
	}

	apply_shared_entries_from_items(items);
}

void
Layer_TextGroup::rebuild_shared_entries_from_valuenode(const ValueNode::Handle& x)
{
	if (!x)
		return;

	ValueBase v = (*x)(Time(0));
	if (v.get_type() != type_list)
		return;

	std::vector<AnimShare> items;
	for (const ValueBase& item : v.get_list())
	{
		if (item.get_type() != type_anim_share)
			continue;
		items.push_back(item.get(AnimShare()));
	}

	apply_shared_entries_from_items(items);
}

bool
Layer_TextGroup::connect_dynamic_param(
    const String& param,
    ValueNode::LooseHandle x)
{
    bool ret = Layer_PasteCanvas::connect_dynamic_param(param, x);

    if (ret && param == "share_animations")
    {
       rebuild_shared_entries_from_valuenode(x);

        pending_shared_rebuild_ = true;
    }

    return ret;
}

void
Layer_TextGroup::retry_pending_shared_entries()
{
	bool any_resolved = false;
	for (auto& e : shared_entries_)
	{
		if (e.valid && e.node)
			continue;
		if (resolve_and_export_node(e))
			any_resolved = true;
	}
	if (any_resolved)
		attach_shared_entries();
}

void
Layer_TextGroup::set_time_vfunc(IndependentContext context, Time time) const
{
	context.set_time(time);
	Canvas::Handle canvas = get_sub_canvas();
	if (!canvas)
		return;

	if (!pending_dynamic_cleanup_.empty())
	{
		Layer_TextGroup* self = const_cast<Layer_TextGroup*>(this);
		for (const auto& p : pending_dynamic_cleanup_)
			self->disconnect_dynamic_param(p);
		pending_dynamic_cleanup_.clear();
	}
	if (pending_shared_rebuild_)
	{
    	Layer_TextGroup* self =
        	const_cast<Layer_TextGroup*>(this);

    	self->pending_shared_rebuild_ = false;

    	self->retry_pending_shared_entries();

    	self->attach_shared_entries();
	}

	Time base_time = time * get_time_dilation() + get_time_offset();
	for (auto iter = canvas->begin(); iter != canvas->end(); ++iter)
		(*iter)->set_time(IndependentContext(canvas->end()), base_time);
}

Layer::Handle
Layer_GlyphShape::clone(etl::loose_handle<Canvas> canvas,
						const GUID& deriv_guid) const
{
	Layer::Handle base = Layer_Shape::clone(canvas, deriv_guid);
	Layer_GlyphShape* cloned = dynamic_cast<Layer_GlyphShape*>(base.get());
	if (cloned)
	{
		cloned->stored_chunks = stored_chunks;
		cloned->glyph_index_ = glyph_index_;
		cloned->cluster_ = cluster_;
		cloned->line_index_ = line_index_;
		cloned->base_y_ = base_y_;
	}
	return base;
}

void
Layer_TextGroup::rebuild_stagger_permutation()
{
	// Always computed, not gated on the current global Stagger Order value:
	// an already-shared entry may have been committed with order=Random
	// even while the slider has since moved on to Forward for the next
	// share, and it still needs a valid permutation available.
	stagger_perm_.clear();

	Canvas::Handle canvas = get_sub_canvas();
	if (!canvas)
		return;

	uint32_t layer_salt = static_cast<uint32_t>(
    	param_stagger_seed.get(int()));

	std::vector<std::pair<uint64_t, size_t>> keyed;
	size_t i = 0;
	for (auto iter = canvas->begin(); iter != canvas->end(); ++iter, ++i)
	{
		auto* gl = dynamic_cast<Layer_GlyphShape*>(iter->get());
		if (!gl)
			continue;

		uint64_t c =
			(uint64_t(gl->get_cluster()) << 32) | gl->get_glyph_index();
		uint64_t h =
			c ^ (layer_salt + 0x9e3779b97f4a7c15ULL + (c << 6) + (c >> 2));
		h ^= h >> 33;
		h *= 0xff51afd7ed558ccdULL;
		h ^= h >> 33;
		h *= 0xc4ceb9fe1a85ec53ULL;
		h ^= h >> 33;
		keyed.push_back({h, i});
	}
	std::sort(keyed.begin(), keyed.end());

	stagger_perm_.resize(keyed.size());
	for (size_t rank = 0; rank < keyed.size(); ++rank)
		stagger_perm_[keyed[rank].second] = (int)rank;
}

String
Layer_TextGroup::encode_shared_entry(const SharedEntry& e)
{
	return e.target_param + "\t" + strprintf("%.6f", (double)e.delay) + "\t" +
		   std::to_string(e.order);
}

bool
Layer_TextGroup::decode_shared_entry(const String& s, SharedEntry& out)
{
	size_t p1 = s.find('\t');
	if (p1 == String::npos)
		return false;
	size_t p2 = s.find('\t', p1 + 1);
	if (p2 == String::npos)
		return false;

	out.target_param = s.substr(0, p1);
	out.delay = Time(atof(s.substr(p1 + 1, p2 - p1 - 1).c_str()));
	out.order = atoi(s.substr(p2 + 1).c_str());
	return true;
}

void
Layer_TextGroup::sync_glyphs()
{
	if (param_text.get(String()).empty())
		return;
	std::string text = param_text.get(std::string());

	FT_Face face = Layer_Freetype::load_font_static(
		param_family.get(std::string()), param_style.get(int()),
		param_weight.get(int()),
		get_canvas() ? get_canvas()->get_file_path()
					 : synfig::filesystem::Path());

	Canvas::Handle canvas = get_sub_canvas();
	if (!canvas)
		return;

	if (text.empty() || !face)
	{
		while (!canvas->empty())
			canvas->erase(canvas->begin());
		signal_subcanvas_changed()();
		changed();
		return;
	}

	const bool grid_fit = param_grid_fit.get(bool());
	const Vector orient = param_orient.get(Vector());
	const Real compress = param_compress.get(Real());
	const Real vcompress = param_vcompress.get(Real());
	const Color color = param_color.get(Color());

	const Vector size = param_size.get(Vector()) * 2;
	const Real scale_x = size[0] / face->units_per_EM;
	const Real scale_y = size[1] / face->units_per_EM;

	const int load_flags =
		grid_fit ? FT_LOAD_NO_SCALE : (FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING);

	auto lines = synfig::text_processing::fetch_text_lines(
		text, param_direction.get(int()));
#if HAVE_HARFBUZZ
	hb_font_t* font = Layer_Freetype::get_cached_hb_font(face);

	if (!font)
	{
		while (!canvas->empty())
			canvas->erase(canvas->begin());
		signal_subcanvas_changed()();
		changed();
		return;
	}

	hb_font_set_scale(font, face->units_per_EM, face->units_per_EM);

	auto shaped_lines = synfig::text_processing::shape_text(lines, font);
#else
	auto shaped_lines = synfig::text_processing::shape_text(lines);
#endif

	struct GlyphData
	{
		rendering::Contour::ChunkList outline;
		uint32_t glyph_index;
		uint32_t cluster;
		Vector pen_offset;
		size_t line_index; // which source line this glyph belongs to
		Vector world_pos;  // final scaled+shifted position (second pass)
	};
	struct GlyphIdentity
	{
		uint32_t cluster;
		uint32_t glyph_index;

		bool operator==(const GlyphIdentity& o) const
		{
			return cluster == o.cluster && glyph_index == o.glyph_index;
		}
	};

	std::vector<std::vector<GlyphData>> line_glyphs;
	std::vector<Real> line_widths;
	const Real initial_y = Real(face->ascender);

	Vector line_start(0, 0);

	for (const auto& shaped_line : shaped_lines)
	{
		std::vector<GlyphData> cur_line;

		Vector offset(0, line_start[1]);

		for (const auto& sg : shaped_line)
		{
			FT_UInt glyph_index = sg.glyph_index;

			if (FT_Load_Glyph(face, glyph_index, load_flags))
				continue;

			rendering::Contour::ChunkList outline;

			if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
			{
				synfig::text_processing::convert_outline_to_contours(
					&face->glyph->outline, outline);

				if (!outline.empty())
				{
					Vector glyph_pos = offset;
					glyph_pos[0] += sg.x_offset * compress;
					glyph_pos[1] += sg.y_offset;

					cur_line.push_back({std::move(outline), sg.glyph_index,
										sg.cluster, glyph_pos, 0, Vector()});
				}
			}

			offset[0] += sg.x_advance * compress;
			offset[1] += sg.y_advance;
		}
		line_widths.push_back(offset[0]);
		line_glyphs.push_back(std::move(cur_line));
		line_start[1] -= face->height * vcompress;
	}

	const Real text_height_fu =
		initial_y + (Real(line_glyphs.size()) - 1) * vcompress * face->height;

	std::vector<GlyphData> glyphs;

	for (size_t i = 0; i < line_glyphs.size(); i++)
	{
		Vector shift;
		shift[0] = -orient[0] * line_widths[i];
		shift[1] = orient[1] * text_height_fu - initial_y;

		for (auto& glyph : line_glyphs[i])
		{
			glyph.line_index = i;
			glyph.world_pos[0] = (glyph.pen_offset[0] + shift[0]) * scale_x;
			glyph.world_pos[1] = (glyph.pen_offset[1] + shift[1]) * scale_y;

			for (auto& chunk : glyph.outline)
			{
				chunk.p1[0] *= scale_x;
				chunk.p1[1] *= scale_y;
				chunk.pp0[0] *= scale_x;
				chunk.pp0[1] *= scale_y;
				chunk.pp1[0] *= scale_x;
				chunk.pp1[1] *= scale_y;
			}

			glyphs.push_back(std::move(glyph));
		}
	}

	std::vector<Layer::Handle> old_layers;
	std::vector<GlyphIdentity> old_identities;
	uint32_t fallback_ordinal = 0;
	for (auto it = canvas->begin(); it != canvas->end(); ++it)
	{
		old_layers.push_back(*it);

		// Sentinel: guaranteed not to collide with any real {cluster,
		// glyph_index} pair, and unique per old layer, so a parse failure never
		// spuriously matches another parse failure.
		GlyphIdentity id{std::numeric_limits<uint32_t>::max(),
						 fallback_ordinal++};

		if (dynamic_cast<Layer_GlyphShape*>(it->get()))
		{
			const std::string desc = (*it)->get_description();
			const auto cp = desc.find("cluster_");
			const auto gp = desc.rfind("_glyph_");
			if (cp != std::string::npos && gp != std::string::npos &&
				gp > cp + 8)
			{
				try
				{
					id.cluster = (uint32_t)std::stoul(
						desc.substr(cp + 8, gp - (cp + 8)));
					id.glyph_index = (uint32_t)std::stoul(desc.substr(gp + 7));
				}
				catch (...)
				{ /* keep sentinel */
				}
			}
		}
		old_identities.push_back(id);
	}

	std::vector<GlyphIdentity> new_identities;
	new_identities.reserve(glyphs.size());
	for (const auto& glyph : glyphs)
		new_identities.push_back(
			GlyphIdentity{glyph.cluster, glyph.glyph_index});

	const size_t n = old_identities.size();
	const size_t m = new_identities.size();

	std::vector<Layer::Handle> reuse_for_new(m); // null -> create a new layer

	bool unchanged_sequence = (n == m);
	if (unchanged_sequence)
		for (size_t k = 0; k < n; ++k)
			if (!(old_identities[k] == new_identities[k]))
			{
				unchanged_sequence = false;
				break;
			}

	if (unchanged_sequence)
	{
		for (size_t j = 0; j < m; ++j)
			reuse_for_new[j] = old_layers[j];
	}
	else
	{
		std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));
		for (size_t i = 1; i <= n; ++i)
			for (size_t j = 1; j <= m; ++j)
				dp[i][j] = (old_identities[i - 1] == new_identities[j - 1])
							   ? dp[i - 1][j - 1] + 1
							   : std::max(dp[i - 1][j], dp[i][j - 1]);
		size_t i = n, j = m;
		while (i > 0 && j > 0)
		{
			if (old_identities[i - 1] == new_identities[j - 1])
			{
				reuse_for_new[j - 1] = old_layers[i - 1];
				--i;
				--j;
			}
			else if (dp[i - 1][j] >= dp[i][j - 1])
			{
				--i;
			}
			else
			{
				--j;
			}
		}
	}

	std::vector<Layer::Handle> new_order;
	new_order.reserve(m);
	for (size_t j = 0; j < m; ++j)
	{
		Layer::Handle matched = reuse_for_new[j];
		if (!matched)
			matched = Layer::Handle(new Layer_GlyphShape());
		new_order.push_back(matched);
	}
	canvas->clear();
	for (auto& layer : new_order)
		canvas->push_back(layer);

	auto layer_iter = canvas->begin();
	for (const auto& glyph : glyphs)
	{
		Layer_GlyphShape* glyph_layer =
			dynamic_cast<Layer_GlyphShape*>(layer_iter->get());
		std::string glyph_key = "cluster_" + std::to_string(glyph.cluster) +
								"_glyph_" + std::to_string(glyph.glyph_index);

		(*layer_iter)->set_description(glyph_key);
		if (glyph_layer)
		{
			glyph_layer->set_param("origin", ValueBase(glyph.world_pos));
			glyph_layer->set_glyph_chunks(glyph.outline);
			glyph_layer->set_glyph_index(glyph.glyph_index);
			glyph_layer->set_cluster(glyph.cluster);
			glyph_layer->set_line_index(glyph.line_index);
			glyph_layer->set_base_y(glyph.world_pos[1]);
			(*layer_iter)->set_param("color", ValueBase(color));
		}
		++layer_iter;
	}
	rebuild_stagger_permutation();
	attach_shared_entries();
	signal_subcanvas_changed()();
	changed();
	retry_pending_shared_entries();
}
