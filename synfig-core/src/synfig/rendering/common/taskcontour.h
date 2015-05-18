#include "taskcommon.h"
#include "contour.h"

namespace synfig {
namespace rendering {

class TaskContour: public TaskCommon {
private:
	Color color;
	Contour::Handle contour;

public:
	const Color& get_color() const { return color; }
	void set_color(const Color& x) const { color = x; }

	const Contour::Handle& get_contour() const { return contour; }
	void set_contour(const Contour::Handle& x) const { contour = x; }
};

}}
