#include "taskcommon.h"

namespace synfig {
namespace rendering {

class TaskBlend: public TaskCommon {
private:
	Color::BlendMethod blend_method;
	Task::Handle subtask_a;
	Task::Handle subtask_b;

public:
	TaskBlend(): blend_method(Color::BLEND_COMPOSITE) {
		register_param(subtask_a);
		register_param(subtask_b);
	}

	Color::BlendMethod get_blend_method() const { return blend_method; }
	void set_blend_method(Color::BlendMethod x) const { blend_method = x; }

	const Task::Handle& get_subtask_a() const { return subtask_a; }
	void set_subtask_a(const Task::Handle& x) { subtask_a = x; }

	const Task::Handle& get_subtask_b() const { return subtask_b; }
	void set_subtask_b(const Task::Handle& x) { subtask_b = x; }
};

}}
