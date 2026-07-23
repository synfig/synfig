#include "synfig/layer.h"
#include "synfig/real.h"
#include "synfig/value.h"
#include "synfig/valuenodes/valuenode_angle.h"
#ifdef USING_PCH  
# include "pch.h"  
#else  
# ifdef HAVE_CONFIG_H  
#  include <config.h>  
#include "synfig/paramdesc.h"
# endif  
#include <hb-ft.h>
#ifdef WITH_FONTCONFIG  
#include <fontconfig/fontconfig.h>  
#endif

#include "lyr_textgroup.h"  
  
#include <synfig/canvas.h>  
#include <synfig/context.h>  
#include <synfig/general.h>  
#include <synfig/localization.h>  
#include <glibmm/ustring.h>  
#include <synfig/valuenode.h>  
#include <synfig/layers/layer_shape.h>  
#include <synfig/rendering/primitive/contour.h>
#include "lyr_freetype.h"  
#include "text_processing.h"
#include <synfig/rendering/common/task/tasktransformation.h>
  
#endif  
  
using namespace synfig;  
  
  
extern FT_Library ft_library;  

/// NL/LF, VT, FF, CR, NEL, LS and PS
static const std::vector<uint32_t> line_endings{'\n', '\v', '\f', '\r', 0x0085, 0x2028, 0x2029};
   
SYNFIG_LAYER_INIT(Layer_TextGroup);  
SYNFIG_LAYER_SET_NAME(Layer_TextGroup,"text_group");  
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_TextGroup,N_("Text Group"));  
SYNFIG_LAYER_SET_CATEGORY(Layer_TextGroup,N_("Other"));  
SYNFIG_LAYER_SET_VERSION(Layer_TextGroup,"0.1");  
  

SYNFIG_LAYER_INIT(Layer_GlyphShape);  
SYNFIG_LAYER_SET_NAME(Layer_GlyphShape,"glyph_shape");  
SYNFIG_LAYER_SET_LOCAL_NAME(Layer_GlyphShape,N_("Glyph"));  
SYNFIG_LAYER_SET_CATEGORY(Layer_GlyphShape,CATEGORY_DO_NOT_USE);  
SYNFIG_LAYER_SET_VERSION(Layer_GlyphShape,"0.1");  
  
Layer_GlyphShape::Layer_GlyphShape()
    : param_scale(ValueBase(Vector(1.0, 1.0)))
    , param_rotation(ValueBase(Angle::zero()))
    , param_anim_offset(ValueBase(Vector(0.0, 0.0)))
    , wave_offset_(0,0) 
{
    SET_INTERPOLATION_DEFAULTS();
    SET_STATIC_DEFAULTS();
}  
  
Layer_GlyphShape::~Layer_GlyphShape() {}  
  
String  
Layer_GlyphShape::get_local_name() const  
{  
    return _("Glyph");  
}  
  
void Layer_GlyphShape::set_glyph_chunks(const rendering::Contour::ChunkList& chunks)  
{  
    stored_chunks = chunks;
    force_sync();
        
}  

void Layer_GlyphShape::set_wave_offset(const Vector& v)
{
   if (wave_offset_ != v) {  
        wave_offset_ = v;  
	}  
}
  
void Layer_GlyphShape::sync_vfunc()  
{  
    clear();  
    if (stored_chunks.empty()) return;  
    const Vector off = wave_offset_;
	if (off == Vector())
	{
    	add(stored_chunks);
    	return;
	}
	rendering::Contour::ChunkList shifted = stored_chunks;
	 
        for (auto& chunk : shifted) {  
            chunk.p1  += off;  
            chunk.pp0 += off;  
            chunk.pp1 += off;  
        }  
        
    add(shifted);  

}

void
Layer_TextGroup::on_canvas_set()
{
    Layer_PasteCanvas::on_canvas_set();
    rebuild_shared_registry();
}
 
  
Layer_TextGroup::Layer_TextGroup()  
    : param_text(ValueBase(std::string()))  
        , param_family(ValueBase(std::string("Sans Serif")))  
    , param_style(ValueBase(TEXT_STYLE_NORMAL))  
    , param_weight(ValueBase(TEXT_WEIGHT_NORMAL))  
    , param_size(ValueBase(Vector(0.25, 0.25)))  
    , param_compress(ValueBase(Real(1.0)))  
    , param_vcompress(ValueBase(Real(1.0)))  
    , param_orient(ValueBase(Vector(0.5, 0.5)))      
    , param_use_kerning(ValueBase(true))  
    , param_grid_fit(ValueBase(false))  
    , param_direction(ValueBase(0))                 
    , param_stagger_delay(ValueBase(Time(0.0)))      
    , param_font(ValueBase(std::string()))           
    , param_color(ValueBase(Color::black()))        
    , param_invert(ValueBase(false))
    , param_wave_amplitude(ValueBase(Real(0.05)))
    , param_wave_period(ValueBase(Time(1.0)))
    , param_share_target(ValueBase(String()))
{  
    SET_INTERPOLATION_DEFAULTS();  
    SET_STATIC_DEFAULTS();  
}  
  
Layer_TextGroup::~Layer_TextGroup(){}  
  
String  
Layer_TextGroup::get_local_name() const  
{  
    return _("Text Group");  
}  

void
Layer_TextGroup::request_full_resync()
{
    if (get_canvas()) get_canvas()->get_root()->signal_force_refresh()();
    sync_glyphs();
}

bool
Layer_TextGroup::set_param(const String& param, const ValueBase& value)
{
    // Structural params: change glyph shapes, count, or layout — need full resync.
    IMPORT_VALUE_PLUS(param_text,      request_full_resync());
    IMPORT_VALUE_PLUS(param_family,    request_full_resync());
    IMPORT_VALUE_PLUS(param_style,     request_full_resync());
    IMPORT_VALUE_PLUS(param_weight,    request_full_resync());
    IMPORT_VALUE_PLUS(param_size,      request_full_resync());
    IMPORT_VALUE_PLUS(param_direction, request_full_resync());
    IMPORT_VALUE_PLUS(param_compress,  request_full_resync());
    IMPORT_VALUE_PLUS(param_vcompress, request_full_resync());
    IMPORT_VALUE_PLUS(param_orient,    request_full_resync());
    IMPORT_VALUE_PLUS(param_font,      request_full_resync());

    // Glyph rebuild needed but no viewport force-refresh:
    IMPORT_VALUE_PLUS(param_use_kerning, sync_glyphs());
    IMPORT_VALUE_PLUS(param_grid_fit,    sync_glyphs());

    // Cosmetic params:no rebuild, no force_refresh.
    IMPORT_VALUE_PLUS(param_color, {
        Canvas::Handle canvas = get_sub_canvas();
        if (canvas) {
            Color color = param_color.get(Color());
            for (auto iter = canvas->begin(); iter != canvas->end(); ++iter)
                (*iter)->set_param("color", ValueBase(color));
        }
        changed();
    });
    IMPORT_VALUE_PLUS(param_invert, {
        Canvas::Handle canvas = get_sub_canvas();
        if (canvas) {
            bool invert = param_invert.get(bool());
            for (auto iter = canvas->begin(); iter != canvas->end(); ++iter)
                (*iter)->set_param("invert", ValueBase(invert));
        }
        changed();
    });

    // Wave animation params
    IMPORT_VALUE_PLUS(param_stagger_delay, {
        update_wave_offsets(get_time_mark(), true);
        if (get_canvas()) get_canvas()->get_root()->signal_force_refresh()();
    });
    IMPORT_VALUE_PLUS(param_wave_amplitude, {
        update_wave_offsets(get_time_mark(), true);
        if (get_canvas()) get_canvas()->get_root()->signal_force_refresh()();
    });
    IMPORT_VALUE_PLUS(param_wave_period, {
        update_wave_offsets(get_time_mark(), true);
        if (get_canvas()) get_canvas()->get_root()->signal_force_refresh()();
    });

    IMPORT_VALUE_PLUS(param_share_target, {
        String target = param_share_target.get(String());
        if (!target.empty())
            share_param(target);
        // immediately snap it back to empty so it can't be silently replayed
        param_share_target = ValueBase(String());
    });
    
    return Layer_PasteCanvas::set_param(param, value);
}

bool Layer_GlyphShape::set_param(const String &param, const ValueBase &value)  
{  
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
	EXPORT_VALUE(param_invert);
	EXPORT_VALUE(param_font);
	EXPORT_VALUE(param_stagger_delay);
	EXPORT_VALUE(param_wave_amplitude);
	EXPORT_VALUE(param_wave_period);
	EXPORT_VALUE(param_share_target);
    EXPORT_NAME();  
    EXPORT_VERSION();  
    return Layer_PasteCanvas::get_param(param);  
}  

ValueBase Layer_GlyphShape::get_param(const String &param) const  
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
        .set_description(_("The text to decompose into per-character layers"))
		.set_hint("paragraph")  
    ); 

    ret.push_back(ParamDesc("family")  
        .set_local_name(_("Font Family"))  
        .set_description(_("Name of the font family"))
        .set_hint("font_family")
    );  
      
   
	ret.push_back(ParamDesc("style")
		.set_local_name(_("Style"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(TEXT_STYLE_NORMAL, "normal" ,_("Normal"))
		.add_enum_value(TEXT_STYLE_OBLIQUE, "oblique" ,_("Oblique"))
		.add_enum_value(TEXT_STYLE_ITALIC, "italic" ,_("Italic"))
	);

	ret.push_back(ParamDesc("weight")
		.set_local_name(_("Weight"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(TEXT_WEIGHT_THIN, "thin" ,_("Thin"))
		.add_enum_value(TEXT_WEIGHT_ULTRALIGHT, "ultralight" ,_("Ultralight"))
		.add_enum_value(TEXT_WEIGHT_LIGHT, "light" ,_("Light"))
		.add_enum_value(TEXT_WEIGHT_BOOK, "book" ,_("Book"))
		.add_enum_value(TEXT_WEIGHT_NORMAL, "normal" ,_("Normal"))
		.add_enum_value(TEXT_WEIGHT_MEDIUM, "medium" ,_("Medium"))
		.add_enum_value(TEXT_WEIGHT_BOLD, "bold" ,_("Bold"))
		.add_enum_value(TEXT_WEIGHT_ULTRABOLD, "ultrabold" ,_("Ultrabold"))
		.add_enum_value(TEXT_WEIGHT_HEAVY, "heavy" ,_("Heavy"))
		.add_enum_value(TEXT_WEIGHT_ULTRAHEAVY, "ultraheavy" ,_("Ultraheavy"))
	);

	ret.push_back(ParamDesc("direction")
		.set_local_name(_("Direction"))
		.set_description(_("The text direction: left-to-right or right-to-left"))
		.set_hint("enum")
		.set_static(true)
		.add_enum_value(TEXT_DIRECTION_AUTO, "auto" ,_("Automatic"))
		.add_enum_value(TEXT_DIRECTION_LTR, "ltr" ,_("LTR"))
		.add_enum_value(TEXT_DIRECTION_RTL, "rtl" ,_("RTL"))
	);   


	ret.push_back(ParamDesc("compress")
		.set_local_name(_("Horizontal Spacing"))
		.set_description(_("Defines how close the glyphs are horizontally"))
	);

	ret.push_back(ParamDesc("vcompress")
		.set_local_name(_("Vertical Spacing"))
		.set_description(_("Defines how close the text lines are vertically"))
	);

	ret.push_back(ParamDesc("size")
		.set_local_name(_("Size"))
		.set_description(_("Size of the text"))
		.set_hint("size")
		.set_origin("origin")
		.set_is_distance()
	);

	ret.push_back(ParamDesc("orient")
		.set_local_name(_("Orientation"))
		.set_description(_("Text Orientation"))
		.set_invisible_duck()
	);

	ret.push_back(ParamDesc("font")
		.set_local_name(_("Font"))
		.set_description(_("Filename of the font to use"))
		.set_hint("filename")
		.not_critical()
		.hidden()
	);

	ret.push_back(ParamDesc("use_kerning")
		.set_local_name(_("Kerning"))
		.set_description(_("When checked, enables font kerning (If the font supports it)"))
	);	

	ret.push_back(ParamDesc("color")
		.set_local_name(_("Color"))
		.set_description(_("Color for the Text"))
	);

	ret.push_back(
    ParamDesc("invert")
        .set_local_name(_("Invert"))
        .set_description(_("Invert fill"))
	);

	ret.push_back(
    ParamDesc("grid_fit")
        .set_local_name(_("Grid Fit"))
        .set_description(_("Use grid fitting"))
	);
	ret.push_back(ParamDesc("stagger_delay")  
    	.set_local_name(_("Stagger Delay"))  
    	.set_description(_("Time offset between consecutive glyph animations"))
    	.set_hint("time")
	);
	ret.push_back(ParamDesc("wave_amplitude")  
    	.set_local_name(_("Wave Amplitude"))  
    	.set_description(_("Vertical amplitude of the wave effect"))  
    	.set_is_distance()  
	);  
	ret.push_back(ParamDesc("wave_period")  
    	.set_local_name(_("Wave Period"))  
    	.set_description(_("Duration of one full wave cycle"))  
	);
	ret.push_back(ParamDesc("share_target")
    	.set_local_name(_("Share Animation"))
   		.set_description(_("Connect all glyphs to the shared animation graph"))
	);
	return ret;  
}

Layer::Vocab Layer_GlyphShape::get_param_vocab() const  
{  
    Layer::Vocab ret(Layer_Shape::get_param_vocab());  
    ret.push_back(ParamDesc("rotation")  
        .set_local_name(_("Rotation"))  
        .set_description(_("Per-glyph rotation"))  
    );  
    ret.push_back(ParamDesc("scale")  
        .set_local_name(_("Scale"))  
        .set_description(_("Per-glyph scale"))  
        .set_is_distance()  
    );
    ret.push_back(ParamDesc("anim_offset") 
    	.set_local_name(_("Animation Offset"))
    	.set_origin("origin")
    	.set_description(_("Per-glyph animated position offset"))
    	.set_is_distance()
	);
 
    return ret;  
}

rendering::Task::Handle  
Layer_GlyphShape::build_composite_task_vfunc(ContextParams context_params) const  
{  
    rendering::Task::Handle task = Layer_Shape::build_composite_task_vfunc(context_params);  
  
    Angle rotation = param_rotation.get(Angle());  
    Vector scale = param_scale.get(Vector());  
    bool has_wave =
    wave_offset_[0] != 0.0 ||
    wave_offset_[1] != 0.0;

  	Vector anim_offset = param_anim_offset.get(Vector());
	
	if (anim_offset != Vector())
	{
    	auto translate =
        	new rendering::TaskTransformationAffine();

    	translate->transformation->matrix =
        	Matrix().set_translate(anim_offset);

    	translate->sub_task() = task;

    	task = translate;
	}

    if (rotation != Angle::zero() || scale != Vector(1.0, 1.0) || has_wave)  
    {  
        Vector pivot;
        Matrix matrix = Matrix().set_translate(pivot)  
                      * Matrix().set_rotate(rotation)  
                      * Matrix().set_scale(scale)  
                      * Matrix().set_translate(-pivot);  
  
        rendering::TaskTransformationAffine::Handle task_transform(  
            new rendering::TaskTransformationAffine());  
        task_transform->transformation->matrix = matrix;  
        task_transform->sub_task() = task;  
        task = task_transform;  
    }
    if (has_wave)   
	{
    	auto wave_translate = new rendering::TaskTransformationAffine();
    	wave_translate->transformation->matrix = Matrix().set_translate(wave_offset_);
    	wave_translate->sub_task() = task;
    	task = wave_translate;
	}
    return task;  
}

void  
Layer_TextGroup::attach_shared_nodes()
{  
    if (in_attach_shared_) return;
    in_attach_shared_ = true;
    
    Canvas::Handle canvas = get_sub_canvas();
    
    if (canvas) {
        for (auto& kv : shared_anim_nodes) {
            const String& param = kv.first;
            ValueNode::Handle node = kv.second;
            if (!node) continue;
            for (auto iter = canvas->begin(); iter != canvas->end(); ++iter) {
                Layer_GlyphShape::Handle g = Layer_GlyphShape::Handle::cast_dynamic(*iter);
                if (!g) continue;
                g->connect_dynamic_param(param, node);
            }
        }
    }

    in_attach_shared_ = false;
}

void
Layer_TextGroup::detach_shared_param(const String& param)
{
    // Currently unused.
	// Will be used when shared animations become removable.
	Canvas::Handle canvas = get_sub_canvas();
    if (canvas) {
        for (auto iter = canvas->begin(); iter != canvas->end(); ++iter) {
            Layer_GlyphShape::Handle g = Layer_GlyphShape::Handle::cast_dynamic(*iter);
            if (g) g->disconnect_dynamic_param(param);
        }
    }
    shared_anim_nodes.erase(param);
    changed();
}

void
Layer_TextGroup::rebuild_shared_registry()
{
    shared_anim_nodes.clear();
    Canvas::Handle canvas = get_sub_canvas();
    if (!canvas) return;

    for (auto iter = canvas->begin(); iter != canvas->end(); ++iter) {
        Layer_GlyphShape::Handle g = Layer_GlyphShape::Handle::cast_dynamic(*iter);
        if (!g) continue;
        for (auto& kv : g->dynamic_param_list()) {
            if (!kv.second->get_id().empty())      // only exported nodes count as "shared"
                shared_anim_nodes[kv.first] = kv.second;
        }
    }
}

Layer_GlyphShape::Handle
Layer_TextGroup::find_source_glyph() const
{
    Canvas::Handle canvas = get_sub_canvas();
    if (!canvas) return nullptr;

    // Find the current source glyph
    Layer_GlyphShape::Handle source_glyph;
    size_t i = 0;
    for (auto iter = canvas->begin(); iter != canvas->end(); ++iter, ++i)
    {
        Layer_GlyphShape::Handle g = Layer_GlyphShape::Handle::cast_dynamic(*iter);

        if (!g) continue;
        if (i == source_glyph_index_) return g;
        if (!source_glyph) source_glyph = g;   // fallback: first glyph seen
    }
    return source_glyph;
}

std::vector<String>
Layer_TextGroup::get_shareable_params() const
{
    std::vector<String> params;
    if (auto g = find_source_glyph())
        for (const auto& kv : g->dynamic_param_list())
            if (kv.second) params.push_back(kv.first);
    return params;
}

void
Layer_TextGroup::share_param(const String& param)
{
    Canvas::Handle canvas = get_sub_canvas();
    if (!canvas) return;

    Layer_GlyphShape::Handle source_glyph = find_source_glyph();

    if (!source_glyph)
    {
        synfig::error("Share: no glyph layers found");
        return;
    }

    const DynamicParamList& dpl = source_glyph->dynamic_param_list();
    auto it = dpl.find(param);

    if (it == dpl.end() || !it->second)
    {
        std::vector<String> valid = get_shareable_params();
   		String joined;
    	for (const String& p : valid)
        	joined += (joined.empty() ? "" : ", ") + p;
        synfig::error(
        	"Share: '%s' isn't animated on the source glyph (animated params available: %s)",
       	 	param.c_str(), joined.empty() ? "none" : joined.c_str());        
        return;
    }

    ValueNode::Handle node = it->second;

    if (node->get_id().empty())
    {
    	String id =
        	"textgroup_" +
            get_guid().get_string() +
            "_" +
            param;

        canvas->add_value_node(node, id);

        // Sanity check.
        ValueNode::Handle check =
            canvas->find_value_node(id, false);

        if (check != node)
        {
            synfig::error(
                "Share: failed to export shared graph '%s'",
                param.c_str());
            return;
        }
    }

    shared_anim_nodes[param] = node;

    attach_shared_nodes();

    changed();
}

void Layer_TextGroup::update_wave_offsets(Time time, bool force_sync_after) const  
{  
    Canvas::Handle canvas = get_sub_canvas();  
    if (!canvas) return;  
  
    Time stagger    = param_stagger_delay.get(Time());  
    Real dilation   = get_time_dilation();  
    Time toffset    = get_time_offset();  
    Real wave_amp   = param_wave_amplitude.get(Real());  
    Time wave_period = param_wave_period.get(Time());  
  
    int i = 0;  
    for (auto iter = canvas->begin(); iter != canvas->end(); ++iter, ++i) {  
        Layer_GlyphShape* gl = dynamic_cast<Layer_GlyphShape*>(iter->get());  
        if (!gl) continue;  
  
        Time glyph_time = time * dilation + toffset + Time(i * (double)stagger);  
        Vector wave_off(0.0, 0.0);  
        if (wave_amp != 0.0 && (double)wave_period != 0.0)  
            wave_off[1] = wave_amp * std::sin(2.0 * M_PI * (double)glyph_time  
                                              / (double)wave_period);  

        gl->set_wave_offset(wave_off);  

    }
	
}

void Layer_TextGroup::set_time_vfunc(IndependentContext context, Time time) const  
{  
    context.set_time(time);  
    Canvas::Handle canvas = get_sub_canvas();  
    if (!canvas) return;  
    update_wave_offsets(time, false);
        
    Time stagger  = param_stagger_delay.get(Time());
    Real dilation = get_time_dilation();
    Time toffset  = get_time_offset(); 

    int i = 0;
    for (auto iter = canvas->begin(); iter != canvas->end(); ++iter, ++i) {  
        Time glyph_time = time * dilation + toffset + Time(i * (double)stagger);  

		(*iter)->set_time(IndependentContext(canvas->end()), glyph_time);  		
          
    }  
}

Layer::Handle  
Layer_GlyphShape::clone(etl::loose_handle<Canvas> canvas, const GUID& deriv_guid) const  
{  
    Layer::Handle base = Layer_Shape::clone(canvas, deriv_guid);  
    Layer_GlyphShape* cloned = dynamic_cast<Layer_GlyphShape*>(base.get());  
    if (cloned){
        cloned->stored_chunks = stored_chunks;
        cloned->glyph_index_  = glyph_index_;
        cloned->cluster_ = cluster_;
    	cloned->line_index_   = line_index_;
    	cloned->base_y_       = base_y_;
    	cloned->wave_offset_  = wave_offset_;
    }
    return base;  
}


void  
Layer_TextGroup::sync_glyphs()  
{  
    if (param_text.get(String()).empty()) return;
    std::string text = param_text.get(std::string());  
  
    FT_Face face = Layer_Freetype::load_font_static(  
        param_family.get(std::string()),  
        param_style.get(int()),  
        param_weight.get(int()),  
        get_canvas() ? get_canvas()->get_file_path() : synfig::filesystem::Path()  
    );  

    Canvas::Handle canvas = get_sub_canvas();  
    if (!canvas) return;  
  
    if (text.empty() || !face) {  
        while (!canvas->empty())  
            canvas->erase(canvas->begin());  
        signal_subcanvas_changed()();  
        changed();  
        return;  
    }  
  
      
      
    const bool    grid_fit    = param_grid_fit.get(bool());  
    const Vector  orient      = param_orient.get(Vector());  
    const Real    compress    = param_compress.get(Real());  
    const Real    vcompress   = param_vcompress.get(Real());  
    const Color   color       = param_color.get(Color());  
    const bool    invert      = param_invert.get(bool());  
  
    const Vector size    = param_size.get(Vector()) * 2;  
    const Real   scale_x = size[0] / face->units_per_EM;  
    const Real   scale_y = size[1] / face->units_per_EM;  
  
    const int load_flags = grid_fit  
        ? FT_LOAD_NO_SCALE  
        : (FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING);  
    
    auto lines =  synfig::text_processing::fetch_text_lines(  
        text, param_direction.get(int())); 
    #if HAVE_HARFBUZZ
hb_font_t* font =
    Layer_Freetype::get_cached_hb_font(face);

    hb_font_set_scale(
    	font,
    	face->units_per_EM,
    	face->units_per_EM
	);

auto shaped_lines =
    synfig::text_processing::shape_text(lines, font);
#else
auto shaped_lines =
    synfig::text_processing::shape_text(lines);
#endif
     
    struct GlyphData {  
        rendering::Contour::ChunkList outline; 
        uint32_t glyph_index;
        uint32_t cluster;  
        Vector pen_offset;
        size_t line_index; //which source line this glyph belongs to
        Vector world_pos;  //final scaled+shifted position (second pass)
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

    		if (FT_Load_Glyph(face,glyph_index,load_flags))
        		continue;

       		rendering::Contour::ChunkList outline;

    		if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
    		{
        		synfig::text_processing::convert_outline_to_contours(&face->glyph->outline,outline);
        		        		
        		if (!outline.empty())
        		{
            		Vector glyph_pos = offset; 
    				glyph_pos[0] += sg.x_offset * compress;
    				glyph_pos[1] += sg.y_offset;

            		cur_line.push_back({
                	std::move(outline),
                	sg.glyph_index,
                	sg.cluster,
                	glyph_pos,
                	0,
                	Vector()
            		});
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
        shift[1] =  orient[1] * text_height_fu - initial_y; 
                    
        for (auto& glyph : line_glyphs[i])
		{
    		glyph.line_index = i;
    		glyph.world_pos[0] = (glyph.pen_offset[0] + shift[0]) * scale_x;
    		glyph.world_pos[1] = (glyph.pen_offset[1] + shift[1]) * scale_y;

    		for (auto& chunk : glyph.outline)
    		{
        		chunk.p1[0]  *= scale_x;  chunk.p1[1]  *= scale_y;
        		chunk.pp0[0] *= scale_x;  chunk.pp0[1] *= scale_y;
        		chunk.pp1[0] *= scale_x;  chunk.pp1[1] *= scale_y;
    		}

    		glyphs.push_back(std::move(glyph));

        }  
    }    
  
	std::vector<Layer::Handle> old_layers;
	std::vector<GlyphIdentity> old_identities;
	for (auto it = canvas->begin(); it != canvas->end(); ++it) {
    	old_layers.push_back(*it);
    	GlyphIdentity id{0, 0};
    	if (auto* gl = dynamic_cast<Layer_GlyphShape*>(it->get())) {
        	id.cluster     = gl->get_cluster();
        	id.glyph_index = gl->get_glyph_index();
    	}
    	old_identities.push_back(id);
	}

	std::vector<GlyphIdentity> new_identities;
	new_identities.reserve(glyphs.size());
	for (const auto& glyph : glyphs)
    	new_identities.push_back(GlyphIdentity{glyph.cluster, glyph.glyph_index});

	const size_t n = old_identities.size();  
	const size_t m = new_identities.size();  
	std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));  
	for (size_t i = 1; i <= n; ++i)  
    	for (size_t j = 1; j <= m; ++j)  
        	dp[i][j] = (old_identities[i-1] == new_identities[j-1])  
                   ? dp[i-1][j-1] + 1  
                   : std::max(dp[i-1][j], dp[i][j-1]);  
 	std::vector<Layer::Handle> reuse_for_new(m); // null -> create a new layer  
	{  
    	size_t i = n, j = m;  
    	while (i > 0 && j > 0) {  
        	if (old_identities[i-1] == new_identities[j-1]) {  
            	reuse_for_new[j-1] = old_layers[i-1]; // reuse -> keeps value nodes  
            	--i; --j;  
        	} else if (dp[i-1][j] >= dp[i][j-1]) {  
            	--i;                                     
        	} else {  
            	--j;                                     
        	}  
    	}  
	}  

	std::vector<Layer::Handle> new_order;  
	new_order.reserve(m);  
	for (size_t j = 0; j < m; ++j) {  
    	Layer::Handle matched = reuse_for_new[j];  
    	if (!matched)  
        	matched = Layer::Handle(new Layer_GlyphShape());  
    	new_order.push_back(matched);  
	}
  	canvas->clear();  
	for (auto& layer : new_order)  
   		canvas->push_back(layer);  

	auto layer_iter = canvas->begin();  
	for (const auto& glyph : glyphs) {  
    	Layer_GlyphShape* glyph_layer =  
        	dynamic_cast<Layer_GlyphShape*>(layer_iter->get());
        	std::string glyph_key =
    		"cluster_" + std::to_string(glyph.cluster) + "_glyph_" +std::to_string(glyph.glyph_index);

		(*layer_iter)->set_description(glyph_key);
    	if (glyph_layer) { 
			glyph_layer->set_param("origin", ValueBase(glyph.world_pos));
        	glyph_layer->set_glyph_chunks(glyph.outline);
        	glyph_layer->set_glyph_index(glyph.glyph_index);
        	glyph_layer->set_cluster( glyph.cluster);   
        	glyph_layer->set_line_index(glyph.line_index);
        	glyph_layer->set_base_y(glyph.world_pos[1]);	
	       	(*layer_iter)->set_param("color",  ValueBase(color));  
        	(*layer_iter)->set_param("invert", ValueBase(invert));
        	
    	}  
    	++layer_iter;  
	}  
	attach_shared_nodes();
    signal_subcanvas_changed()();  
    changed();  
}
