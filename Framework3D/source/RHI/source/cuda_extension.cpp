#if USTC_CG_WITH_CUDA
#include <cuda_runtime.h>

#include <RHI/internal/cuda_extension.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "RHI/internal/optix/optix.h"
#include "RHI/internal/optix/optix_function_table_definition.h"
#include "RHI/internal/optix/optix_stack_size.h"
#include "RHI/internal/optix/optix_stubs.h"
#include "cuda_extension_utils.h"
#include "nvrhi/nvrhi.h"
#include "nvrtc_config.h"

#define STRINGIFY(x)  STRINGIFY2(x)
#define STRINGIFY2(x) #x
#define LINE_STR      STRINGIFY(__LINE__)

#include <nvrtc.h>
// Error check/report helper for users of the C API
#define NVRTC_CHECK_ERROR(func)                          \
    do {                                                 \
        nvrtcResult code = func;                         \
        if (code != NVRTC_SUCCESS)                       \
            throw std::runtime_error(                    \
                "ERROR: " __FILE__ "(" LINE_STR "): " +  \
                std::string(nvrtcGetErrorString(code))); \
    } while (0)

USTC_CG_NAMESPACE_OPEN_SCOPE

static CUstream optixStream;
static OptixDeviceContext optixContext;
static bool isOptiXInitalized = false;
char optix_log[2048];

static void context_log_cb(
    unsigned int level,
    const char* tag,
    const char* message,
    void* /*cbdata */)
{
    std::cerr << "[" << std::setw(2) << level << "][" << std::setw(12) << tag
              << "]: " << message << "\n";
}

//////////////////////////////////////////////////////////////////////////
// CUDA and OptiX
//////////////////////////////////////////////////////////////////////////
namespace cuda {

static bool readSourceFile(std::string& str, const std::string& filename)
{
    // Try to open file
    std::ifstream file(filename.c_str());
    if (file.good()) {
        // Found usable source file
        std::stringstream source_buffer;
        source_buffer << file.rdbuf();
        str = source_buffer.str();
        return true;
    }
    return false;
}

static void getCuStringFromFile(
    std::string& cu,
    std::string& location,
    const char* sampleDir,
    const char* filename)
{
    std::vector<std::string> source_locations;

    const std::string base_dir = OptiX_DIR;

    // Potential source locations (in priority order)
    if (sampleDir)
        source_locations.push_back(base_dir + '/' + sampleDir + '/' + filename);
    source_locations.push_back(
        base_dir + "../../source/renderer/nodes/shaders/shaders/" + filename);
    source_locations.push_back(base_dir + "test/src/OptiX/" + filename);

    for (const std::string& loc : source_locations) {
        // Try to get source code from file
        if (readSourceFile(cu, loc)) {
            location = loc;
            return;
        }
    }

    // Wasn't able to find or open the requested file
    throw std::runtime_error(
        "Couldn't open source file " + std::string(filename));
}

static std::string g_nvrtcLog;

static void getPtxFromCuString(
    std::string& ptx,
    const char* sample_name,
    const char* cu_source,
    const char* name,
    const char** log_string)
{
    // Create program
    nvrtcProgram prog = 0;
    NVRTC_CHECK_ERROR(
        nvrtcCreateProgram(&prog, cu_source, name, 0, NULL, NULL));

    // Gather NVRTC options
    std::vector<const char*> options;

    const std::string base_dir = OptiX_DIR;

    // Set sample dir as the primary include path
    std::string sample_dir;
    if (sample_name) {
        sample_dir = std::string("-I") + base_dir + '/' + sample_name;
        options.push_back(sample_dir.c_str());
    }

    // Collect include dirs
    std::vector<std::string> include_dirs;
    const char* abs_dirs[] = { OptiX_ABSOLUTE_INCLUDE_DIRS };
    const char* rel_dirs[] = { OptiX_RELATIVE_INCLUDE_DIRS };

    for (const char* dir : abs_dirs) {
        include_dirs.push_back(std::string("-I") + dir);
    }
    for (const char* dir : rel_dirs) {
        include_dirs.push_back("-I" + base_dir + '/' + dir);
    }
    for (const std::string& dir : include_dirs) {
        options.push_back(dir.c_str());
    }

    // Collect NVRTC options
    const char* compiler_options[] = { CUDA_NVRTC_OPTIONS };
    std::copy(
        std::begin(compiler_options),
        std::end(compiler_options),
        std::back_inserter(options));

    // JIT compile CU to PTX
    const nvrtcResult compileRes =
        nvrtcCompileProgram(prog, (int)options.size(), options.data());

    // Retrieve log output
    size_t log_size = 0;
    NVRTC_CHECK_ERROR(nvrtcGetProgramLogSize(prog, &log_size));
    g_nvrtcLog.resize(log_size);
    if (log_size > 1) {
        NVRTC_CHECK_ERROR(nvrtcGetProgramLog(prog, &g_nvrtcLog[0]));
        if (log_string)
            *log_string = g_nvrtcLog.c_str();
    }
    if (compileRes != NVRTC_SUCCESS)
        throw std::runtime_error("NVRTC Compilation failed.\n" + g_nvrtcLog);

    // Retrieve PTX code
    size_t ptx_size = 0;
    NVRTC_CHECK_ERROR(nvrtcGetPTXSize(prog, &ptx_size));
    ptx.resize(ptx_size);
    NVRTC_CHECK_ERROR(nvrtcGetPTX(prog, &ptx[0]));

    // Cleanup
    NVRTC_CHECK_ERROR(nvrtcDestroyProgram(&prog));
}

struct PtxSourceCache {
    std::map<std::string, std::string*> map;
    ~PtxSourceCache()
    {
        for (std::map<std::string, std::string*>::const_iterator it =
                 map.begin();
             it != map.end();
             ++it)
            delete it->second;
    }
};
static PtxSourceCache g_ptxSourceCache;

const char* get_ptx_string_from_cu(const char* filename, const char** log)
{
    if (log)
        *log = NULL;

    std::string *ptx, cu;
    std::string key = std::string(filename) + ";";
    std::map<std::string, std::string*>::iterator elem =
        g_ptxSourceCache.map.find(key);

    if (elem == g_ptxSourceCache.map.end()) {
        ptx = new std::string();
        std::string location;
        getCuStringFromFile(cu, location, "", filename);
        getPtxFromCuString(*ptx, "", cu.c_str(), location.c_str(), log);
        g_ptxSourceCache.map[key] = ptx;
    }
    else {
        ptx = elem->second;
    }

    return ptx->c_str();
}

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

using cudaSurfaceObject_t = unsigned long long;

class ICUDASurfaceObject : public nvrhi::IResource {
   public:
    virtual ~ICUDASurfaceObject() = default;
    [[nodiscard]] virtual const CUDASurfaceObjectDesc& getDesc() const = 0;
    virtual cudaSurfaceObject_t GetSurfaceObject() const = 0;
};

using CUDASurfaceObjectHandle = nvrhi::RefCountPtr<ICUDASurfaceObject>;

using nvrhi::RefCountPtr;

using nvrhi::RefCounter;

class CUDASurfaceObject : public RefCounter<ICUDASurfaceObject> {
   public:
    CUDASurfaceObject(const CUDASurfaceObjectDesc& in_desc);
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
};

class OptiXModule : public RefCounter<IOptiXModule> {
   public:
    explicit OptiXModule(const OptiXModuleDesc& desc);

    [[nodiscard]] const OptiXModuleDesc& getDesc() const override
    {
        return desc;
    }

   protected:
    OptixModule getModule() const override
    {
        return module;
    }

   private:
    OptiXModuleDesc desc;
    OptixModule module;
    std::string ptx;
};

class OptiXProgramGroup : public RefCounter<IOptiXProgramGroup> {
   public:
    explicit OptiXProgramGroup(
        OptiXProgramGroupDesc desc,
        OptiXModuleHandle module);
    explicit OptiXProgramGroup(
        OptiXProgramGroupDesc desc,
        std::tuple<OptiXModuleHandle, OptiXModuleHandle, OptiXModuleHandle>
            modules);

   private:
    [[nodiscard]] const OptiXProgramGroupDesc& getDesc() const override
    {
        return desc;
    }

   private:
    OptiXProgramGroupDesc desc;
    OptixProgramGroup hitgroup_prog_group;

   protected:
    OptixProgramGroup getProgramGroup() const override
    {
        return hitgroup_prog_group;
    }
};

class OptiXPipeline : public RefCounter<IOptiXPipeline> {
   public:
    explicit OptiXPipeline(
        OptiXPipelineDesc desc,
        const std::vector<OptiXProgramGroupHandle>& program_groups);

    [[nodiscard]] const OptiXPipelineDesc& getDesc() const override
    {
        return desc;
    }

   protected:
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
    explicit OptiXTraversable(const OptiXTraversableDesc& desc);

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

CUDASurfaceObjectHandle createCUDASurfaceObject(const CUDASurfaceObjectDesc& d)
{
    auto buffer = new CUDASurfaceObject(d);
    return CUDASurfaceObjectHandle::Create(buffer);
}

[[nodiscard]] OptixDeviceContext OptixContext()
{
    optix_init();
    return optixContext;
}

OptiXModule::OptiXModule(const OptiXModuleDesc& desc) : desc(desc)
{
    size_t sizeof_log = sizeof(optix_log);

    if (!desc.file_name.empty()) {
        if (ptx.empty()) {
            ptx = get_ptx_string_from_cu(desc.file_name.c_str());
        }

        OPTIX_CHECK_LOG(optixModuleCreate(
            OptixContext(),
            &desc.module_compile_options,
            &desc.pipeline_compile_options,
            ptx.c_str(),
            ptx.size(),
            optix_log,
            &sizeof_log,
            &module));
    }
    else {
        OPTIX_CHECK(optixBuiltinISModuleGet(
            OptixContext(),
            &desc.module_compile_options,
            &desc.pipeline_compile_options,
            &desc.builtinISOptions,
            &module));
    }
}

OptiXProgramGroupDesc& OptiXProgramGroupDesc::set_program_group_kind(
    OptixProgramGroupKind kind)
{
    prog_group_desc.kind = kind;
    return *this;
}

OptiXProgramGroupDesc& OptiXProgramGroupDesc::set_entry_name(const char* name)
{
    prog_group_desc.raygen.entryFunctionName = name;
    return *this;
}

OptiXProgramGroupDesc& OptiXProgramGroupDesc::set_entry_name(
    const char* is,
    const char* ahs,
    const char* chs)
{
    prog_group_desc.hitgroup.entryFunctionNameIS = is;
    prog_group_desc.hitgroup.entryFunctionNameAH = ahs;
    prog_group_desc.hitgroup.entryFunctionNameCH = chs;
    return *this;
}

OptiXProgramGroup::OptiXProgramGroup(
    OptiXProgramGroupDesc desc,
    OptiXModuleHandle module)
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
        OptixContext(),
        &desc.prog_group_desc,
        1,  // num program groups
        &desc.program_group_options,
        optix_log,
        &sizeof_log,
        &hitgroup_prog_group));
}

OptiXProgramGroup::OptiXProgramGroup(
    OptiXProgramGroupDesc desc,
    std::tuple<OptiXModuleHandle, OptiXModuleHandle, OptiXModuleHandle> modules)
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
        OptixContext(),
        &desc.prog_group_desc,
        1,  // num program groups
        &desc.program_group_options,
        optix_log,
        &sizeof_log,
        &hitgroup_prog_group));
}

OptiXPipeline::OptiXPipeline(
    OptiXPipelineDesc desc,
    const std::vector<OptiXProgramGroupHandle>& program_groups)
    : desc(desc)
{
    size_t sizeof_log = sizeof(optix_log);

    std::vector<OptixProgramGroup> concrete_program_groups;
    for (int i = 0; i < program_groups.size(); ++i) {
        concrete_program_groups.push_back(program_groups[i]->getProgramGroup());
    }

    OPTIX_CHECK_LOG(optixPipelineCreate(
        OptixContext(),

        &desc.pipeline_compile_options,
        &desc.pipeline_link_options,
        concrete_program_groups.data(),
        program_groups.size(),
        optix_log,
        &sizeof_log,
        &pipeline));

    OptixStackSizes stack_sizes = {};

    for (auto& prog_group : concrete_program_groups) {
        OPTIX_CHECK(
            optixUtilAccumulateStackSizes(prog_group, &stack_sizes, pipeline));
    }

    uint32_t direct_callable_stack_size_from_traversal;
    uint32_t direct_callable_stack_size_from_state;
    uint32_t continuation_stack_size;

    const uint32_t max_trace_depth = desc.pipeline_link_options.maxTraceDepth;

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

OptiXTraversable::OptiXTraversable(const OptiXTraversableDesc& desc)
    : desc(desc)
{
    OptixAccelBufferSizes gas_buffer_sizes;
    OPTIX_CHECK(optixAccelComputeMemoryUsage(
        OptixContext(),
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
        OptixContext(),
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

// Here the resourcetype could be texture or buffer now.
template<typename ResourceType>
HANDLE getSharedApiHandle(nvrhi::IDevice* device, ResourceType* texture_handle)
{
    return texture_handle->getNativeObject(nvrhi::ObjectTypes::SharedHandle);
}

CUDASurfaceObject::CUDASurfaceObject(const CUDASurfaceObjectDesc& in_desc)
    : desc(in_desc)
{
    throw std::runtime_error("Not implemented yet");
}

CUDASurfaceObject::~CUDASurfaceObject()
{
    cudaDestroySurfaceObject(surface_obejct);
}

int cuda_init()
{
    return 0;
}

int optix_init()
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

    return 0;
}

int cuda_shutdown()
{
    return 0;
}

OptiXTraversableHandle create_optix_traversable(const OptiXTraversableDesc& d)
{
    auto buffer = new OptiXTraversable(d);

    return OptiXTraversableHandle::Create(buffer);
}

OptiXTraversableHandle create_optix_traversable(
    std::vector<CUdeviceptr> vertexBuffer,
    unsigned int numVertices,
    std::vector<CUdeviceptr> widthBuffer,
    CUdeviceptr indexBuffer,
    unsigned int numPrimitives,
    bool rebuilding)
{
    OptiXTraversableDesc desc;

    desc.buildInput.type = OPTIX_BUILD_INPUT_TYPE_CURVES;

    OptixBuildInputCurveArray& curveArray = desc.buildInput.curveArray;
    curveArray.curveType = OPTIX_PRIMITIVE_TYPE_ROUND_LINEAR;

    curveArray.numPrimitives = numPrimitives;

    curveArray.vertexBuffers = vertexBuffer.data();
    curveArray.numVertices = numVertices;
    curveArray.vertexStrideInBytes = sizeof(float3);
    curveArray.widthBuffers = widthBuffer.data();
    curveArray.widthStrideInBytes = sizeof(float);
    curveArray.indexBuffer = indexBuffer;
    curveArray.indexStrideInBytes = sizeof(unsigned int);
    curveArray.flag = OPTIX_GEOMETRY_FLAG_NONE;
    curveArray.primitiveIndexOffset = 0;
    curveArray.endcapFlags = OPTIX_CURVE_ENDCAP_DEFAULT;

    desc.buildOptions.buildFlags =
        OPTIX_BUILD_FLAG_ALLOW_COMPACTION | OPTIX_BUILD_FLAG_ALLOW_UPDATE;
    desc.buildOptions.motionOptions.numKeys = 1;

    if (rebuilding)
        desc.buildOptions.operation = OPTIX_BUILD_OPERATION_UPDATE;
    else
        desc.buildOptions.operation = OPTIX_BUILD_OPERATION_BUILD;

    return create_optix_traversable(desc);
}

OptiXProgramGroupHandle create_optix_program_group(
    const OptiXProgramGroupDesc& d,
    std::tuple<OptiXModuleHandle, OptiXModuleHandle, OptiXModuleHandle> modules)
{
    OptiXProgramGroupDesc desc = d;
    auto buffer = new OptiXProgramGroup(desc, modules);

    return OptiXProgramGroupHandle::Create(buffer);
}

OptiXProgramGroupHandle create_optix_raygen(
    const std::string& file_path,
    const char* entry_name)
{
    auto module = create_optix_module(file_path);

    OptiXProgramGroupDesc desc;
    desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_RAYGEN)
        .set_entry_name(entry_name);

    return create_optix_program_group(desc, module);
}

OptiXProgramGroupHandle create_optix_miss(
    const std::string& file_path,
    const char* entry_name)
{
    auto module = create_optix_module(file_path);

    OptiXProgramGroupDesc desc;
    desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_MISS)
        .set_entry_name(entry_name);

    return create_optix_program_group(desc, module);
}

OptiXModuleHandle create_optix_module(const OptiXModuleDesc& d)
{
    OptiXModuleDesc desc = d;
    auto module = new OptiXModule(desc);

    return OptiXModuleHandle::Create(module);
}

OptixModuleCompileOptions get_default_module_compile_options()
{
    OptixModuleCompileOptions module_compile_options = {};
    module_compile_options.maxRegisterCount = 128;
    module_compile_options.optLevel = OPTIX_COMPILE_OPTIMIZATION_DEFAULT;
    module_compile_options.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_NONE;
    module_compile_options.numBoundValues = 0;
    module_compile_options.boundValues = nullptr;

    return module_compile_options;
}

OptixPipelineCompileOptions get_default_pipeline_compile_options()
{
    OptixPipelineCompileOptions pipeline_compile_options = {};
    pipeline_compile_options.traversableGraphFlags =
        OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_ANY;
    pipeline_compile_options.usesMotionBlur = false;
    pipeline_compile_options.numPayloadValues = 2;
    pipeline_compile_options.numAttributeValues = 2;
    pipeline_compile_options.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
    pipeline_compile_options.pipelineLaunchParamsVariableName = "params";
    pipeline_compile_options.usesPrimitiveTypeFlags = 0;
    pipeline_compile_options.usesPrimitiveTypeFlags |=
        OPTIX_PRIMITIVE_TYPE_FLAGS_TRIANGLE;
    pipeline_compile_options.usesPrimitiveTypeFlags |=
        OPTIX_PRIMITIVE_TYPE_FLAGS_CUSTOM;
    pipeline_compile_options.usesPrimitiveTypeFlags |=
        OPTIX_PRIMITIVE_TYPE_FLAGS_ROUND_LINEAR;
    return pipeline_compile_options;
}

OptixPipelineLinkOptions get_default_pipeline_link_options()
{
    OptixPipelineLinkOptions pipeline_link_options = {};
    pipeline_link_options.maxTraceDepth = 2;
    return pipeline_link_options;
}

OptixBuiltinISOptions get_default_built_in_is_options()
{
    OptixBuiltinISOptions options = {};
    options.usesMotionBlur = false;
    options.buildFlags = OPTIX_BUILD_FLAG_NONE;
    options.curveEndcapFlags = OPTIX_CURVE_ENDCAP_DEFAULT;
    return options;
}

OptiXModuleHandle create_optix_module(const std::string& file_path)
{
    OptiXModuleDesc desc;
    desc.file_name = file_path;
    desc.module_compile_options = get_default_module_compile_options();
    desc.pipeline_compile_options = get_default_pipeline_compile_options();
    return create_optix_module(desc);
}

OptiXModuleHandle get_builtin_module(OptixPrimitiveType type)
{
    OptiXModuleDesc desc;
    desc.module_compile_options = get_default_module_compile_options();
    desc.pipeline_compile_options = get_default_pipeline_compile_options();
    OptixBuiltinISOptions& options = desc.builtinISOptions;
    options = get_default_built_in_is_options();
    options.builtinISModuleType = type;

    return create_optix_module(desc);
}

OptiXPipelineHandle create_optix_pipeline(
    const OptiXPipelineDesc& d,
    std::vector<OptiXProgramGroupHandle> program_groups)
{
    OptiXPipelineDesc desc = d;
    auto buffer = new OptiXPipeline(desc, program_groups);

    return OptiXPipelineHandle::Create(buffer);
}

OptiXPipelineHandle create_optix_pipeline(
    std::vector<OptiXProgramGroupHandle> program_groups)
{
    OptiXPipelineDesc desc;
    desc.pipeline_compile_options = get_default_pipeline_compile_options();
    desc.pipeline_link_options = get_default_pipeline_link_options();
    return create_optix_pipeline(desc, program_groups);
}

OptiXProgramGroupHandle create_optix_program_group(
    const OptiXProgramGroupDesc& d,
    OptiXModuleHandle module)
{
    OptiXProgramGroupDesc desc = d;
    auto buffer = new OptiXProgramGroup(desc, module);

    return OptiXProgramGroupHandle::Create(buffer);
}
}  // namespace cuda

//
// void FetchD3DMemory(
//    nvrhi::IResource* resource_handle,
//    nvrhi::IDevice* device,
//    size_t& actualSize,
//    HANDLE sharedHandle,
//    cudaExternalMemoryHandleDesc& externalMemoryHandleDesc)
//{
// #ifdef _WIN64
//    ID3D12Resource* resource =
//        resource_handle->getNativeObject(nvrhi::ObjectTypes::D3D12_Resource);
//    ID3D12Device* native_device =
//        getNativeObject(nvrhi::ObjectTypes::D3D12_Device);
//
//    D3D12_RESOURCE_ALLOCATION_INFO d3d12ResourceAllocationInfo;
//
//    D3D12_RESOURCE_DESC texture_desc = resource->GetDesc();
//
//    d3d12ResourceAllocationInfo =
//        native_GetResourceAllocationInfo(0, 1, &texture_desc);
//    actualSize = d3d12ResourceAllocationInfo.SizeInBytes;
//
//    externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeD3D12Resource;
//    externalMemoryHandleDesc.handle.win32.handle = sharedHandle;
//    externalMemoryHandleDesc.size = actualSize;
//    externalMemoryHandleDesc.flags = cudaExternalMemoryDedicated;
// #else
//    throw std::runtime_error("D3D12 in Windows only.");
// #endif
//}
//
// void FetchVulkanMemory(
//    size_t& actualSize,
//    HANDLE sharedHandle,
//    cudaExternalMemoryHandleDesc& externalMemoryHandleDesc,
//    vk::MemoryRequirements vkMemoryRequirements)
//{
//    actualSize = vkMemoryRequirements.size;
// #ifdef _WIN64
//    externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
//    externalMemoryHandleDesc.handle.win32.handle = sharedHandle;
// #else
//    externalMemoryHandleDesc.type = cudaExternalMemoryHandleTypeOpaqueFd;
//    externalMemoryHandleDesc.handle.fd = sharedHandle;
// #endif
//    externalMemoryHandleDesc.size = actualSize;
//    externalMemoryHandleDesc.flags = 0;
//}
//
// cudaExternalMemory_t FetchExternalTextureMemory(
//    nvrhi::ITexture* image_handle,
//    nvrhi::IDevice* device,
//    size_t& actualSize,
//    HANDLE sharedHandle)
//{
//    cudaExternalMemoryHandleDesc externalMemoryHandleDesc;
//    memset(&externalMemoryHandleDesc, 0, sizeof(externalMemoryHandleDesc));
//
//    if (getGraphicsAPI() == nvrhi::GraphicsAPI::D3D12) {
//        FetchD3DMemory(
//            image_handle,
//            device,
//            actualSize,
//            sharedHandle,
//            externalMemoryHandleDesc);
//    }
//    else if (getGraphicsAPI() == nvrhi::GraphicsAPI::VULKAN) {
//        vk::Device native_device =
//            VkDevice(getNativeObject(nvrhi::ObjectTypes::VK_Device));
//        VkImage image =
//            image_handle->getNativeObject(nvrhi::ObjectTypes::VK_Image);
//
//        vk::MemoryRequirements vkMemoryRequirements = {};
//        native_device.getImageMemoryRequirements(image,
//        &vkMemoryRequirements);
//
//        FetchVulkanMemory(
//            actualSize,
//            sharedHandle,
//            externalMemoryHandleDesc,
//            vkMemoryRequirements);
//    }
//
//    cudaExternalMemory_t externalMemory;
//    CUDA_CHECK(
//        cudaImportExternalMemory(&externalMemory, &externalMemoryHandleDesc));
//    return externalMemory;
//}
//
// cudaExternalMemory_t FetchExternalBufferMemory(
//    nvrhi::IBuffer* buffer_handle,
//    nvrhi::IDevice* device,
//    size_t& actualSize,
//    HANDLE sharedHandle)
//{
//    cudaExternalMemoryHandleDesc externalMemoryHandleDesc;
//    memset(&externalMemoryHandleDesc, 0, sizeof(externalMemoryHandleDesc));
//
//    if (getGraphicsAPI() == nvrhi::GraphicsAPI::D3D12) {
//        FetchD3DMemory(
//            buffer_handle,
//            device,
//            actualSize,
//            sharedHandle,
//            externalMemoryHandleDesc);
//    }
//    else if (getGraphicsAPI() == nvrhi::GraphicsAPI::VULKAN) {
//        vk::Device native_device =
//            VkDevice(getNativeObject(nvrhi::ObjectTypes::VK_Device));
//        VkBuffer buffer =
//            buffer_handle->getNativeObject(nvrhi::ObjectTypes::VK_Buffer);
//
//        vk::MemoryRequirements vkMemoryRequirements = {};
//        native_device.getBufferMemoryRequirements(
//            buffer, &vkMemoryRequirements);
//        FetchVulkanMemory(
//            actualSize,
//            sharedHandle,
//            externalMemoryHandleDesc,
//            vkMemoryRequirements);
//    }
//
//    cudaExternalMemory_t externalMemory;
//    CUDA_CHECK(
//        cudaImportExternalMemory(&externalMemory, &externalMemoryHandleDesc));
//    return externalMemory;
//}
//
// bool importBufferToBuffer(
//    nvrhi::IBuffer* buffer_handle,
//    void*& devPtr,
//    uint32_t cudaUsageFlags,
//    nvrhi::IDevice* device)
//{
//    HANDLE sharedHandle = getSharedApiHandle(device, buffer_handle);
//    if (sharedHandle == NULL) {
//        throw std::runtime_error(
//            "FalcorCUDA::importTextureToMipmappedArray - texture shared handle
//            " "creation failed");
//        return false;
//    }
//
//    size_t actualSize;
//
//    cudaExternalMemory_t externalMemory = FetchExternalBufferMemory(
//        buffer_handle, device, actualSize, sharedHandle);
//
//    cudaExternalMemoryBufferDesc externalMemBufferDesc;
//    memset(&externalMemBufferDesc, 0, sizeof(externalMemBufferDesc));
//
//    externalMemBufferDesc.offset = 0;
//    externalMemBufferDesc.size = actualSize;
//    externalMemBufferDesc.flags = cudaUsageFlags;
//    CUDA_SYNC_CHECK();
//    CUDA_CHECK(cudaExternalMemoryGetMappedBuffer(
//        &devPtr, externalMemory, &externalMemBufferDesc));
//
//    return true;
//}
//
// bool importTextureToBuffer(
//    nvrhi::ITexture* image_handle,
//    void*& devPtr,
//    uint32_t cudaUsageFlags,
//    nvrhi::IDevice* device)
//{
//    HANDLE sharedHandle = getSharedApiHandle(device, image_handle);
//    if (sharedHandle == NULL) {
//        throw std::runtime_error(
//            "FalcorCUDA::importTextureToMipmappedArray - texture shared handle
//            " "creation failed");
//        return false;
//    }
//
//    size_t actualSize;
//
//    cudaExternalMemory_t externalMemory = FetchExternalTextureMemory(
//        image_handle, device, actualSize, sharedHandle);
//
//    cudaExternalMemoryBufferDesc externalMemBufferDesc;
//    memset(&externalMemBufferDesc, 0, sizeof(externalMemBufferDesc));
//
//    externalMemBufferDesc.offset = 0;
//    externalMemBufferDesc.size = actualSize;
//    externalMemBufferDesc.flags = cudaUsageFlags;
//
//    CUDA_CHECK(cudaExternalMemoryGetMappedBuffer(
//        &devPtr, externalMemory, &externalMemBufferDesc));
//
//    return true;
//}
//
// bool importTextureToMipmappedArray(
//    nvrhi::ITexture* image_handle,
//    cudaMipmappedArray_t& mipmappedArray,
//    uint32_t cudaUsageFlags,
//    nvrhi::IDevice* device)
//{
//    HANDLE sharedHandle = getSharedApiHandle(device, image_handle);
//    if (sharedHandle == NULL) {
//        throw std::runtime_error(
//            "FalcorCUDA::importTextureToMipmappedArray - texture shared handle
//            " "creation failed");
//        return false;
//    }
//
//    size_t actualSize;
//
//    cudaExternalMemory_t externalMemory = FetchExternalTextureMemory(
//        image_handle, device, actualSize, sharedHandle);
//
//    cudaExternalMemoryMipmappedArrayDesc mipDesc;
//    memset(&mipDesc, 0, sizeof(mipDesc));
//
//    nvrhi::Format format = image_handle->getDesc().format;
//    mipDesc.formatDesc.x = formatBitsInfo[format].redBits;
//    mipDesc.formatDesc.y = formatBitsInfo[format].greenBits;
//    mipDesc.formatDesc.z = formatBitsInfo[format].blueBits;
//    mipDesc.formatDesc.w = formatBitsInfo[format].alphaBits;
//    mipDesc.formatDesc.f =
//        (nvrhi::getFormatInfo(format).kind == nvrhi::FormatKind::Float)
//            ? cudaChannelFormatKindFloat
//            : cudaChannelFormatKindUnsigned;
//
//    mipDesc.extent.depth = 0;
//    mipDesc.extent.width = image_handle->getDesc().width;
//    mipDesc.extent.height = image_handle->getDesc().height;
//    mipDesc.flags = cudaUsageFlags;
//    mipDesc.numLevels = 1;
//    mipDesc.offset = 0;
//
//    CUDA_CHECK(cudaExternalMemoryGetMappedMipmappedArray(
//        &mipmappedArray, externalMemory, &mipDesc));
//
//    // CloseHandle(sharedHandle);
//    return true;
//}
//
// CUsurfObject mapTextureToSurface(
//    nvrhi::ITexture* image_handle,
//    uint32_t cudaUsageFlags,
//    nvrhi::IDevice* device)
//{
//    // Create a mipmapped array from the texture
//    cudaMipmappedArray_t mipmap;
//
//    if (!importTextureToMipmappedArray(
//            image_handle, mipmap, cudaUsageFlags, device)) {
//        throw std::runtime_error(
//            "Failed to import texture into a mipmapped array");
//        return 0;
//    }
//
//    // Grab level 0
//    cudaArray_t cudaArray;
//    CUDA_CHECK(cudaGetMipmappedArrayLevel(&cudaArray, mipmap, 0));
//
//    // Create cudaSurfObject_t from CUDA array
//    cudaResourceDesc resDesc;
//    memset(&resDesc, 0, sizeof(resDesc));
//    resDesc.res.array.array = cudaArray;
//    resDesc.resType = cudaResourceTypeArray;
//
//    cudaSurfaceObject_t surface;
//    CUDA_CHECK(cudaCreateSurfaceObject(&surface, &resDesc));
//    return surface;
//}
//
// CUtexObject mapTextureToCUDATex(
//    nvrhi::ITexture* image_handle,
//    uint32_t cudaUsageFlags,
//    nvrhi::IDevice* device)
//{
//    // Create a mipmapped array from the texture
//    cudaMipmappedArray_t mipmap;
//
//    if (!importTextureToMipmappedArray(
//            image_handle, mipmap, cudaUsageFlags, device)) {
//        throw std::runtime_error(
//            "Failed to import texture into a mipmapped array");
//        return 0;
//    }
//
//    // Grab level 0
//    cudaArray_t cudaArray;
//    CUDA_CHECK(cudaGetMipmappedArrayLevel(&cudaArray, mipmap, 0));
//
//    // Create cudaSurfObject_t from CUDA array
//    cudaResourceDesc resDesc;
//    memset(&resDesc, 0, sizeof(resDesc));
//    resDesc.res.mipmap.mipmap = mipmap;
//    resDesc.resType = cudaResourceTypeMipmappedArray;
//
//    cudaTextureObject_t texture;
//    auto desc = image_handle->getDesc();
//    auto formatInfo = nvrhi::getFormatInfo(desc.format);
//    auto mipLevels = image_handle->getDesc().mipLevels;
//
//    cudaTextureDesc texDescr;
//    memset(&texDescr, 0, sizeof(cudaTextureDesc));
//    texDescr.normalizedCoords = true;
//    texDescr.filterMode = cudaFilterModeLinear;
//    texDescr.mipmapFilterMode = cudaFilterModeLinear;
//
//    texDescr.addressMode[0] = cudaAddressModeWrap;
//    texDescr.addressMode[1] = cudaAddressModeWrap;
//
//    texDescr.sRGB = formatInfo.isSRGB;
//
//    texDescr.maxMipmapLevelClamp = float(mipLevels - 1);
//    texDescr.readMode = cudaReadModeNormalizedFloat;
//
//    CUDA_CHECK(cudaCreateTextureObject(&texture, &resDesc, &texDescr,
//    nullptr)); return texture;
//}
//
// CUdeviceptr mapTextureToCUDABuffer(
//    nvrhi::ITexture* pTex,
//    uint32_t cudaUsageFlags,
//    nvrhi::IDevice* device)
//{
//    // Create a mipmapped array from the texture
//
//    void* devicePtr;
//    if (!importTextureToBuffer(pTex, devicePtr, cudaUsageFlags, device)) {
//        throw std::runtime_error("Failed to import texture into a buffer");
//    }
//
//    return (CUdeviceptr)devicePtr;
//}
//
// CUdeviceptr mapBufferToCUDABuffer(
//    nvrhi::IBuffer* pBuf,
//    uint32_t cudaUsageFlags,
//    nvrhi::IDevice* device)
//{
//    // Create a mipmapped array from the texture
//
//    void* devicePtr;
//    if (!importBufferToBuffer(pBuf, devicePtr, cudaUsageFlags, device)) {
//        throw std::runtime_error("Failed to import texture into a buffer");
//    }
//
//    return (CUdeviceptr)devicePtr;
//}

USTC_CG_NAMESPACE_CLOSE_SCOPE

#endif