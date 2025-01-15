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

#include "RHI/internal/cuda_extension_utils.h"
#include "RHI/internal/optix/optix.h"
#include "RHI/internal/optix/optix_function_table_definition.h"
#include "RHI/internal/optix/optix_stack_size.h"
#include "RHI/internal/optix/optix_stubs.h"
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
static cudaStream_t optixStream;

cudaStream_t get_optix_stream()
{
    return optixStream;
}

int optix_trace_ray(
    OptiXTraversableHandle traversable,
    OptiXPipelineHandle handle,
    CUdeviceptr launch_params,
    unsigned launch_params_size,
    int x,
    int y,
    int z)
{
    auto sbt = handle->getSbt();
    optixLaunch(
        handle->getPipeline(),
        get_optix_stream(),
        launch_params,
        launch_params_size,
        &sbt,
        x,
        y,
        z);
    CUDA_SYNC_CHECK();
    return 0;
}

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
    source_locations.push_back(base_dir + filename);
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

std::vector<std::string> extra_relative_include_dirs;

void add_extra_relative_include_dir_for_optix(const std::string& dir)
{
    extra_relative_include_dirs.push_back(dir);
}

static bool getPtxFromCuString(
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

    for (const std::string& dir : extra_relative_include_dirs) {
        include_dirs.push_back("-I" + base_dir + dir);
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
    if (compileRes != NVRTC_SUCCESS) {
        std::cout << g_nvrtcLog << std::endl
                  << std::endl
                  << std::endl
                  << std::endl;
        return false;
    }

    // Retrieve PTX code
    size_t ptx_size = 0;
    NVRTC_CHECK_ERROR(nvrtcGetPTXSize(prog, &ptx_size));
    ptx.resize(ptx_size);
    NVRTC_CHECK_ERROR(nvrtcGetPTX(prog, &ptx[0]));

    // Cleanup
    NVRTC_CHECK_ERROR(nvrtcDestroyProgram(&prog));
    return true;
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
        bool compiled = false;
        while (!compiled) {
            getCuStringFromFile(cu, location, "", filename);

            compiled =
                getPtxFromCuString(*ptx, "", cu.c_str(), location.c_str(), log);
        }
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

    [[nodiscard]] const OptiXProgramGroupDesc& getDesc() const override
    {
        return desc;
    }

    OptixProgramGroupKind getKind() const override;

   protected:
    OptixProgramGroup getProgramGroup() const override
    {
        return hitgroup_prog_group;
    }

    OptiXProgramGroupDesc desc;
    OptixProgramGroup hitgroup_prog_group;
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

    OptixShaderBindingTable getSbt() const override
    {
        return sbt;
    }

   private:
    CUDALinearBufferHandle raygen_record;
    CUDALinearBufferHandle hitgroup_record;
    CUDALinearBufferHandle miss_record;
    CUDALinearBufferHandle callable_record;
    OptiXPipelineDesc desc;
    OptixPipeline pipeline;
    OptixShaderBindingTable sbt = {};
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

struct CUDALinearBufferView : RefCounter<ICUDALinearBuffer> {
    explicit CUDALinearBufferView(const CUDALinearBufferDesc& desc, void* data);

    [[nodiscard]] const CUDALinearBufferDesc& getDesc() const override
    {
        return desc;
    }

    CUdeviceptr get_device_ptr() override;

   protected:
    thrust::host_vector<uint8_t> get_host_data() override;
    void assign_host_data(const thrust::host_vector<uint8_t>& data) override;

   private:
    void* cuda_ptr;
    CUDALinearBufferDesc desc;
};

CUDALinearBufferHandle borrow_cuda_linear_buffer(
    const CUDALinearBufferDesc& desc,
    void* cuda_ptr)
{
    auto buffer = new CUDALinearBufferView(desc, cuda_ptr);
    return CUDALinearBufferHandle::Create(buffer);
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

OptixProgramGroupKind OptiXProgramGroup::getKind() const
{
    return desc.prog_group_desc.kind;
}

template<typename IntegerType>
IntegerType roundUp(IntegerType x, IntegerType y)
{
    return ((x + y - 1) / y) * y;
}

template<typename T>
struct SbtRecord {
    __align__(
        OPTIX_SBT_RECORD_ALIGNMENT) char header[OPTIX_SBT_RECORD_HEADER_SIZE];
    T data;
};

using RayGenSbtRecord = SbtRecord<int>;
using HitGroupSbtRecord = SbtRecord<int>;
using MissSbtRecord = SbtRecord<int>;
using CallableSbtRecord = SbtRecord<int>;

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

    OptiXProgramGroupHandle solid_raygen_group;
    std::vector<OptiXProgramGroupHandle> solid_hitgroup_group;
    std::vector<OptiXProgramGroupHandle> solid_miss_group;
    std::vector<OptiXProgramGroupHandle> solid_callable_group;

    for (auto&& program_group : program_groups) {
        switch (program_group->getKind()) {
            case OPTIX_PROGRAM_GROUP_KIND_RAYGEN:
                solid_raygen_group = program_group;
                break;
            case OPTIX_PROGRAM_GROUP_KIND_HITGROUP:
                solid_hitgroup_group.push_back(program_group);
                break;
            case OPTIX_PROGRAM_GROUP_KIND_MISS:
                solid_miss_group.push_back(program_group);
                break;
            case OPTIX_PROGRAM_GROUP_KIND_CALLABLES:
                solid_callable_group.push_back(program_group);
                break;
            default: throw std::runtime_error("Unknown program group kind.");
        }
    }

    unsigned hitgroup_count = solid_hitgroup_group.size();
    unsigned miss_count = solid_miss_group.size();
    unsigned callable_count = solid_callable_group.size();

    const size_t raygen_record_size = sizeof(RayGenSbtRecord);
    raygen_record = create_cuda_linear_buffer(
        CUDALinearBufferDesc{ 1, raygen_record_size });
    CUdeviceptr d_raygen_record = raygen_record->get_device_ptr();
    RayGenSbtRecord rg_sbt;
    OPTIX_CHECK(optixSbtRecordPackHeader(
        solid_raygen_group->getProgramGroup(), &rg_sbt));
    CUDA_CHECK(cudaMemcpy(
        reinterpret_cast<void*>(d_raygen_record),
        &rg_sbt,
        raygen_record_size,
        cudaMemcpyHostToDevice));

    unsigned hitgroupRecordStrideInBytes =
        roundUp<int>(sizeof(HitGroupSbtRecord), OPTIX_SBT_RECORD_ALIGNMENT);

    const int hitgroup_record_size =
        hitgroupRecordStrideInBytes * hitgroup_count;

    hitgroup_record = create_cuda_linear_buffer(
        CUDALinearBufferDesc{ hitgroup_count, hitgroupRecordStrideInBytes });
    CUdeviceptr d_hitgroup_record = hitgroup_record->get_device_ptr();
    std::vector<HitGroupSbtRecord> hg_sbts(hitgroup_count);

    for (int i = 0; i < hitgroup_count; ++i) {
        OPTIX_CHECK(optixSbtRecordPackHeader(
            solid_hitgroup_group[i]->getProgramGroup(), &hg_sbts[i]));
    }

    CUDA_CHECK(cudaMemcpy(
        reinterpret_cast<void*>(d_hitgroup_record),
        hg_sbts.data(),
        hitgroup_record_size,
        cudaMemcpyHostToDevice));

    if (miss_count > 0) {
        unsigned missRecordStrideInBytes =
            roundUp<size_t>(sizeof(MissSbtRecord), OPTIX_SBT_RECORD_ALIGNMENT);

        int miss_record_size = missRecordStrideInBytes * miss_count;

        miss_record = create_cuda_linear_buffer(
            CUDALinearBufferDesc{ miss_count, missRecordStrideInBytes });

        CUdeviceptr d_miss_record = miss_record->get_device_ptr();

        std::vector<MissSbtRecord> ms_sbts(miss_count);
        for (int i = 0; i < miss_count; ++i) {
            // currently, do nothing.
            OPTIX_CHECK(optixSbtRecordPackHeader(
                solid_miss_group[i]->getProgramGroup(), &ms_sbts[i]));
        }

        CUDA_CHECK(cudaMemcpy(
            reinterpret_cast<void*>(d_miss_record),
            ms_sbts.data(),
            miss_record_size,
            cudaMemcpyHostToDevice));

        sbt.missRecordBase = CUdeviceptr(d_miss_record);
        sbt.missRecordStrideInBytes = missRecordStrideInBytes;
        sbt.missRecordCount = miss_count;
    }
    else {
        sbt.missRecordCount = sbt.missRecordStrideInBytes = sbt.missRecordBase =
            0;
    }

    if (callable_count > 0) {
        unsigned callableRecordStrideInBytes = roundUp<size_t>(
            sizeof(CallableSbtRecord), OPTIX_SBT_RECORD_ALIGNMENT);

        int callable_record_size = callableRecordStrideInBytes * callable_count;

        callable_record = create_cuda_linear_buffer(CUDALinearBufferDesc{
            callable_count, callableRecordStrideInBytes });

        CUdeviceptr d_callable_record = callable_record->get_device_ptr();

        std::vector<CallableSbtRecord> ms_sbts(callable_count);
        for (int i = 0; i < callable_count; ++i) {
            // currently, do nothing.
            OPTIX_CHECK(optixSbtRecordPackHeader(
                solid_callable_group[i]->getProgramGroup(), &ms_sbts[i]));
        }

        CUDA_CHECK(cudaMemcpy(
            reinterpret_cast<void*>(d_callable_record),
            ms_sbts.data(),
            callable_record_size,
            cudaMemcpyHostToDevice));

        sbt.callablesRecordBase = CUdeviceptr(d_callable_record);
        sbt.callablesRecordStrideInBytes = callableRecordStrideInBytes;
        sbt.callablesRecordCount = callable_count;
    }
    else {
        sbt.callablesRecordCount = sbt.callablesRecordStrideInBytes =
            sbt.callablesRecordBase = 0;
    }

    sbt.raygenRecord = CUdeviceptr(d_raygen_record);
    sbt.hitgroupRecordBase = CUdeviceptr(d_hitgroup_record);
    sbt.hitgroupRecordStrideInBytes = hitgroupRecordStrideInBytes;
    sbt.hitgroupRecordCount = hitgroup_count;
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
    CUDA_SYNC_CHECK();

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

CUDALinearBufferView::CUDALinearBufferView(
    const CUDALinearBufferDesc& desc,
    void* data)
{
    this->desc = desc;
    cuda_ptr = data;
}

CUdeviceptr CUDALinearBufferView::get_device_ptr()
{
    return reinterpret_cast<CUdeviceptr>(cuda_ptr);
}

thrust::host_vector<uint8_t> CUDALinearBufferView::get_host_data()
{
    thrust::host_vector<uint8_t> host_data(
        desc.element_size * desc.element_count);
    cudaMemcpy(
        host_data.data(), cuda_ptr, host_data.size(), cudaMemcpyDeviceToHost);
    return host_data;
}

void CUDALinearBufferView::assign_host_data(
    const thrust::host_vector<uint8_t>& data)
{
    assert(data.size() == desc.element_size * desc.element_count);
    cudaMemcpy(cuda_ptr, data.data(), data.size(), cudaMemcpyHostToDevice);
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

OptiXTraversableHandle create_linear_curve_optix_traversable(
    std::vector<CUdeviceptr> vertexBuffer,
    unsigned int numVertices,
    std::vector<CUdeviceptr> widthBuffer,
    CUdeviceptr indexBuffer,
    unsigned int numPrimitives,
    bool rebuilding,
    OptixPrimitiveType primitive_type)
{
    OptiXTraversableDesc desc;

    desc.buildInput.type = OPTIX_BUILD_INPUT_TYPE_CURVES;

    OptixBuildInputCurveArray& curveArray = desc.buildInput.curveArray;
    curveArray.curveType = primitive_type;

    curveArray.numPrimitives = numPrimitives;

    curveArray.vertexBuffers = vertexBuffer.data();
    curveArray.numVertices = numVertices;
    curveArray.vertexStrideInBytes = sizeof(float3);
    curveArray.widthBuffers = widthBuffer.data();
    curveArray.widthStrideInBytes = sizeof(float);
    curveArray.indexBuffer = indexBuffer;
    curveArray.indexStrideInBytes = sizeof(unsigned int);
    curveArray.flag = OPTIX_GEOMETRY_FLAG_REQUIRE_SINGLE_ANYHIT_CALL;
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

OptiXTraversableHandle create_mesh_optix_traversable(
    std::vector<CUdeviceptr> vertexBuffer,
    unsigned int numVertices,
    unsigned int vertexBufferStride,
    CUdeviceptr indexBuffer,
    unsigned int numPrimitives,
    bool rebuilding)
{
    OptiXTraversableDesc desc;

    desc.buildInput.type = OPTIX_BUILD_INPUT_TYPE_TRIANGLES;

    OptixBuildInputTriangleArray& triangleArray = desc.buildInput.triangleArray;
    triangleArray.numVertices = numVertices;
    triangleArray.vertexBuffers = vertexBuffer.data();
    triangleArray.vertexStrideInBytes = vertexBufferStride;
    triangleArray.vertexFormat = OPTIX_VERTEX_FORMAT_FLOAT3;
    triangleArray.indexBuffer = indexBuffer;
    triangleArray.indexStrideInBytes = sizeof(unsigned int) * 3;
    triangleArray.numIndexTriplets = numPrimitives;
    triangleArray.indexFormat = OPTIX_INDICES_FORMAT_UNSIGNED_INT3;
    triangleArray.numSbtRecords = 1;

    unsigned int flags = OPTIX_GEOMETRY_FLAG_NONE;

    triangleArray.flags = &flags;

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
    const char* entry_name,
    const char* param_name)
{
    auto module = create_optix_module(file_path, param_name);

    OptiXProgramGroupDesc desc;
    desc.set_program_group_kind(OPTIX_PROGRAM_GROUP_KIND_RAYGEN)
        .set_entry_name(entry_name);

    return create_optix_program_group(desc, module);
}

OptiXProgramGroupHandle create_optix_miss(
    const std::string& file_path,
    const char* entry_name,
    const char* param_name)
{
    auto module = create_optix_module(file_path, param_name);

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
#ifdef NDEBUG
    module_compile_options.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_NONE;
#else
    module_compile_options.debugLevel = OPTIX_COMPILE_DEBUG_LEVEL_MODERATE;
#endif
    module_compile_options.numBoundValues = 0;
    module_compile_options.boundValues = nullptr;

    return module_compile_options;
}

OptixPipelineCompileOptions get_default_pipeline_compile_options(
    const char* launch_param_name)
{
    OptixPipelineCompileOptions pipeline_compile_options = {};
    pipeline_compile_options.traversableGraphFlags =
        OPTIX_TRAVERSABLE_GRAPH_FLAG_ALLOW_ANY;
    pipeline_compile_options.usesMotionBlur = false;
    pipeline_compile_options.numPayloadValues = 32;
    pipeline_compile_options.numAttributeValues = 8;
    pipeline_compile_options.exceptionFlags = OPTIX_EXCEPTION_FLAG_NONE;
    pipeline_compile_options.pipelineLaunchParamsVariableName =
        launch_param_name;
    pipeline_compile_options.usesPrimitiveTypeFlags = 0;
    pipeline_compile_options.usesPrimitiveTypeFlags |=
        OPTIX_PRIMITIVE_TYPE_FLAGS_TRIANGLE;
    pipeline_compile_options.usesPrimitiveTypeFlags |=
        OPTIX_PRIMITIVE_TYPE_FLAGS_CUSTOM;
    pipeline_compile_options.usesPrimitiveTypeFlags |=
        OPTIX_PRIMITIVE_TYPE_FLAGS_ROUND_LINEAR;
    pipeline_compile_options.usesPrimitiveTypeFlags |=
        OPTIX_PRIMITIVE_TYPE_FLAGS_ROUND_QUADRATIC_BSPLINE;
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

OptiXModuleHandle create_optix_module(
    const std::string& file_path,
    const char* param_name)
{
    OptiXModuleDesc desc;
    desc.file_name = file_path;
    desc.module_compile_options = get_default_module_compile_options();
    desc.pipeline_compile_options =
        get_default_pipeline_compile_options(param_name);
    return create_optix_module(desc);
}

OptiXModuleHandle get_builtin_module(
    OptixPrimitiveType type,
    const char* param_name)
{
    OptiXModuleDesc desc;
    desc.module_compile_options = get_default_module_compile_options();
    desc.pipeline_compile_options =
        get_default_pipeline_compile_options(param_name);
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
    std::vector<OptiXProgramGroupHandle> program_groups,
    const char* param_name)
{
    OptiXPipelineDesc desc;
    desc.pipeline_compile_options =
        get_default_pipeline_compile_options(param_name);
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