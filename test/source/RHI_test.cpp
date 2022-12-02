#include <string>

#include "RHI/RHI.hpp"

auto main() -> int
{
  auto const exported = exported_class {};

  return std::string("RHI") == exported.name() ? 0 : 1;
}
