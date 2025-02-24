/***************************************************************************
 # Copyright (c) 2015-24, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once
#include <cuda.h>

#include <vector>

#include "Core/API/CopyContext.h"
#include "Core/API/Device.h"
#include "Core/Error.h"
#include "Core/Macros.h"
#include "Core/Object.h"
#include "CudaRuntime.h"  // Instead of <cuda_runtime.h> to avoid name clashes.
#include "nvrhi/utils.h"

#define FALCOR_CUDA_CHECK(call)                                   \
    {                                                             \
        cudaError_t result = call;                                \
        if (result != cudaSuccess) {                              \
            const char* errorName = cudaGetErrorName(result);     \
            const char* errorString = cudaGetErrorString(result); \
            FALCOR_THROW(                                         \
                "CUDA call {} failed with error {} ({}).",        \
                #call,                                            \
                errorName,                                        \
                errorString);                                     \
        }                                                         \
    }

#define FALCOR_CU_CHECK(call)                              \
    do {                                                   \
        CUresult result = call;                            \
        if (result != CUDA_SUCCESS) {                      \
            const char* errorName;                         \
            cuGetErrorName(result, &errorName);            \
            const char* errorString;                       \
            cuGetErrorString(result, &errorString);        \
            FALCOR_THROW(                                  \
                "CUDA call {} failed with error {} ({}).", \
                #call,                                     \
                errorName,                                 \
                errorString);                              \
        }                                                  \
    } while (0)

/// CUDA device pointer.
typedef unsigned long long CUdeviceptr;

namespace USTC_CG {

namespace cuda_utils {
    HD_USTC_CG_API void deviceSynchronize();

    HD_USTC_CG_API void* mallocDevice(size_t size);
    HD_USTC_CG_API void freeDevice(void* devPtr);

    HD_USTC_CG_API void
    memcpyDeviceToDevice(void* dst, const void* src, size_t count);
    HD_USTC_CG_API void
    memcpyHostToDevice(void* dst, const void* src, size_t count);
    HD_USTC_CG_API void
    memcpyDeviceToHost(void* dst, const void* src, size_t count);

    HD_USTC_CG_API void memsetDevice(void* devPtr, int value, size_t count);

    HD_USTC_CG_API cudaExternalMemory_t importExternalMemory(const Buffer* buffer);
    HD_USTC_CG_API void destroyExternalMemory(cudaExternalMemory_t extMem);
    HD_USTC_CG_API void* externalMemoryGetMappedBuffer(
        cudaExternalMemory_t extMem,
        size_t offset,
        size_t size);

    HD_USTC_CG_API cudaExternalSemaphore_t
    importExternalSemaphore(const Fence* fence);
    HD_USTC_CG_API void destroyExternalSemaphore(cudaExternalSemaphore_t extSem);
    HD_USTC_CG_API void signalExternalSemaphore(
        cudaExternalSemaphore_t extSem,
        uint64_t value,
        cudaStream_t stream = 0);
    HD_USTC_CG_API void waitExternalSemaphore(
        cudaExternalSemaphore_t extSem,
        uint64_t value,
        cudaStream_t stream = 0);

    /**
     * Calls cudaFree() on the provided pointer.
     */
    HD_USTC_CG_API bool freeSharedDevicePtr(void* ptr);

    /**
     * Maps a texture to a surface object which can be read and written within a
     * CUDA kernel. This method should only be called once per texture on
     * initial load. Store the returned surface object for repeated use.
     * @param pTex Pointer to the texture being mapped
     * @param usageFlags The requested flags to be bound to the underlying
     * mipmapped array that will be used to create the surface object
     * @return The surface object that the input texture is bound to.
     */
    HD_USTC_CG_API cudaSurfaceObject_t
    mapTextureToSurface(nvrhi::TextureHandle pTex, uint32_t usageFlags);

    /// Wraps a CUDA device, context and stream.
    class HD_USTC_CG_API CudaDevice : public Object {
        FALCOR_OBJECT(cuda_utils::CudaDevice)
       public:
        /// Constructor.
        /// Creates a CUDA device on the same adapter as the Falcor device.
        CudaDevice(const Device* pDevice);
        ~CudaDevice();

        CUdevice getDevice() const
        {
            return mCudaDevice;
        }
        CUcontext getContext() const
        {
            return mCudaContext;
        }
        CUstream getStream() const
        {
            return mCudaStream;
        }

       private:
        CUdevice mCudaDevice;
        CUcontext mCudaContext;
        CUstream mCudaStream;
    };

    /// Wraps an external semaphore.
    class ExternalSemaphore : public Object {
        FALCOR_OBJECT(cuda_utils::ExternalSemaphore)
       public:
        ExternalSemaphore(nvrhi::EventQueryHandle pFence)
            : mpFence(pFence.Get())
        {
            nvrhi::utils::NotImplemented();
            // FALCOR_CHECK(mpFence, "'fence' is null.");
            // FALCOR_CHECK(
            //     mpFence->getDesc().shared,
            //     "'fence' must be created with shared=true.");
            // mExternalSemaphore = importExternalSemaphore(mpFence);
        }

        ~ExternalSemaphore()
        {
            destroyExternalSemaphore(mExternalSemaphore);
        }

        void signal(uint64_t value, cudaStream_t stream = 0)
        {
            signalExternalSemaphore(mExternalSemaphore, value, stream);
        }

        void wait(uint64_t value, cudaStream_t stream = 0)
        {
            waitExternalSemaphore(mExternalSemaphore, value, stream);
        }

        void waitForCuda(
            CopyContext* pCopyContext,
            cudaStream_t stream = 0,
            uint64_t value = 0)
        {
            nvrhi::utils::NotImplemented();
            // uint64_t signalValue = mpFence->updateSignaledValue(value);
            // signal(signalValue, stream);
            // pCopyContext->wait(mpFence, signalValue);
        }

        void waitForFalcor(
            CopyContext* pCopyContext,
            cudaStream_t stream = 0,
            uint64_t value = 0)
        {
            nvrhi::utils::NotImplemented();

            // uint64_t signalValue = pCopyContext->signal(mpFence, value);
            // wait(signalValue, stream);
        }

       private:
        /// Keep a non-owning pointer to the fence.
        /// TODO: If available, we should use a weak_ref here.
        Fence* mpFence;
        cudaExternalSemaphore_t mExternalSemaphore;
    };

}  // namespace cuda_utils

/**
 * Structure to encapsulate DX <-> CUDA interop data for a buffer.
 */
struct InteropBuffer {
    nvrhi::BufferHandle buffer;              // Falcor buffer
    CUdeviceptr devicePtr = (CUdeviceptr)0;  // CUDA pointer to buffer

    void free()
    {
        if (devicePtr) {
            cuda_utils::freeSharedDevicePtr((void*)devicePtr);
            devicePtr = (CUdeviceptr)0;
        }
    }
};

inline InteropBuffer createInteropBuffer(ref<Device> pDevice, size_t byteSize)
{
    nvrhi::utils::NotImplemented();
    InteropBuffer interop;

    //// Create a new DX <-> CUDA shared buffer using the Falcor API to create,
    //// then find its CUDA pointer.
    // interop.buffer = pDevice->getNvrhiDevice()->createBuffer(
    //     byteSize,
    //     ResourceBindFlags::ShaderResource |
    //     ResourceBindFlags::UnorderedAccess |
    //         ResourceBindFlags::Shared);
    // interop.devicePtr = (CUdeviceptr)cuda_utils::getSharedDevicePtr(
    //     pDevice->getType(),
    //     interop.buffer->getSharedApiHandle(),
    //     (uint32_t)interop.buffer->getSize());
    // FALCOR_CHECK(
    //     interop.devicePtr != (CUdeviceptr)0,
    //     "Failed to create CUDA device ptr for buffer");

    return interop;
}
}  // namespace USTC_CG
