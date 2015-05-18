#include "../task.h"
#include "surfacegl.h"

namespace synfig {
namespace rendering {

class TaskGL: public Task {
public:
	void set_target_surface(const SurfaceGL::Handle &surface);
};

}}
