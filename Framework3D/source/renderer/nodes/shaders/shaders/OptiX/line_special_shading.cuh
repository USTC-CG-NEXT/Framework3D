#pragma once

//using Float = float;
//#include <cuda/std/complex>
//using Complex = cuda::std::complex<Float>;
//#define Sqrt(x)    cuda::std::sqrt(Complex(x))
//#define Log        log
//#define GetReal(x) sqrtf(x.real() * x.real() + x.imag() * x.imag())

constexpr float three_rcq = 1.0f / 3.0f;


#define CalcPowerSeries(name)                   \
    Float name##_power2 = (name)*name; \
    Float name##_power3 = (name)*name##_power2; \
    Float name##_power4 = (name)*name##_power3; \
    Float name##_power5 = (name)*name##_power4; \
    Float name##_power6 = (name)*name##_power5;

#define CalcPowerSeriesComplex(name)              \
    Complex name##_power2 = (name)*name; \
    Complex name##_power3 = (name)*name##_power2; \
    Complex name##_power4 = (name)*name##_power3; \
    Complex name##_power5 = (name)*name##_power4; \
    Complex name##_power6 = (name)*name##_power5;

#define DeclarePowerSeries(name)                                                        \
    Float name, Float name##_power2, Float name##_power3, Float name##_power4, \
        Float name##_power5, Float name##_power6

#define UsePowerSeries(name)                                                                  \
    name , name##_power2, name##_power3, name##_power4, name##_power5, name##_power6

#undef Power

#define Power(name, n) name##_power##n

#ifdef __CUDACC__
#define HOSTDEVICE __host__ __device__ inline
#else
#define HOSTDEVICE inline
#endif


HOSTDEVICE Complex sumpart(
    Float lower,
    Float upper,
    Complex y,
    DeclarePowerSeries(width),
    DeclarePowerSeries(halfX),
    DeclarePowerSeries(halfZ),
    DeclarePowerSeries(r))
{
    CalcPowerSeriesComplex(y);

    auto log_val_u = log(upper - y);
    auto log_val_l = log(lower - y);
    return -(
        (((powf(halfZ - Power(halfZ, 3) * Power(r, 2), 2) +
           Power(halfX, 2) * (-1 + Power(halfZ, 4) * Power(r, 4))
          )
          *
          Power(width, 3) -
          4 * halfX * halfZ *
          (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
           Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
          Power(width, 2) * y +
          4 *
          (-7 * Power(halfX, 2) + 7 * Power(halfZ, 2) +
           2 *
           (3 * Power(halfX, 4) - 2 * Power(halfX, 2) * Power(halfZ, 2) -
            4 * Power(halfZ, 4)) *
           Power(r, 2) +
           Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
           (2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
          width * Power(y, 2) +
          64 * halfX * halfZ * (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
          Power(y, 3)
         )
         *
         (log_val_u - log_val_l)
        )
        /
        (halfX * halfZ * Power(r, 2) * Power(width, 3) +
         2 * (1 + (-2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) * Power(width, 2) * y -
         12 * halfX * halfZ * Power(r, 2) * width * Power(y, 2) -
         8 * (-1 + Power(halfZ, 2) * Power(r, 2)) * Power(y, 3))
    );
}

HOSTDEVICE auto calc_res(
    Float x,
    DeclarePowerSeries(width),
    DeclarePowerSeries(halfX),
    DeclarePowerSeries(halfZ),
    DeclarePowerSeries(r))
{
    CalcPowerSeries(x);

    return (4 *
            (halfX * halfZ * Power(r, 2) * (-1 + Power(halfZ, 2) * Power(r, 2)) *
             (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
              Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
             Power(width, 5) -
             2 *
             (powf(-1 + Power(halfZ, 2) * Power(r, 2), 3) *
              (1 + Power(halfZ, 2) * Power(r, 2)) +
              4 * Power(halfX, 4) * Power(halfZ, 2) * Power(r, 6) *
              (3 + Power(halfZ, 2) * Power(r, 2)) +
              Power(halfX, 2) * Power(r, 2) *
              (2 - 11 * Power(halfZ, 2) * Power(r, 2) +
               4 * Power(halfZ, 4) * Power(r, 4) +
               5 * Power(halfZ, 6) * Power(r, 6))
             )
             *
             Power(width, 4) * x +
             4 * halfX * halfZ * Power(r, 2) *
             (6 + (-19 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
              2 *
              (6 * Power(halfX, 4) - Power(halfX, 2) * Power(halfZ, 2) -
               4 * Power(halfZ, 4)) *
              Power(r, 4) +
              Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) *
              (4 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 6)) *
             Power(width, 3) * Power(x, 2) +
             8 * (1 + Power(halfZ, 2) * Power(r, 2)) *
             (-1 + (2 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
             (-2 + (3 * Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2) +
              Power(halfZ, 2) * (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 4)) *
             Power(width, 2) * Power(x, 3) -
             64 * halfX * halfZ * Power(r, 2) * (-1 + Power(halfZ, 2) * Power(r, 2)) *
             (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) * width * Power(x, 4) -
             32 * powf(-1 + Power(halfZ, 2) * Power(r, 2), 2) *
             (-1 + (Power(halfX, 2) + Power(halfZ, 2)) *
              Power(r, 2)) * Power(x, 5)
            )
           )
           /
           ((-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2)) *
            (-((1 + halfZ * r) * Power(width, 2)) + 4 * halfX * r * width * x +
             4 * (-1 + halfZ * r) * Power(x, 2)) *
            ((1 - halfZ * r) * Power(width, 2) + 4 * halfX * r * width * x +
             4 * (1 + halfZ * r) * Power(x, 2)));
}


HOSTDEVICE

Float lineShade(Float lower, Float upper, Float alpha, Float halfX, Float halfZ, Float width)
{
    Float r = sqrtf(1 - alpha * alpha);

    CalcPowerSeries(width);
    CalcPowerSeries(halfX);
    CalcPowerSeries(halfZ);
    CalcPowerSeries(r);

    Complex temp = Sqrt(
        -Power(width, 2) + Power(halfX, 2) * Power(r, 2) * Power(width, 2) +
        Power(halfZ, 2) * Power(r, 2) * Power(width, 2));

    Complex c[] = { (-(halfX * r * width) - temp) / (Float(2.) * (-1 + halfZ * r)),
                    (-(halfX * r * width) - temp) / (Float(2.) * (1 + halfZ * r)),
                    (-(halfX * r * width) + temp) / (Float(2.) * (-1 + halfZ * r)),
                    (-(halfX * r * width) + temp) / (Float(2.) * (1 + halfZ * r)) };

    auto ret = Complex(0, 0);

    for (int i = 0; i < 4; i++)
    {
        auto part = sumpart(
            lower,
            upper,
            c[i],
            UsePowerSeries(width),
            UsePowerSeries(halfX),
            UsePowerSeries(halfZ),
            UsePowerSeries(r));

        ret += part;
    }

    ret *= (Power(r, 2) * (-1 + Power(halfZ, 2) * Power(r, 2)) * width) /
        (-1 + (Power(halfX, 2) + Power(halfZ, 2)) * Power(r, 2));

    ret += calc_res(
            upper,
            UsePowerSeries(width),
            UsePowerSeries(halfX),
            UsePowerSeries(halfZ),
            UsePowerSeries(r)) -
        calc_res(
            lower,
            UsePowerSeries(width),
            UsePowerSeries(halfX),
            UsePowerSeries(halfZ),
            UsePowerSeries(r));
    Float coeff = -alpha * alpha / (Float(8.) * Pi * powf(-1 + Power(halfZ, 2) * Power(r, 2), 3));

    ret *= coeff;

    return GetReal(ret);
}
