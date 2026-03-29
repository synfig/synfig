#ifndef __SYNFIG_LAYER_FREEFORMDEFORM_H
#define __SYNFIG_LAYER_FREEFORMDEFORM_H

#include "layer_meshtransform.h"

namespace synfig {

class Layer_FreeFormDeform : public Layer_MeshTransform
{
	SYNFIG_LAYER_MODULE_EXT
private:
	//! Parameter : (list of Points) The control lattice
	ValueBase param_grid_points;

	//! Parameter : (Integer) e.g., 3 for a 3x3 grid
	ValueBase param_grid_size_x;
	ValueBase param_grid_size_y;

public:
	Layer_FreeFormDeform();
	virtual ~Layer_FreeFormDeform();

	virtual String get_local_name() const;
	virtual bool set_param(const String & param, const ValueBase & value);
	virtual ValueBase get_param(const String & param) const;
	virtual Vocab get_param_vocab() const;

	void prepare_mesh(); // The core math engine
};
}
#endif
