#pragma once
#include "api.h"
#include "RHI/ResourceManager/resource_allocator.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class HD_USTC_CG_API ProgramVars {
   public:
    ProgramVars(ResourceAllocator& r);

    template<typename... Args>
    ProgramVars(
        ResourceAllocator& r,
        const ProgramHandle& program,
        Args&&... args);

    ~ProgramVars();

    void finish_setting_vars();

    nvrhi::IResource*& operator[](const std::string& name);

    void set_descriptor_table(
        const std::string& name,
        nvrhi::IDescriptorTable* table, BindingLayoutHandle layout_handle);
    // This is for setting extra settings
    void set_binding(
        const std::string& name,
        nvrhi::ITexture* resource,
        const nvrhi::TextureSubresourceSet& subset = {});
    nvrhi::BindingSetVector get_binding_sets() const;
    nvrhi::BindingLayoutVector& get_binding_layout();
    std::vector<IProgram*> get_programs() const;

   private:
    nvrhi::BindingLayoutVector binding_layouts;

    nvrhi::static_vector<nvrhi::BindingSetItemArray, nvrhi::c_MaxBindingLayouts>
        binding_spaces;

    nvrhi::static_vector<nvrhi::BindingSetHandle, nvrhi::c_MaxBindingLayouts>
        binding_sets_solid;

    nvrhi::static_vector<nvrhi::IDescriptorTable*, nvrhi::c_MaxBindingLayouts>
        descriptor_tables;

    ResourceAllocator& resource_allocator_;
    std::vector<IProgram*> programs;

    unsigned get_binding_space(const std::string& name);
    unsigned get_binding_id(const std::string& name);

    nvrhi::ResourceType get_binding_type(const std::string& name);
    std::tuple<unsigned, unsigned> get_binding_location(
        const std::string& name);

    ShaderReflectionInfo final_reflection_info;
};

template<typename... Args>
ProgramVars::ProgramVars(
    ResourceAllocator& r,
    const ProgramHandle& program,
    Args&&... args)
    : ProgramVars(r, std::forward<Args>(args)...)
{
    programs.push_back(program.Get());
    final_reflection_info += program.Get()->get_reflection_info();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE