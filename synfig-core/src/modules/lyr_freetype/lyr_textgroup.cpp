#ifdef USING_PCH  
# include "pch.h"  
#else  
# ifdef HAVE_CONFIG_H  
#  include <config.h>  
#include "synfig/paramdesc.h"
# endif  

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
#include <synfig/rendering/common/task/tasktransformation.h>
  
#endif  
  
using namespace synfig;  
  
  
extern FT_Library ft_library;  
  
  
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
    clear();    
    add(chunks);    
    shape_contour().close();  
    
}  
  
void  
Layer_GlyphShape::sync_vfunc()  
{  
    clear();  
    if (!stored_chunks.empty())
	{  
        add(stored_chunks);    
	    shape_contour().close();
    } 
}

void
Layer_TextGroup::on_canvas_set()
{
    Layer_PasteCanvas::on_canvas_set();   
    sync_glyphs();
}
 
  
Layer_TextGroup::Layer_TextGroup()  
    : param_text(ValueBase(std::string()))  
    , param_family(ValueBase(std::string("Sans Serif")))  
    , param_style(ValueBase(0))  
    , param_weight(ValueBase(400))  
    , param_size(ValueBase(Vector(0.25, 0.25)))
    , param_direction(ValueBase(0))
    , param_compress(ValueBase(Real(1.0)))
	, param_vcompress(ValueBase(Real(1.0)))
	, param_orient(ValueBase(Vector(0.5, 0.5)))
	, param_use_kerning(ValueBase(true))
	, param_grid_fit(ValueBase(false))
	, param_color(ValueBase(Color::black()))
	, param_invert(ValueBase(false))
	, param_font(ValueBase(std::string()))
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
  
bool  
Layer_TextGroup::set_param(const String& param, const ValueBase& value)  
{  
    if (param == "text" && value.can_get(String())) 
    {   
        param_text = value;  
        sync_glyphs();  
        return true;  
    }  
    if (param == "family" && value.can_get(String())) {  
        param_family = value;
        sync_glyphs();  
        return true;  
    }  
    if (param == "style" && value.can_get(int())) {  
        param_style = value;  
		sync_glyphs();  
        return true;  
    }  
    if (param == "weight" && value.can_get(int())) {  
        param_weight = value;  
    	sync_glyphs();  
        return true;  
    }  
    if (param == "size" && value.can_get(Vector())) {  
        param_size = value;  
        sync_glyphs();  
        return true;  
    }  
    if (param == "direction" && value.can_get(int()))
	{
    	param_direction = value;
    	sync_glyphs();
    	return true;
	}

	if (param == "color")
	{
    	param_color = value;
    	sync_glyphs();
    	return true;
	}

	if (param == "compress" && value.can_get(Real(1.0)))
	{
    	param_compress = value;
    	sync_glyphs();
    	return true;
	}

	if (param == "vcompress" && value.can_get(Real(1.0)))
	{
    	param_vcompress = value;
    	sync_glyphs();
    	return true;
	}

	if (param == "orient" && value.can_get(Vector()))
	{
    	param_orient = value;
    	sync_glyphs();
    	return true;
	}

	if (param == "use_kerning" && value.can_get(bool()))
	{
    	param_use_kerning = value;
    	sync_glyphs();
    	return true;
	}

	if (param == "grid_fit" && value.can_get(bool()))
	{
    	param_grid_fit = value;
    	sync_glyphs();
    	return true;
	}

	if (param == "invert" && value.can_get(bool()))
	{
    	param_invert = value;
    	sync_glyphs();
    	return true;
	}

	if (param == "font" && value.can_get(String()))
	{
    	param_font = value;
    	sync_glyphs();
    	return true;
	}   
	return Layer_PasteCanvas::set_param(param, value);
      
} 

bool Layer_GlyphShape::set_param(const String &param, const ValueBase &value)  
{  
    IMPORT_VALUE(param_rotation);  
    IMPORT_VALUE(param_scale);  
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
    EXPORT_NAME();  
    EXPORT_VERSION();  
    return Layer_PasteCanvas::get_param(param);  
}  

ValueBase Layer_GlyphShape::get_param(const String &param) const  
{  
    EXPORT_VALUE(param_rotation);  
    EXPORT_VALUE(param_scale);  
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
    return ret;  
}

rendering::Task::Handle  
Layer_GlyphShape::build_composite_task_vfunc(ContextParams context_params) const  
{  
    rendering::Task::Handle task = Layer_Shape::build_composite_task_vfunc(context_params);  
  
    Angle rotation = param_rotation.get(Angle());  
    Vector scale = param_scale.get(Vector());  
  
    if (rotation != Angle::zero() || scale != Vector(1.0, 1.0))  
    {  
        Vector origin = param_origin.get(Vector());  
          
        Matrix matrix = Matrix().set_translate(origin)  
                      * Matrix().set_rotate(rotation)  
                      * Matrix().set_scale(scale)  
                      * Matrix().set_translate(-origin);  
  
        rendering::TaskTransformationAffine::Handle task_transform(  
            new rendering::TaskTransformationAffine());  
        task_transform->transformation->matrix = matrix;  
        task_transform->sub_task() = task;  
        task = task_transform;  
    }  
    return task;  
}

void Layer_TextGroup::set_time_vfunc(IndependentContext context, Time time) const  
{  
    context.set_time(time);  
    Canvas::Handle canvas = get_sub_canvas();  
    if (!canvas) return;  
      
    // Use base class behavior - no stagger for now  
    Real time_dilation = get_time_dilation();  
    Time time_offset = get_time_offset();  
    canvas->set_time(time * time_dilation + time_offset);  
}

void  
Layer_TextGroup::sync_glyphs()  
{  
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
  
      
    const bool    use_kerning = param_use_kerning.get(bool());  
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
    const FT_UInt kern_mode = grid_fit  
        ? FT_KERNING_DEFAULT  
        : FT_KERNING_UNFITTED;  

  	// TODO:
    // Replace codepoint iteration with HarfBuzz-shaped
    // glyph iteration so TextGroup uses the same shaping
    // pipeline as Layer_Freetype.
    auto lines = Layer_Freetype::fetch_text_lines(  
        text, param_direction.get(int()));  
  
    struct GlyphData {  
        rendering::Contour::ChunkList outline;  
        uint32_t charcode;  
    };  
  
    std::vector<std::vector<GlyphData>> line_glyphs;  
    std::vector<Real> line_widths;     
    Real initial_y = 0;                
  
    Vector line_start(0, 0);           
  
    for (const auto& line : lines)  
    {  
        std::vector<GlyphData> cur_line;  
        uint32_t prev_glyph_index = 0;  
        Vector offset(0, line_start[1]);  
  
        for (const auto& span : line)  
        {  
            for (uint32_t charcode : span.codepoints)  
            {  
                FT_UInt glyph_index = FT_Get_Char_Index(face, charcode);  
                if (!glyph_index) continue;  
  
                // Kerning  
                if (use_kerning && prev_glyph_index && FT_HAS_KERNING(face))  
                {  
                    FT_Vector delta;  
                    if (!FT_Get_Kerning(face, prev_glyph_index, glyph_index,  
                                        kern_mode, &delta))  
                    {  
                        offset[0] += delta.x * compress;  
                        offset[1] += delta.y * compress;  
                    }  
                }  
  
                if (FT_Load_Glyph(face, glyph_index, load_flags)) continue;  
  
                FT_Glyph ftglyph;  
                if (FT_Get_Glyph(face->glyph, &ftglyph)) continue;  
  
                rendering::Contour::ChunkList outline;  
  
                if (ftglyph->format == FT_GLYPH_FORMAT_OUTLINE)  
                {  
                    FT_OutlineGlyph og =  
                        reinterpret_cast<FT_OutlineGlyph>(ftglyph);  
                    Layer_Freetype::convert_outline_to_contours(og, outline);  
                    Layer_Freetype::shift_contour_chunks(outline, offset);  
  
                    if (line_glyphs.empty() && cur_line.empty())  
                    {  
                        FT_BBox bbox;  
                        FT_Glyph_Get_CBox(ftglyph,  
                            ft_glyph_bbox_subpixels, &bbox);  
                        initial_y = std::max(initial_y, Real(bbox.yMax));  
                    }  
  
                    if (!outline.empty())  
                        cur_line.push_back({outline, charcode});  
                }  
  
                offset[0] += (ftglyph->advance.x >> 10) * compress;  
                offset[1] += (ftglyph->advance.y >> 10);  
  
                FT_Done_Glyph(ftglyph);  
                prev_glyph_index = glyph_index;  
            }  
        }  
  
        line_widths.push_back(offset[0]);     
        line_glyphs.push_back(cur_line);  
        line_start[1] -= face->height * vcompress;   
    }
    const Real text_height_fu =  
        initial_y + (Real(line_glyphs.size()) - 1) * vcompress * face->height;  
  
    std::vector<GlyphData> glyphs;  
  
    for (size_t i = 0; i < line_glyphs.size(); i++)  
    {  
        Vector shift;  
        shift[0] = -orient[0] * line_widths[i];  
        shift[1] =  orient[1] * text_height_fu - initial_y  
                  - Real(i) * face->height * vcompress;  
  
        for (auto& glyph : line_glyphs[i])  
        {  
            Layer_Freetype::shift_contour_chunks(glyph.outline, shift);  
  
            for (auto& chunk : glyph.outline)  
            {  
                chunk.p1[0]  *= scale_x;  chunk.p1[1]  *= scale_y;  
                chunk.pp0[0] *= scale_x;  chunk.pp0[1] *= scale_y;  
                chunk.pp1[0] *= scale_x;  chunk.pp1[1] *= scale_y;  
            }  
  
            glyphs.push_back(glyph);  
        }  
    }  
  
    while (static_cast<size_t>(canvas->size()) > glyphs.size())  
        canvas->erase(--canvas->end());  
    while (static_cast<size_t>(canvas->size()) < glyphs.size())  
        canvas->push_back(Layer::Handle(new Layer_GlyphShape()));  
  
    auto layer_iter = canvas->begin();  
    for (const auto& glyph : glyphs)  
    {  
        if (layer_iter == canvas->end()) break;  
  
        Layer::Handle child = *layer_iter;  
        Layer_GlyphShape* glyph_layer =  
            dynamic_cast<Layer_GlyphShape*>(child.get());  
  
        if (glyph_layer)  
        {  
            glyph_layer->set_glyph_chunks(glyph.outline);  
            child->set_description(Glib::ustring(1, glyph.charcode));  
            child->set_param("color",  ValueBase(color));  
            child->set_param("invert", ValueBase(invert));  
        }  
  
        ++layer_iter;  
    }  

    // TODO:
    // Current synchronization preserves glyph layers
    // by index only.
    //
    // Future implementation should use HarfBuzz
    // cluster information to maintain stable glyph
    // identity across insertions, deletions and
    // ligature formation.
    signal_subcanvas_changed()();  
    changed();  
}
