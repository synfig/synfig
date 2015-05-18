#include "../task.h"
#include "surfacesw.h"

namespace synfig {
namespace rendering {

class TaskSW: public Task {
public:
	void set_target_surface(const SurfaceSW::Handle &surface);
};

}}
