#pragma once
#include <map>

#include "RHI/api.h"
#include "RHI/internal/nvrhi_patch.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
class RHI_API ShaderReflectionInfo {
   public:
    [[nodiscard]] const nvrhi::BindingLayoutDescVector&
    get_binding_layout_descs() const;
    unsigned get_binding_space(const std::string& name);
    unsigned get_binding_location(const std::string& name);
    nvrhi::ResourceType get_binding_type(const std::string& name);

    ShaderReflectionInfo operator+(const ShaderReflectionInfo& other) const;
    ShaderReflectionInfo& operator+=(const ShaderReflectionInfo& other);

   private:
    nvrhi::BindingLayoutDescVector binding_spaces;
    std::map<std::string, std::tuple<unsigned, unsigned>> binding_locations;

    friend class ShaderFactory;
    friend RHI_API std::ostream& operator<<(
        std::ostream& os,
        const ShaderReflectionInfo& info);
};

RHI_API std::ostream& operator<<(
    std::ostream& os,
    const ShaderReflectionInfo& info);
USTC_CG_NAMESPACE_CLOSE_SCOPE