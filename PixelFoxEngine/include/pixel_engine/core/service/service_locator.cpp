#include "pch.h"
#include "service_locator.h"

namespace pixel_engine
{
    std::shared_mutex ServiceLocator::s_mutex{};
    fox::vector<ServiceLocator::Deleter> ServiceLocator::s_deleters{};
    std::unordered_map<std::type_index, void*> ServiceLocator::s_services{};
} // namespace pixel_engine