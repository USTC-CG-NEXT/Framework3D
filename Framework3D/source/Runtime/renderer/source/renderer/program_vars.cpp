#include "program_vars.hpp"

#include <nvrhi/nvrhi.h>

#include "RHI/ResourceManager/resource_allocator.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE
ProgramVars::ProgramVars(ResourceAllocator& r) : resource_allocator_(r)
{
}

ProgramVars::~ProgramVars()
{
    for (int i = 0; i < binding_sets_solid.size(); ++i) {
        resource_allocator_.destroy(binding_sets_solid[i]);
    }
    for (int i = 0; i < binding_layouts.size(); ++i) {
        resource_allocator_.destroy(binding_layouts[i]);
    }
}

void ProgramVars::finish_setting_vars()
{
    for (int i = 0; i < binding_sets_solid.size(); ++i) {
        resource_allocator_.destroy(binding_sets_solid[i]);
    }
    binding_sets_solid.resize(0);
    binding_sets_solid.resize(binding_spaces.size());

    for (int i = 0; i < binding_spaces.size(); ++i) {
        if (!descriptor_tables[i]) {
            BindingSetDesc desc{};
            desc.bindings = binding_spaces[i];

            for (int j = 0; j < desc.bindings.size(); ++j) {
                if (dynamic_cast<nvrhi::IBuffer*>(
                        desc.bindings[j].resourceHandle)) {
                    desc.bindings[j].range = nvrhi::EntireBuffer;
                }
                else if (dynamic_cast<nvrhi::ITexture*>(
                             desc.bindings[j].resourceHandle)) {
                    desc.bindings[j].subresources = nvrhi::AllSubresources;
                }
            }
            binding_sets_solid[i] =
                resource_allocator_.create(desc, binding_layouts[i].Get());
        }
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
    auto binding_space = get_binding_space(name);
    auto binding_location = final_reflection_info.get_binding_location(name);

    auto slot = final_reflection_info.get_binding_layout_descs()[binding_space]
                    .bindings[binding_location]
                    .slot;

    return slot;
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
    if (descriptor_tables.size() <= binding_space_id) {
        descriptor_tables.resize(binding_space_id + 1);
    }

    auto& binding_space = binding_spaces[binding_space_id];

    auto& binding_layout = get_binding_layout()[binding_space_id];
    auto& layout_items = binding_layout->getDesc()->bindings;

    auto pos = std::find_if(
        layout_items.begin(),
        layout_items.end(),
        [&name, this](const nvrhi::BindingLayoutItem& binding) {
            return binding.slot == get_binding_id(name) &&
                   binding.type == get_binding_type(name);
        });

    assert(pos != layout_items.end());

    unsigned binding_set_location = std::distance(layout_items.begin(), pos);

    if (binding_set_location >= binding_space.size()) {
        binding_space.resize(binding_set_location + 1);
    }

    nvrhi::BindingSetItem& item = binding_space[binding_set_location];

    item.slot = get_binding_id(name);
    item.type = get_binding_type(name);
    item.subresources = nvrhi::AllSubresources;

    return std::make_tuple(binding_space_id, binding_set_location);
}

nvrhi::IResource*& ProgramVars::operator[](const std::string& name)
{
    auto [binding_space_id, binding_set_location] = get_binding_location(name);

    return binding_spaces[binding_space_id][binding_set_location]
        .resourceHandle;
}

void ProgramVars::set_descriptor_table(
    const std::string& name,
    nvrhi::IDescriptorTable* table,
    BindingLayoutHandle layout_handle)
{
    auto [binding_space_id, binding_set_location] = get_binding_location(name);
    descriptor_tables[binding_space_id] = table;

    if (binding_layouts[binding_space_id]) {
        resource_allocator_.destroy(binding_layouts[binding_space_id]);
        binding_layouts[binding_space_id] = layout_handle;
    }
}

void ProgramVars::set_binding(
    const std::string& name,
    nvrhi::ITexture* resource,
    const nvrhi::TextureSubresourceSet& subset)
{
    auto [binding_space_id, binding_set_location] = get_binding_location(name);
    auto& binding_set = binding_spaces[binding_space_id][binding_set_location];

    binding_set.resourceHandle = resource;
    binding_set.subresources = subset;
    if (subset.baseArraySlice != 0 || subset.numArraySlices != 1) {
        binding_set.dimension = nvrhi::TextureDimension::Texture2DArray;
    }
}

nvrhi::BindingSetVector ProgramVars::get_binding_sets() const
{
    nvrhi::BindingSetVector result;

    result.resize(binding_sets_solid.size());
    for (int i = 0; i < binding_sets_solid.size(); ++i) {
        if (binding_sets_solid[i]) {
            result[i] = binding_sets_solid[i].Get();
        }
        if (descriptor_tables[i]) {
            result[i] = descriptor_tables[i];
        }
    }
    return result;
}

nvrhi::BindingLayoutVector& ProgramVars::get_binding_layout()
{
    if (binding_layouts.empty()) {
        auto binding_layout_descs =
            final_reflection_info.get_binding_layout_descs();
        for (int i = 0; i < binding_layout_descs.size(); ++i) {
            auto binding_layout =
                resource_allocator_.create(binding_layout_descs[i]);
            binding_layouts.push_back(binding_layout);
        }
    }
    return binding_layouts;
}

std::vector<IProgram*> ProgramVars::get_programs() const
{
    return programs;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE