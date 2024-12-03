#pragma once
#include <RHI/api.h>

#include <cstdint>
#include <string>

#include "nvrhi/common/resource.h"



USTC_CG_NAMESPACE_OPEN_SCOPE
RHI_API void cuda_init();
RHI_API void cuda_shutdown();

//////////////////////////////////////////////////////////////////////////
// CUDA and OptiX
//////////////////////////////////////////////////////////////////////////

struct CUDALinearBufferDesc {
    int size;
    int element_size;

    CUDALinearBufferDesc(int size = 0, int element_size = 0)
        : size(size),
          element_size(element_size)
    {
    }

    friend class CUDALinearBuffer;
};

struct CUDASurfaceObjectDesc {
    uint32_t width;
    uint32_t height;
    uint64_t element_size;

    std::string debugName;

    CUDASurfaceObjectDesc(
        uint32_t width = 1,
        uint32_t height = 1,
        uint64_t element_size = 1)
        : width(width),
          height(height),
          element_size(element_size)
    {
    }

    friend class CUDASurfaceObject;
};

struct PtrTranpoline {
    explicit PtrTranpoline(void* ptr) : ptr(ptr)
    {
    }

    template<typename T>
    operator T*()
    {
        return reinterpret_cast<T*>(ptr);
    }

   private:
    void* ptr;
};

class ICUDALinearBuffer : public nvrhi::IResource {
   public:
    [[nodiscard]] virtual const CUDALinearBufferDesc& getDesc() const = 0;
    virtual PtrTranpoline GetGPUAddress() const = 0;
};

using cudaSurfaceObject_t = unsigned long long;

class ICUDASurfaceObject : public nvrhi::IResource {
   public:
    [[nodiscard]] virtual const CUDASurfaceObjectDesc& getDesc() const = 0;
    virtual cudaSurfaceObject_t GetSurfaceObject() const = 0;
};

using CUDALinearBufferHandle = nvrhi::RefCountPtr<ICUDALinearBuffer>;
using CUDASurfaceObjectHandle = nvrhi::RefCountPtr<ICUDASurfaceObject>;

class OptiXModuleDesc {
   public:
    OptixModuleCompileOptions module_compile_options;
    OptixPipelineCompileOptions pipeline_compile_options;
    OptixBuiltinISOptions builtinISOptions;

    std::string ptx;
};

class OptiXPipelineDesc {
   public:
    OptixPipelineCompileOptions pipeline_compile_options;
    OptixPipelineLinkOptions pipeline_link_options;
};

class OptiXProgramGroupDesc {
   public:
    OptixProgramGroupOptions program_group_options = {};
    OptixProgramGroupDesc prog_group_desc;
};

class IOptiXProgramGroup : public IResource {
   public:
    [[nodiscard]] virtual const OptiXProgramGroupDesc& getDesc() const = 0;

    virtual OptixProgramGroup getProgramGroup() const = 0;
};

class IOptiXModule : public IResource {
   public:
    [[nodiscard]] virtual const OptiXModuleDesc& getDesc() const = 0;
    virtual OptixModule getModule() const = 0;
};

class IOptiXPipeline : public IResource {
   public:
    [[nodiscard]] virtual const OptiXPipelineDesc& getDesc() const = 0;
    virtual OptixPipeline getPipeline() const = 0;
};
using OptiXModuleHandle = RefCountPtr<IOptiXModule>;
using OptiXPipelineHandle = RefCountPtr<IOptiXPipeline>;
using OptiXProgramGroupHandle = RefCountPtr<IOptiXProgramGroup>;

class OptiXTraversableDesc;
class IOptiXTraversable : public IResource {
   public:
    [[nodiscard]] virtual const OptiXTraversableDesc& getDesc() const = 0;
    virtual OptixTraversableHandle getOptiXTraversable() const = 0;
};

using OptiXTraversableHandle = RefCountPtr<IOptiXTraversable>;

class OptiXTraversableDesc {
   public:
    OptixBuildInput buildInput = {};
    OptixAccelBuildOptions buildOptions = {};

    std::vector<OptiXTraversableHandle> handles;
};

#endif

namespace detail {
class CUDALinearBuffer : public RefCounter<ICUDALinearBuffer> {
   public:
    CUDALinearBuffer(
        const CUDALinearBufferDesc& in_desc,
        IResource* source_resource,
        IDevice* device);
    ~CUDALinearBuffer() override;

    const CUDALinearBufferDesc& getDesc() const override
    {
        return desc;
    }

    PtrTranpoline GetGPUAddress() const override
    {
        return PtrTranpoline{ data };
    }

   protected:
    const CUDALinearBufferDesc desc;
    void* data;

    ResourceHandle handle;
};

class CUDASurfaceObject : public RefCounter<ICUDASurfaceObject> {
   public:
    CUDASurfaceObject(
        const CUDASurfaceObjectDesc& in_desc,
        IResource* source_resource,
        IDevice* device);
    ~CUDASurfaceObject() override;

    const CUDASurfaceObjectDesc& getDesc() const override
    {
        return desc;
    }

    cudaSurfaceObject_t GetSurfaceObject() const override
    {
        return surface_obejct;
    }

   protected:
    const CUDASurfaceObjectDesc desc;
    cudaSurfaceObject_t surface_obejct;

    ResourceHandle handle;
};

class OptiXModule : public RefCounter<IOptiXModule> {
   public:
    explicit OptiXModule(const OptiXModuleDesc& desc, IDevice* device);

    [[nodiscard]] const OptiXModuleDesc& getDesc() const override
    {
        return desc;
    }

    OptixModule getModule() const override
    {
        return module;
    }

   private:
    OptiXModuleDesc desc;
    OptixModule module;
};

class OptiXProgramGroup : public RefCounter<IOptiXProgramGroup> {
   public:
    explicit OptiXProgramGroup(
        OptiXProgramGroupDesc desc,
        OptiXModuleHandle module,
        IDevice* device);
    explicit OptiXProgramGroup(
        OptiXProgramGroupDesc desc,
        std::tuple<OptiXModuleHandle, OptiXModuleHandle, OptiXModuleHandle>
            modules,
        IDevice* device);

   private:
    [[nodiscard]] const OptiXProgramGroupDesc& getDesc() const override
    {
        return desc;
    }

   public:
    OptixProgramGroup getProgramGroup() const override
    {
        return hitgroup_prog_group;
    }

   private:
    OptiXProgramGroupDesc desc;
    OptixProgramGroup hitgroup_prog_group;
};

class OptiXPipeline : public RefCounter<IOptiXPipeline> {
   public:
    explicit OptiXPipeline(
        OptiXPipelineDesc desc,
        const std::vector<OptiXProgramGroupHandle>& program_groups,
        IDevice* device);

    [[nodiscard]] const OptiXPipelineDesc& getDesc() const override
    {
        return desc;
    }

    OptixPipeline getPipeline() const override
    {
        return pipeline;
    }

   private:
    OptiXPipelineDesc desc;
    OptixPipeline pipeline;
};

class OptiXTraversable : public RefCounter<IOptiXTraversable> {
   public:
    explicit OptiXTraversable(
        const OptiXTraversableDesc& desc,
        IDevice* device);

    ~OptiXTraversable() override;

    OptixTraversableHandle getOptiXTraversable() const override
    {
        return handle;
    }

    [[nodiscard]] const OptiXTraversableDesc& getDesc() const override
    {
        return desc;
    }

   private:
    OptiXTraversableDesc desc;
    OptixTraversableHandle handle = 0;
    CUdeviceptr traversableBuffer;
};

CUDALinearBufferHandle createCUDALinearBuffer(
    const CUDALinearBufferDesc& d,
    IResource* source = nullptr)
{
    auto buffer = new CUDALinearBuffer(d, source, this);
    return CUDALinearBufferHandle::Create(buffer);
}

CUDASurfaceObjectHandle createCUDASurfaceObject(
    const CUDASurfaceObjectDesc& d,
    IResource* source = nullptr)
{
    auto buffer = new CUDASurfaceObject(d, source, this);
    return CUDASurfaceObjectHandle::Create(buffer);
}

OptiXModuleHandle createOptiXModule(const OptiXModuleDesc& d)
{
    OptiXModuleDesc desc = d;
    auto module = new OptiXModule(desc, this);

    return OptiXModuleHandle::Create(module);
}

OptiXPipelineHandle createOptiXPipeline(
    const OptiXPipelineDesc& d,
    std::vector<OptiXProgramGroupHandle> program_groups = {})
{
    OptiXPipelineDesc desc = d;
    auto buffer = new OptiXPipeline(desc, program_groups, this);

    return OptiXPipelineHandle::Create(buffer);
}

OptiXProgramGroupHandle createOptiXProgramGroup(
    const OptiXProgramGroupDesc& d,
    OptiXModuleHandle module)
{
    OptiXProgramGroupDesc desc = d;
    auto buffer = new OptiXProgramGroup(desc, module, this);

    return OptiXProgramGroupHandle::Create(buffer);
}

OptiXProgramGroupHandle createOptiXProgramGroup(
    const OptiXProgramGroupDesc& d,
    std::tuple<OptiXModuleHandle, OptiXModuleHandle, OptiXModuleHandle> modules)
{
    OptiXProgramGroupDesc desc = d;
    auto buffer = new OptiXProgramGroup(desc, modules, this);

    return OptiXProgramGroupHandle::Create(buffer);
}

OptiXTraversableHandle createOptiXTraversable(const OptiXTraversableDesc& d)
{
    auto buffer = new OptiXTraversable(d, this);

    return OptiXTraversableHandle::Create(buffer);
}

[[nodiscard]] OptixDeviceContext OptixContext()
{
    OptixPrepare();
    return optixContext;
}

namespace nvrhi {
    OptiXModule::OptiXModule(const OptiXModuleDesc& desc, IDevice* device)
        : desc(desc)
    {
        size_t sizeof_log = sizeof(optix_log);

        if (!desc.ptx.empty()) {
            OPTIX_CHECK_LOG(optixModuleCreate(
                device->OptixContext(),
                &desc.module_compile_options,
                &desc.pipeline_compile_options,
                desc.ptx.c_str(),
                desc.ptx.size(),
                optix_log,
                &sizeof_log,
                &module));
        }
        else {
            OPTIX_CHECK(optixBuiltinISModuleGet(
                device->OptixContext(),
                &desc.module_compile_options,
                &desc.pipeline_compile_options,
                &desc.builtinISOptions,
                &module));
        }
    }

    OptiXProgramGroup::OptiXProgramGroup(
        OptiXProgramGroupDesc desc,
        OptiXModuleHandle module,
        IDevice* device)
        : desc(desc)
    {
        desc.prog_group_desc.raygen.module = module->getModule();

        if (desc.prog_group_desc.kind == OPTIX_PROGRAM_GROUP_KIND_CALLABLES) {
            desc.prog_group_desc.callables.moduleDC = nullptr;
            desc.prog_group_desc.callables.entryFunctionNameDC = nullptr;
            desc.prog_group_desc.callables.moduleCC = module->getModule();
        }

        size_t sizeof_log = sizeof(optix_log);
        OPTIX_CHECK_LOG(optixProgramGroupCreate(
            device->OptixContext(),
            &desc.prog_group_desc,
            1,  // num program groups
            &desc.program_group_options,
            optix_log,
            &sizeof_log,
            &hitgroup_prog_group));
    }

    OptiXProgramGroup::OptiXProgramGroup(
        OptiXProgramGroupDesc desc,
        std::tuple<OptiXModuleHandle, OptiXModuleHandle, OptiXModuleHandle>
            modules,
        IDevice* device)
        : desc(desc)
    {
        assert(desc.prog_group_desc.kind == OPTIX_PROGRAM_GROUP_KIND_HITGROUP);

        if (std::get<0>(modules))
            desc.prog_group_desc.hitgroup.moduleIS =
                std::get<0>(modules)->getModule();
        else
            desc.prog_group_desc.hitgroup.moduleIS = nullptr;

        if (std::get<1>(modules))
            desc.prog_group_desc.hitgroup.moduleAH =
                std::get<1>(modules)->getModule();
        else
            desc.prog_group_desc.hitgroup.moduleAH = nullptr;

        if (std::get<2>(modules))
            desc.prog_group_desc.hitgroup.moduleCH =
                std::get<2>(modules)->getModule();
        else
            throw std::runtime_error("A Closest hit shader must be specified.");

        size_t sizeof_log = sizeof(optix_log);
        OPTIX_CHECK_LOG(optixProgramGroupCreate(
            device->OptixContext(),
            &desc.prog_group_desc,
            1,  // num program groups
            &desc.program_group_options,
            optix_log,
            &sizeof_log,
            &hitgroup_prog_group));
    }

    OptiXPipeline::OptiXPipeline(
        OptiXPipelineDesc desc,
        const std::vector<OptiXProgramGroupHandle>& program_groups,
        IDevice* device)
        : desc(desc)
    {
        size_t sizeof_log = sizeof(optix_log);

        std::vector<OptixProgramGroup> concrete_program_groups;
        for (int i = 0; i < program_groups.size(); ++i) {
            concrete_program_groups.push_back(
                program_groups[i]->getProgramGroup());
        }

        OPTIX_CHECK_LOG(optixPipelineCreate(
            device->OptixContext(),

            &desc.pipeline_compile_options,
            &desc.pipeline_link_options,
            concrete_program_groups.data(),
            program_groups.size(),
            optix_log,
            &sizeof_log,
            &pipeline));

        OptixStackSizes stack_sizes = {};

        for (auto& prog_group : concrete_program_groups) {
            OPTIX_CHECK(optixUtilAccumulateStackSizes(
                prog_group, &stack_sizes, pipeline));
        }

        uint32_t direct_callable_stack_size_from_traversal;
        uint32_t direct_callable_stack_size_from_state;
        uint32_t continuation_stack_size;

        const uint32_t max_trace_depth =
            desc.pipeline_link_options.maxTraceDepth;

        OPTIX_CHECK(optixUtilComputeStackSizes(
            &stack_sizes,
            max_trace_depth,
            max_trace_depth,  // maxCCDepth
            0,                // maxDCDEpth
            &direct_callable_stack_size_from_traversal,
            &direct_callable_stack_size_from_state,
            &continuation_stack_size));
        OPTIX_CHECK(optixPipelineSetStackSize(
            pipeline,
            direct_callable_stack_size_from_traversal,
            direct_callable_stack_size_from_state,
            continuation_stack_size,
            max_trace_depth  // maxTraversableDepth
            ));
    }

    OptiXTraversable::OptiXTraversable(
        const OptiXTraversableDesc& desc,
        IDevice* device)
        : desc(desc)
    {
        OptixAccelBufferSizes gas_buffer_sizes;
        OPTIX_CHECK(optixAccelComputeMemoryUsage(
            device->OptixContext(),
            &desc.buildOptions,
            &desc.buildInput,
            1,
            &gas_buffer_sizes));

        CUdeviceptr deviceTempBufferGAS;

        CUDA_CHECK(cudaMalloc(
            reinterpret_cast<void**>(&deviceTempBufferGAS),
            gas_buffer_sizes.tempSizeInBytes));
        CUDA_CHECK(cudaMalloc(
            reinterpret_cast<void**>(&traversableBuffer),
            gas_buffer_sizes.outputSizeInBytes));

        OPTIX_CHECK(optixAccelBuild(
            device->OptixContext(),
            0,
            // CUDA stream
            &desc.buildOptions,
            &desc.buildInput,
            1,
            // num build inputs
            deviceTempBufferGAS,
            gas_buffer_sizes.tempSizeInBytes,
            traversableBuffer,
            gas_buffer_sizes.outputSizeInBytes,
            &handle,
            nullptr,
            // emitted property list
            0  // num emitted properties
            ));
        CUDA_SYNC_CHECK();
        CUDA_CHECK(cudaFree((void*)deviceTempBufferGAS));
        CUDA_SYNC_CHECK();
    }

    OptiXTraversable::~OptiXTraversable()
    {
        cudaFree((void*)traversableBuffer);
    }

    static void context_log_cb(
        unsigned int level,
        const char* tag,
        const char* message,
        void* /*cbdata */)
    {
        std::cerr << "[" << std::setw(2) << level << "][" << std::setw(12)
                  << tag << "]: " << message << "\n";
    }

    CUstream IDevice::optixStream;
    OptixDeviceContext IDevice::optixContext;
    void IDevice::OptixPrepare()
    {
        if (!isOptiXInitalized) {
            // Initialize CUDA
            CUDA_CHECK(cudaFree(0));

            OPTIX_CHECK(optixInit());
            OptixDeviceContextOptions options = {};
            options.logCallbackFunction = &context_log_cb;
            options.logCallbackLevel = 4;
            OPTIX_CHECK(optixDeviceContextCreate(0, &options, &optixContext));
            CUDA_CHECK(cudaStreamCreate(&optixStream));
        }
        isOptiXInitalized = true;
    }

    USTC_CG_NAMESPACE_CLOSE_SCOPE