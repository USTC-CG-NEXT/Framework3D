#include "RHI/ShaderFactory/shader_reflection.hpp"

#include "Logger/Logger.h"
#include "RHI/ShaderFactory/shader.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

const nvrhi::BindingLayoutDescVector&
ShaderReflectionInfo::get_binding_layout_descs() const
{
    return binding_spaces;
}

unsigned ShaderReflectionInfo::get_binding_space(const std::string& name)
{
    auto it = binding_locations.find(name);
    if (it != binding_locations.end()) {
        return std::get<0>(it->second);
    }
    log::error("Binding space not found: %s", name.c_str());
    return -1;
}

unsigned ShaderReflectionInfo::get_binding_location(const std::string& name)
{
    auto it = binding_locations.find(name);
    if (it != binding_locations.end()) {
        return std::get<1>(it->second);
    }
    log::error("Binding location not found: %s", name.c_str());
    return -1;
}

nvrhi::ResourceType ShaderReflectionInfo::get_binding_type(
    const std::string& name)
{
    auto it = binding_locations.find(name);
    if (it != binding_locations.end()) {
        return binding_spaces[std::get<0>(it->second)]
            .bindings[std::get<1>(it->second)]
            .type;
    }
    log::error("Binding type not found: %s", name.c_str());
    return nvrhi::ResourceType::None;
}

ShaderReflectionInfo ShaderReflectionInfo::operator+(
    const ShaderReflectionInfo& other) const
{
    ShaderReflectionInfo result;
    result.binding_spaces = binding_spaces;

    auto larger_size =
        std::max(binding_spaces.size(), other.binding_spaces.size());

    result.binding_spaces.resize(larger_size);

    auto r_size = other.binding_spaces.size();
    for (int i = 0; i < r_size; ++i) {
        auto& r_space = other.binding_spaces[i];
        auto& l_space = result.binding_spaces[i];

        l_space.visibility = l_space.visibility | r_space.visibility;
    }

    for (const auto& [name, location] : other.binding_locations) {
        auto r_space_id = std::get<0>(location);
        auto r_location_id = std::get<1>(location);

        nvrhi::BindingLayoutItem r_binding_item =
            other.binding_spaces[r_space_id].bindings[r_location_id];

        // search in the first binding layout
        auto l_space_id = r_space_id;
        auto& l_space = result.binding_spaces[r_space_id];

        auto pos = std::find(
            l_space.bindings.begin(), l_space.bindings.end(), r_binding_item);

        if (pos == l_space.bindings.end()) {
            l_space.bindings.push_back(r_binding_item);
            unsigned new_l_location = l_space.bindings.size() - 1;
            result.binding_locations[name] =
                std::make_tuple(l_space_id, new_l_location);
        }
        else {
            result.binding_locations[name] =
                std::make_tuple(l_space_id, pos - l_space.bindings.begin());
        }
    }

    return result;
}

ShaderReflectionInfo& ShaderReflectionInfo::operator+=(
    const ShaderReflectionInfo& other)
{
    *this = *this + other;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const ShaderReflectionInfo& info)
{
    // print binding layout using binding locations
    for (const std::pair<const std::string, std::tuple<unsigned, unsigned>>&
             binding_location : info.binding_locations) {
        os << binding_location.first << " : ";
        auto space_id = std::get<0>(binding_location.second);
        auto location_id = std::get<1>(binding_location.second);

        os << "space: " << space_id << ", location: " << location_id << "; "
           << std::endl;
    }
    return os;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
