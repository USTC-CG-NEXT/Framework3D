#include "shaderCompiler.h"

#include "RHI/api.h"

#ifdef _WIN32
#include <codecvt>
#endif
#include <filesystem>
#include <fstream>

#include "Logger/Logger.h"
#include "slang.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

std::filesystem::path SlangShaderCompiler::find_root(
    const std::filesystem::path& p)
{
    auto mark_prelude_name = "SDK/slang/include/slang-cuda-prelude.h";
    auto current = absolute(p);

    while (!exists(current / mark_prelude_name)) {
        if (current.has_parent_path()) {
            current = current.parent_path();
        }
        else {
            log::error("CUDA Prelude not found.");
            return "";
        }
    }
    // std::cerr << current.generic_string() << std::endl;

    return current;
}

SlangResult SlangShaderCompiler::addHLSLPrelude(slang::IGlobalSession* session)
{
    std::filesystem::path includePath = ".";

    auto root = find_root(includePath);

    auto prelude_name = "/SDK/slang/include/slang-hlsl-prelude.h";
    std::ostringstream prelude;
    prelude << "#include \"" << root.generic_string() + prelude_name
            << "\"\n\n";

    // std::cerr << prelude.str() << std::endl;
    session->setLanguagePrelude(
        SLANG_SOURCE_LANGUAGE_HLSL, prelude.str().c_str());
    return SLANG_OK;
}

SlangResult SlangShaderCompiler::addCPPPrelude(slang::IGlobalSession* session)
{
    std::filesystem::path includePath = ".";

    auto root = find_root(includePath);

    auto prelude_name = "/SDK/slang/include/slang-cpp-prelude.h";
    std::ostringstream prelude;
    prelude << "#include \"" << root.generic_string() + prelude_name
            << "\"\n\n";

    // std::cerr << prelude.str() << std::endl;
    session->setLanguagePrelude(
        SLANG_SOURCE_LANGUAGE_CPP, prelude.str().c_str());
    return SLANG_OK;
}

SlangResult SlangShaderCompiler::addCPPHeaderInclude(
    SlangCompileRequest* slangRequest)
{
    auto unordered_dense = find_root(".") / "SDK\\unordered_dense\\include";
    auto unordered_dense_command = "-I" + unordered_dense.generic_string();

    auto prelude_path = find_root(".") / "SDK\\slang\\include";

    auto prelude_command = "-I" + prelude_path.generic_string();

    // Inclusion in prelude should be passed to down stream compilers.....
    const char* args[] = { "-Xgenericcpp...",
                           unordered_dense_command.c_str(),
                           prelude_command.c_str(),
                           "-X." };
    return slangRequest->processCommandLineArguments(
        args, sizeof(args) / sizeof(const char*));
}

SlangResult SlangShaderCompiler::addHLSLHeaderInclude(
    SlangCompileRequest* slangRequest)
{
    auto hlsl_path = find_root(".") / "usd/hd_USTC_CG_GL/resources/nvapi/";

    auto hlsl_path_name = "-I" + hlsl_path.generic_string();

    // Inclusion in prelude should be passed to down stream compilers.....
    const char* args[] = { "-Xdxc...", hlsl_path_name.c_str(), "-X." };
    return slangRequest->processCommandLineArguments(
        args, sizeof(args) / sizeof(const char*));
}

SlangResult SlangShaderCompiler::addHLSLSupportPreDefine(
    SlangCompileRequest* slangRequest)
{
    // However, this predefine remains to dxc...
    slangRequest->addPreprocessorDefine("SLANG_HLSL_ENABLE_NVAPI", "1");
    slangRequest->addPreprocessorDefine(
        "NV_SHADER_EXTN_REGISTER_SPACE", "space0");
    slangRequest->addPreprocessorDefine("NV_SHADER_EXTN_SLOT", "u127");
    return SLANG_OK;
}
#if USTC_CG_WITH_CUDA

SlangResult SlangShaderCompiler::addCUDAPrelude(slang::IGlobalSession* session)
{
    std::filesystem::path includePath = ".";

    auto root = find_root(includePath);

    auto prelude_name = "/SDK/slang/include/slang-cuda-prelude.h";
    std::ostringstream prelude;
    prelude << "#include \"" << root.generic_string() + prelude_name
            << "\"\n\n";

    // std::cerr << prelude.str() << std::endl;
    session->setLanguagePrelude(
        SLANG_SOURCE_LANGUAGE_CUDA, prelude.str().c_str());
    return SLANG_OK;
}

SlangResult SlangShaderCompiler::addOptiXHeaderInclude(
    SlangCompileRequest* slangRequest)
{
    auto optix_path = find_root(".") / "usd/hd_USTC_CG_GL/resources/optix/";
    auto optix_path_name = "-I" + optix_path.generic_string();

    // Inclusion in prelude should be passed to down stream compilers.....
    const char* args[] = { "-Xnvrtc...", optix_path_name.c_str(), "-X." };
    return slangRequest->processCommandLineArguments(
        args, sizeof(args) / sizeof(const char*));
}

SlangResult SlangShaderCompiler::addOptiXSupportPreDefine(
    SlangCompileRequest* slangRequest)
{
    // However, this predefine remains to nvrtc...
    slangRequest->addPreprocessorDefine("SLANG_CUDA_ENABLE_OPTIX", "1");
    return SLANG_OK;
}

SlangResult SlangShaderCompiler::addOptiXSupport(
    SlangCompileRequest* slangRequest)
{
    addOptiXSupportPreDefine(slangRequest);
    return addOptiXHeaderInclude(slangRequest);
}

#endif

void SlangShaderCompiler::save_file(
    const std::string& filename,
    const char* data)
{
    std::ofstream file(filename);

    file << std::string(data);
    file.close();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE