#include "taskcommon.h"

namespace synfig {
namespace rendering {

class TaskConvertSurface: public TaskCommon {
private:
	Task::Handle subtask;

public:
	TaskConvertSurface() {
		register_param(subtask);
	}

	const Task::Handle& get_subtask() const { return subtask; }
	void set_subtask(const Task::Handle& x) { subtask = x; }
};

}}
