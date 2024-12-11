/*
 * Copyright (c) 2014-2021, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <nvrhi/nvrhi.h>

#include <RHI/internal/nvrhi_equality.hpp>
#include <RHI/internal/nvrhi_hash.hpp>
#include <memory>
#include <unordered_map>

#include "api.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
class DescriptorTableManager;
typedef int DescriptorIndex;

// Stores a descriptor index in a descriptor table. Releases the descriptor when
// destroyed.
class HD_USTC_CG_API DescriptorHandle {
   private:
    std::weak_ptr<DescriptorTableManager> m_Manager;
    DescriptorIndex m_DescriptorIndex;

   public:
    DescriptorHandle();
    DescriptorHandle(
        const std::shared_ptr<DescriptorTableManager>& managerPtr,
        DescriptorIndex index);
    ~DescriptorHandle();

    [[nodiscard]] bool IsValid() const
    {
        return m_DescriptorIndex >= 0 && !m_Manager.expired();
    }
    [[nodiscard]] DescriptorIndex Get() const
    {
        if (m_DescriptorIndex >= 0)
            assert(!m_Manager.expired());
        return m_DescriptorIndex;
    }
    void Reset()
    {
        m_DescriptorIndex = -1;
        m_Manager.reset();
    }

    // Movable but non-copyable
    DescriptorHandle(const DescriptorHandle&) = delete;
    DescriptorHandle(DescriptorHandle&&) = default;
    DescriptorHandle& operator=(const DescriptorHandle&) = delete;
    DescriptorHandle& operator=(DescriptorHandle&&) = default;
};

class HD_USTC_CG_API DescriptorTableManager
    : public std::enable_shared_from_this<DescriptorTableManager> {
   protected:
    nvrhi::DeviceHandle m_Device;
    nvrhi::DescriptorTableHandle m_DescriptorTable;

    std::vector<nvrhi::BindingSetItem> m_Descriptors;
    std::unordered_map<nvrhi::BindingSetItem, DescriptorIndex>
        m_DescriptorIndexMap;
    std::vector<bool> m_AllocatedDescriptors;
    int m_SearchStart = 0;

   public:
    DescriptorTableManager(
        nvrhi::IDevice* device,
        nvrhi::IBindingLayout* layout);
    ~DescriptorTableManager();

    nvrhi::IDescriptorTable* GetDescriptorTable() const
    {
        return m_DescriptorTable;
    }

    DescriptorIndex CreateDescriptor(nvrhi::BindingSetItem item);
    DescriptorHandle CreateDescriptorHandle(nvrhi::BindingSetItem item);
    nvrhi::BindingSetItem GetDescriptor(DescriptorIndex index);
    void ReleaseDescriptor(DescriptorIndex index);
};
USTC_CG_NAMESPACE_CLOSE_SCOPE
