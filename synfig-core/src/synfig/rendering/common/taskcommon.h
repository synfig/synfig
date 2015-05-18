#include "../task.h"

namespace synfig {
namespace rendering {

class TaskCommon: public Task {
public:
	void set_target_surface(const Surface &surface);
};

}}
