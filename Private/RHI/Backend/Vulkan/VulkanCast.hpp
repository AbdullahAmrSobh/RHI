
namespace RHI
{
namespace Vulkan
{

	template<typename Base, typename Drived>
	Drived Cast(Base& base)
	{
		return Drived(base.GetDevice());
	}

}
} // namespace RHI
