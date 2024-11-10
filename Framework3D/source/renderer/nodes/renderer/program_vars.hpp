#pragma once

#include "RHI/ResourceManager/resource_allocator.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class ProgramVars {
   public:
    ProgramVars(ResourceAllocator& r) : resource_allocator_(r)
    {
    }

    template<typename... T>
    ProgramVars(const ProgramHandle& program, T&&... args, ResourceAllocator& r)
        : ProgramVars(std::forward<T>(args)..., r)
    {
        programs.push_back(program.Get());
        final_reflection_info += program.Get()->get_reflection_info();
    }
    ~ProgramVars();

    void finish_setting_vars();

    nvrhi::IResource*& operator[](const std::string& name);
    nvrhi::BindingSetVector get_binding_sets() const;

   private:
    nvrhi::static_vector<nvrhi::BindingSetItemArray, nvrhi::c_MaxBindingLayouts>
        binding_spaces;

    nvrhi::static_vector<nvrhi::BindingSetHandle, nvrhi::c_MaxBindingLayouts>
        bindingSetsSolid_;
    ResourceAllocator& resource_allocator_;
    std::vector<IProgram*> programs;

    unsigned get_binding_space(const std::string& name);
    unsigned get_binding_id(const std::string& name);

    nvrhi::ResourceType get_binding_type(const std::string& name);
    std::tuple<unsigned, unsigned> get_binding_location(
        const std::string& name);

    ShaderReflectionInfo final_reflection_info;
};
USTC_CG_NAMESPACE_CLOSE_SCOPE