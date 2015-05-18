#include "taskcommon.h"
#include "mesh.h"

namespace synfig {
namespace rendering {

class TaskMesh: public TaskCommon {
private:
	Mesh::Handle mesh;
	Task::Handle subtask;

public:
	TaskMesh() {
		register_param(subtask);
	}

	const Mesh::Handle& get_mesh() const { return mesh; }
	void set_mesh(const Mesh::Handle& x) { mesh = x; }

	const Task::Handle& get_subtask() const { return subtask; }
	void set_subtask(const Task::Handle& x) { subtask = x; }
};

}}
