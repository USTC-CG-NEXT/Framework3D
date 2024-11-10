#include "program_vars.hpp"

#include "RHI/ResourceManager/resource_allocator.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE

ProgramVars::~ProgramVars()
{
}

void ProgramVars::finish_setting_vars()
{
    for (int i = 0; i < binding_spaces.size(); ++i) {
        BindingSetDesc desc{};
        desc.bindings = binding_spaces[i];
    }
}

// This is based on reflection
unsigned ProgramVars::get_binding_space(const std::string& name)
{
    return final_reflection_info.get_binding_space(name);
}

// This is based on reflection
unsigned ProgramVars::get_binding_id(const std::string& name)
{
    return final_reflection_info.get_binding_location(name);
}

// This is based on reflection
nvrhi::ResourceType ProgramVars::get_binding_type(const std::string& name)
{
    return final_reflection_info.get_binding_type(name);
}

// This is where it is within the binding set
std::tuple<unsigned, unsigned> ProgramVars::get_binding_location(
    const std::string& name)
{
    unsigned binding_space_id = get_binding_space(name);

    if (binding_spaces.size() <= binding_space_id) {
        binding_spaces.resize(binding_space_id + 1);
    }

    auto& binding_space = binding_spaces[binding_space_id];

    auto pos = std::find_if(
        binding_space.begin(),
        binding_space.end(),
        [&name, this](const nvrhi::BindingSetItem& binding) {
            return binding.slot == get_binding_id(name) &&
                   binding.type == get_binding_type(name);
        });

    if (pos == binding_space.end()) {
        // Create it
        nvrhi::BindingSetItem item{};
        item.slot = get_binding_id(name);
        item.type = get_binding_type(name);
        binding_space.push_back(item);
        pos = binding_space.end() - 1;
    }

    unsigned binding_set_location = std::distance(binding_space.begin(), pos);

    return std::make_tuple(binding_space_id, binding_set_location);
}

nvrhi::IResource*& ProgramVars::operator[](const std::string& name)
{
    auto [binding_space_id, binding_set_location] = get_binding_location(name);

    return binding_spaces[binding_space_id][binding_set_location]
        .resourceHandle;
}

nvrhi::BindingSetVector ProgramVars::get_binding_sets() const
{
    nvrhi::BindingSetVector result;
    for (int i = 0; i < bindingSetsSolid_.size(); ++i) {
        result.push_back(bindingSetsSolid_[i].Get());
    }
    return result;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE