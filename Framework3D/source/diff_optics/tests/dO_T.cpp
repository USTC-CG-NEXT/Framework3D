#include <GUI/window.h>
#include <gtest/gtest.h>

#include <diff_optics/diff_optics.hpp>

#include "diff_optics/lens_system.hpp"

using namespace USTC_CG;

const char* str = R"(
{
    "Originate": "US02532751-1",
    "data": [
        {
            "type": "O",
            "distance": 0.0,
            "roc": 0.0,
            "diameter": 0.0,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0,
            "roc": 13.354729316461547,
            "diameter": 17.4,
            "material": "SSK4",
            "additional_params": [0.005, 1e-6, 1e-8, -3e-10]
        },
        {
            "type": "S",
            "distance": 2.2352,
            "roc": 35.64148197667863,
            "diameter": 17.4,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0762,
            "roc": 10.330017837998932,
            "diameter": 14.0,
            "material": "SK1"
        },
        {
            "type": "S",
            "distance": 3.1750,
            "roc": 0.0,
            "diameter": 14.0,
            "material": "F15"
        },
        {
            "type": "S",
            "distance": 0.9652,
            "roc": 6.494496063151893,
            "diameter": 9.0,
            "material": "VACUUM"
        },
        {
            "type": "A",
            "distance": 3.8608,
            "roc": 0.0,
            "diameter": 4.886,
            "material": "OCCLUDER"
        },
        {
            "type": "S",
            "distance": 3.302,
            "roc": -7.026950339915501,
            "diameter": 9.0,
            "material": "F15"
        },
        {
            "type": "S",
            "distance": 0.9652,
            "roc": 0.0,
            "diameter": 12.0,
            "material": "SK16"
        },
        {
            "type": "S",
            "distance": 2.7686,
            "roc": -9.746574604143909,
            "diameter": 12.0,
            "material": "VACUUM"
        },
        {
            "type": "S",
            "distance": 0.0762,
            "roc": 69.81692521236866,
            "diameter": 14.0,
            "material": "SK16"
        },
        {
            "type": "S",
            "distance": 1.7526,
            "roc": -19.226275376106166,
            "diameter": 14.0,
            "material": "VACUUM"
        },
        {
            "type": "I",
            "distance": 17.42769142705,
            "roc": 0.0,
            "diameter": 20.664,
            "material": "VACUUM"
        }
    ]
}

)";

TEST(dO_T, diff_optics)
{
    LensSystem lens_system;

    std::string json = std::string(str);
    lens_system.deserialize(json);
}