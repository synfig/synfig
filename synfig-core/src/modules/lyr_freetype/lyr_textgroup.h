#ifndef __SYNFIG_LYR_TEXTGROUP_H
#define __SYNFIG_LYR_TEXTGROUP_H

#include <synfig/layer.h>
#include <synfig/layers/layer_shape.h>
#include <synfig/layers/layer_pastecanvas.h>
#include <synfig/rendering/primitive/contour.h>
#include <synfig/value.h>
#include <synfig/string.h>
#include <sigc++/connection.h>
#include <set>
#include <map>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#if HAVE_HARFBUZZ
#include <hb.h>
#endif

enum class StaggerOrder
{
    STAGGER_ORDER_FORWARD = 0,
    STAGGER_ORDER_REVERSE = 1,
    STAGGER_ORDER_CENTER_OUT = 2,
    STAGGER_ORDER_RANDOM = 3
};

static const int SHARE_TARGET_NONE = 0;

class Layer_GlyphShape : public synfig::Layer_Shape
{
	SYNFIG_LAYER_MODULE_EXT

private:
	synfig::rendering::Contour::ChunkList stored_chunks;
	synfig::ValueBase param_scale;
	synfig::ValueBase param_rotation;
	synfig::ValueBase param_anim_offset;
	uint32_t glyph_index_ = 0;
	size_t line_index_ = 0;
	synfig::Real base_y_ = 0.0;
	uint32_t cluster_ = 0;
	mutable std::recursive_mutex params_mutex_;

public:
	Layer_GlyphShape();
	~Layer_GlyphShape();

	synfig::String get_local_name() const override;

	void set_glyph_chunks(const synfig::rendering::Contour::ChunkList& chunks);
	bool set_param(const synfig::String& param,
				   const synfig::ValueBase& value) override;
	synfig::ValueBase get_param(const synfig::String& param) const override;
	Layer::Vocab get_param_vocab() const override;
	void set_glyph_index(uint32_t gi) { glyph_index_ = gi; }
	uint32_t get_glyph_index() const { return glyph_index_; }
	void set_line_index(size_t i) { line_index_ = i; }
	size_t get_line_index() const { return line_index_; }
	void set_base_y(synfig::Real y) { base_y_ = y; }
	synfig::Real get_base_y() const { return base_y_; }
	void set_cluster(uint32_t c) { cluster_ = c; }
	uint32_t get_cluster() const { return cluster_; }

	virtual Layer::Handle
	clone(etl::loose_handle<synfig::Canvas> canvas,
		  const synfig::GUID& deriv_guid = synfig::GUID()) const override;

protected:
	void sync_vfunc() override;
	void set_time_vfunc(synfig::IndependentContext context, synfig::Time time) const override;
	synfig::rendering::Task::Handle build_composite_task_vfunc(
		synfig::ContextParams context_params) const override;
};

struct SharedEntry
{
	synfig::String target_param;
	synfig::Time delay;
	int order;
	synfig::ValueNode::RHandle node;
	bool valid = true;
	sigc::connection deleted_conn;
	std::map<synfig::Layer*, std::pair<synfig::Time, synfig::ValueNode::Handle>>
		wrapper_cache;
	std::map<synfig::Layer*, synfig::ValueNode::Handle> pre_share_nodes;
};

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
	synfig::ValueBase param_stagger_order;
	synfig::ValueBase param_stagger_seed;
	synfig::ValueBase param_share_target;
	synfig::ValueBase param_share_animations;

public:
	Layer_TextGroup();
	~Layer_TextGroup();

	bool set_param(const synfig::String& param,const synfig::ValueBase& value) override;
	bool connect_dynamic_param(const synfig::String& param,synfig::ValueNode::LooseHandle x) override;
	synfig::ValueBase get_param(const synfig::String& param) const override;
	synfig::Layer::Vocab get_param_vocab() const override;
	synfig::String get_local_name() const override;
	std::vector<int> stagger_perm_;
	void connect_shared_animations_signal(const synfig::ValueNode::LooseHandle& node);

private:
	void sync_glyphs();
	bool in_attach_shared_ = false;
	void detach_shared_param(const SharedEntry& entry);
	size_t source_glyph_index_ = 0;
	bool destructing_ = false;
	mutable std::set<synfig::String> pending_dynamic_cleanup_;
	void rebuild_stagger_permutation();
	void request_full_resync();
	Layer_GlyphShape::Handle find_source_glyph() const;
	std::vector<synfig::String> get_shareable_params() const;
	std::vector<SharedEntry> shared_entries_;
	synfig::String encode_shared_entry(const SharedEntry& e);
	bool decode_shared_entry(const synfig::String& s, SharedEntry& out);
	mutable bool pending_shared_rebuild_ = false;
    void rebuild_shared_entries_from_valuenode(const synfig::ValueNode::Handle& x);
    sigc::connection shared_animations_changed_conn_;

	struct ShareChoice
	{
		synfig::String param;
		bool already_shared = false;
		synfig::Time cur_delay;
		int cur_order = 0;
	};
	enum class ShareMode
	{
		SHARE,
		UNSHARE
	};
	struct ShareAction
	{
		synfig::String param;
		ShareMode mode = ShareMode::SHARE;

		ShareAction() = default;
		ShareAction(synfig::String p, ShareMode m)
			: param(std::move(p)), mode(m)
		{
		}
	};

	std::vector<ShareChoice> build_share_choices() const;
	mutable std::vector<ShareAction> last_share_actions_;

	bool resolve_and_export_node(SharedEntry& entry);
	bool share_param(const synfig::String& param, synfig::Time delay,
					 int order);
	bool unshare_param(const synfig::String& param);
	void on_shared_node_deleted(synfig::String target_param);

	void rebuild_shared_entries_from_param();
	void apply_shared_entries_from_items(const std::vector<synfig::AnimShare>& items);
	void retry_pending_shared_entries();
	void push_shared_animations_param();
	void attach_shared_entries();
	int ordinal_for_entry(const SharedEntry& entry, int index, int count) const;
	synfig::ValueNode::Handle
	find_wired_shared_node(const synfig::String& target_param) const;

protected:
	void on_canvas_set() override;
	virtual void set_time_vfunc(synfig::IndependentContext context,
								synfig::Time time) const override;
};

#endif
