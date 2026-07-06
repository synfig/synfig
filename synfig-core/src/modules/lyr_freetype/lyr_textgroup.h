#ifndef __SYNFIG_LYR_TEXTGROUP_H  
#define __SYNFIG_LYR_TEXTGROUP_H  

#include <synfig/layer.h>
#include <synfig/layers/layer_shape.h>
#include <synfig/layers/layer_pastecanvas.h>  
#include <synfig/rendering/primitive/contour.h>
#include <synfig/value.h>
#include <synfig/string.h>

#include <ft2build.h>  
#include FT_FREETYPE_H  
#include FT_GLYPH_H  
#if HAVE_HARFBUZZ  
#include <hb.h>  
#endif  

class Layer_TextGroup : public synfig::Layer_PasteCanvas  
{  
    SYNFIG_LAYER_MODULE_EXT  
private:  
    synfig::ValueBase param_text;  
    synfig::ValueBase param_family;  
    synfig::ValueBase param_style;  
    synfig::ValueBase param_weight;  
    synfig::ValueBase param_size;  
    synfig::ValueBase param_compress;  
    synfig::ValueBase param_vcompress;  
    synfig::ValueBase param_orient;  
    synfig::ValueBase param_use_kerning;  
    synfig::ValueBase param_grid_fit;  
    synfig::ValueBase param_direction; 
    synfig::ValueBase param_stagger_delay;
    synfig::ValueBase param_font;
    synfig::ValueBase param_color;
    synfig::ValueBase param_invert;
	synfig::ValueBase param_wave_amplitude;
	synfig::ValueBase param_wave_period;
	synfig::ValueBase param_broadcast;

public:  
    Layer_TextGroup();  
    ~Layer_TextGroup();  
  
    bool set_param(const synfig::String & param, const synfig::ValueBase &value) override;  
    synfig::ValueBase get_param(const synfig::String &param) const override;  
    synfig::Layer::Vocab get_param_vocab() const override;  
    synfig::String get_local_name() const override;
    
private:  
    void sync_glyphs();
	void update_wave_offsets(synfig::Time time, bool force_sync_after = false) const;    
	std::map<synfig::String, synfig::ValueNode::Handle> shared_anim_nodes;
	void attach_shared_nodes();
	bool in_attach_shared_ = false;
	void detach_shared_param(const synfig::String& param);
	size_t master_glyph_index_ = 0;

	void broadcast_dynamic_param(const synfig::String& param);   
	
protected:
    void on_canvas_set() override;     
    virtual void set_time_vfunc(synfig::IndependentContext context,
                            synfig::Time time) const override;

};  

class Layer_GlyphShape : public synfig::Layer_Shape  
{  
    SYNFIG_LAYER_MODULE_EXT  
  
private:  
    synfig::rendering::Contour::ChunkList stored_chunks;  
    synfig::ValueBase param_scale;
    synfig::ValueBase param_rotation;
    synfig::ValueBase param_offset;
    synfig::ValueBase param_anim_offset;
    mutable synfig::Vector wave_offset_;  

public:  
    Layer_GlyphShape();  
    ~Layer_GlyphShape();  
  
    synfig::String get_local_name() const override; 
        
    void set_glyph_chunks(const synfig::rendering::Contour::ChunkList& chunks);
    bool set_param(const synfig::String &param, const synfig::ValueBase &value) override;  
    synfig::ValueBase get_param(const synfig::String &param) const override;  
    Layer::Vocab get_param_vocab() const override;  
    void set_wave_offset(const synfig::Vector& v);

	virtual Layer::Handle clone(etl::loose_handle<synfig::Canvas> canvas,  
                            const synfig::GUID& deriv_guid = synfig::GUID()) const override;  
protected:  
    void sync_vfunc() override;

    synfig::rendering::Task::Handle build_composite_task_vfunc(  
   	    synfig::ContextParams context_params) const override;  
    
}; 
  
#endif
