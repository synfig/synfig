#include "taskcommon.h"
#include <synfig/blur.h>

namespace synfig {
namespace rendering {

class TaskBlur: public TaskCommon {
private:
	synfig::Blur::Type type;
	Vector size;
	Task::Handle subtask;

public:
	TaskBlur(): type() {
		register_param(subtask);
	}

	synfig::Blur::Type get_type() const { return type; }
	void set_type(synfig::Blur::Type x) { type = x; }

	Vector get_size() const { return size; }
	void set_size(Vector x) { size = x; }

	const Task::Handle& get_subtask() const { return subtask; }
	void set_subtask(const Task::Handle& x) { subtask = x; }
};

}}
