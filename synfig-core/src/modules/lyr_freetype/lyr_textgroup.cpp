#ifdef USING_PCH  
# include "pch.h"  
#else  
# ifdef HAVE_CONFIG_H  
#  include <config.h>  
# endif  
  
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
{  
    param_rotation = ValueBase(Angle::zero());  
    param_scale    = ValueBase(Vector(1.0, 1.0));
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
    clear();  // Layer_Shape::clear() - protected, accessible from subclass  
    add(chunks);  // Layer_Shape::add(ChunkList) - protected, accessible from subclass  
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
    : face(nullptr)  
{  
    param_text  = ValueBase(std::string());  
    param_size  = ValueBase(Vector(0.25, 0.25));     
    param_family = ValueBase(std::string("Sans Serif"));  
    param_style  = ValueBase(int(0));  
    param_weight = ValueBase(int(0));  
    param_stagger_delay = ValueBase(Time(0.1));
    SET_INTERPOLATION_DEFAULTS();  
    SET_STATIC_DEFAULTS();  

    // TODO:
    // Prototype-stage hardcoded font loading.
    // Final implementation should reuse Synfig's
    // existing font discovery/loading pipeline.  
    FT_New_Face(ft_library,  
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",  
        0, &face);  
}  
  
Layer_TextGroup::~Layer_TextGroup()  
{  
    if (face)  
        FT_Done_Face(face);  
}  
  
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
        // Build transform: translate to origin, scale, rotate, translate back  
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

    if (text.empty() || !face)
        return;

    Canvas::Handle canvas = get_sub_canvas();

    if (!canvas)
        return;

    // TODO:
    // Current implementation uses Glib::ustring iteration as an
    // intermediate improvement over raw byte iteration.
    //
    // Final implementation should use HarfBuzz cluster-aware shaping
    // and preserve glyph/codepoint mapping correctly for ligatures,
    // RTL text, combining marks, etc.

    Glib::ustring utf8_text(text);

    struct GlyphData
    {
        rendering::Contour::ChunkList outline;
        uint32_t charcode;

        // TODO:
        // Future implementation may use stored glyph metadata
        // for stable cluster/glyph identity tracking.
        Vector offset;
    };

    std::vector<GlyphData> glyphs;

    Vector size = param_size.get(Vector()) * 2;

    Real scale_x = size[0] / face->units_per_EM;
    Real scale_y = size[1] / face->units_per_EM;

    Vector offset(0, 0);

    for (auto it = utf8_text.begin(); it != utf8_text.end(); ++it)
    {
        uint32_t charcode = *it;

        // TODO:
        // Current implementation maps Unicode codepoints
        // directly to glyph indices.
        //
        // Final implementation should use HarfBuzz-shaped
        // glyph clusters instead.

        FT_UInt glyph_index =
            FT_Get_Char_Index(face, charcode);

        if (!glyph_index)
            continue;

        FT_Error error =
            FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE);

        if (error)
            continue;

        FT_Glyph ftglyph;

        error = FT_Get_Glyph(face->glyph, &ftglyph);

        if (error)
            continue;

        rendering::Contour::ChunkList outline;

        if (ftglyph->format == FT_GLYPH_FORMAT_OUTLINE)
        {
            FT_OutlineGlyph outline_glyph =
                reinterpret_cast<FT_OutlineGlyph>(ftglyph);

            Layer_Freetype::convert_outline_to_contours(
                outline_glyph,
                outline
            );
        }

        if (!outline.empty())
        {
            Layer_Freetype::shift_contour_chunks(
                outline,
                offset
            );

            for (auto &chunk : outline)
            {
                chunk.p1[0]  *= scale_x;
                chunk.p1[1]  *= scale_y;

                chunk.pp0[0] *= scale_x;
                chunk.pp0[1] *= scale_y;

                chunk.pp1[0] *= scale_x;
                chunk.pp1[1] *= scale_y;
            }

            glyphs.push_back({
                outline,
                charcode,
                offset
            });
        }

        offset[0] += ftglyph->advance.x >> 10;
        offset[1] += ftglyph->advance.y >> 10;

        FT_Done_Glyph(ftglyph);
    }

    // Reuse existing glyph layers where possible
    // to preserve animation parameters and future
    // ValueNode connections.

    while (static_cast<size_t>(canvas->size()) > glyphs.size())
    {
        canvas->erase(--canvas->end());
    }

    while (static_cast<size_t>(canvas->size()) < glyphs.size())
    {
        Layer::Handle child(new Layer_GlyphShape());

        canvas->push_back(child);
    }

    auto layer_iter = canvas->begin();

    for (const auto &glyph : glyphs)
    {
        if (layer_iter == canvas->end())
            break;

        Layer::Handle child = *layer_iter;

        Layer_GlyphShape *glyph_layer =
            dynamic_cast<Layer_GlyphShape*>(child.get());

        if (glyph_layer)
        {
            glyph_layer->set_glyph_chunks(glyph.outline);

            child->set_description(
                Glib::ustring(1, glyph.charcode)
            );
        }

        ++layer_iter;
    }

    // TODO:
    // Current synchronization preserves layer instances
    // by index/order only.
    //
    // Future implementation should preserve stable glyph
    // identity across insertions/deletions using shaped
    // cluster mapping so animation state survives edits.

    signal_subcanvas_changed()();

    changed();
}
