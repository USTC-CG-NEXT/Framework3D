#pragma once

#include "RHI/ResourceManager/resource_allocator.hpp"


USTC_CG_NAMESPACE_OPEN_SCOPE
class ProgramVars {
   public:
    ProgramVars(
        const ProgramHandle& program,
        ResourceAllocator& r);
    ~ProgramVars();

    void finish_setting_vars();

    nvrhi::BindingSetItem& operator[](const std::string& name);
    nvrhi::BindingSetVector get_binding_sets() const;

   private:
    nvrhi::static_vector<nvrhi::BindingSetItemArray, nvrhi::c_MaxBindingLayouts>
        bindingSetItems_;

    nvrhi::static_vector<nvrhi::BindingSetHandle, nvrhi::c_MaxBindingLayouts>
        bindingSetsSolid_;
    ResourceAllocator& resource_allocator_;
    IProgram* program_;

    unsigned get_binding_space(const std::string& name);
    unsigned get_binding_id(const std::string& name);
};
USTC_CG_NAMESPACE_CLOSE_SCOPE