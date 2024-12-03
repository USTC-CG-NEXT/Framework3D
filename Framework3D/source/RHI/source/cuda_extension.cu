#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include <RHI/internal/cuda_extension.hpp>

USTC_CG_NAMESPACE_OPEN_SCOPE

namespace cuda {
class CUDALinearBuffer : public nvrhi::RefCounter<cuda::ICUDALinearBuffer> {
   public:
    CUDALinearBuffer(const cuda::CUDALinearBufferDesc& in_desc);
    ~CUDALinearBuffer() override;

    const CUDALinearBufferDesc& getDesc() const override
    {
        return desc;
    }

    CUdeviceptr get_device_ptr() override;

   protected:
    thrust::host_vector<uint8_t> get_host_data() override;
    void assign_host_data(const thrust::host_vector<uint8_t>& data) override;

    const cuda::CUDALinearBufferDesc desc;
    thrust::device_vector<uint8_t> d_vec;
};

CUDALinearBuffer::CUDALinearBuffer(const cuda::CUDALinearBufferDesc& in_desc)
    : desc(in_desc)
{
    d_vec.resize(desc.size * desc.element_size);
}

CUDALinearBuffer::~CUDALinearBuffer()
{
}

CUdeviceptr CUDALinearBuffer::get_device_ptr()
{
    return reinterpret_cast<CUdeviceptr>(d_vec.data().get());
}

thrust::host_vector<uint8_t> CUDALinearBuffer::get_host_data()
{
    thrust::host_vector<uint8_t> h_vec = d_vec;
    return h_vec;
}

void CUDALinearBuffer::assign_host_data(
    const thrust::host_vector<uint8_t>& data)
{
    d_vec = data;
}

CUDALinearBufferHandle create_cuda_linear_buffer(const CUDALinearBufferDesc& d)
{
    auto buffer = new CUDALinearBuffer(d);
    return CUDALinearBufferHandle::Create(buffer);
}

}  // namespace cuda

USTC_CG_NAMESPACE_CLOSE_SCOPE