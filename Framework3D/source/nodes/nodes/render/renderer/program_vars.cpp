#include "program_vars.hpp"

#include "RCore/ResourceAllocator.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE

ProgramVars::ProgramVars(
    const ProgramHandle& program,
    ResourceAllocator& resource_allocator)
    : resource_allocator_(resource_allocator)
{
}

ProgramVars::~ProgramVars()
{
}

void ProgramVars::finish_setting_vars()
{
    for (int i = 0; i < bindingSetItems_.size(); ++i) {
        BindingSetDesc desc{};
        desc.bindings = bindingSetItems_[i];
        bindingSetsSolid_[i] = resource_allocator_.create(desc);
    }
}

unsigned ProgramVars::get_binding_space(const std::string& name)
{
    return 0;
}

unsigned ProgramVars::get_binding_id(const std::string& name)
{
    return 0;
}

nvrhi::BindingSetItem& ProgramVars::operator[](const std::string& name)

{
    if (get_binding_space(name) >= bindingSetItems_.size()) {
        bindingSetItems_.resize(get_binding_space(name) + 1);
    }

    return bindingSetItems_[get_binding_space(name)][get_binding_id(name)];
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