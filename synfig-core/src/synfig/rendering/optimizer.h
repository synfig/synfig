namespace synfig
{
namespace rendering
{

class Optimizer: public etl::shared_object
{
public:
	virtual void optimize(const Task::Handle &task);
};

}
}
