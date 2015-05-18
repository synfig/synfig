#include "taskcommon.h"
#include "transformation.h"

namespace synfig {
namespace rendering {

class TaskTransformation: public TaskCommon {
private:
	Task::Handle subtask;
	Transformation::Handle transformation;

public:
	TaskTransformation() {
		register_param(subtask);
	}

	const Transformation::Handle& get_transformation() const { return transformation; }
	void set_transformation(const Transformation::Handle& x) { transformation = x; }

	const Task::Handle& get_subtask() const { return subtask; }
	void set_subtask(const Task::Handle& x) { subtask = x; }
};

}}
