#include "optical_material.hpp"

#include "diff_optics/lens_system.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
OpticalProperty get_optical_property(const std::string& name)
{
    std::string upper_name = name;
    std::transform(
        upper_name.begin(), upper_name.end(), upper_name.begin(), ::toupper);

    if (upper_name == "VACUUM") {
        return OpticalProperty{ 1.0, std::numeric_limits<float>::infinity() };
    }
    else if (upper_name == "AIR") {
        return OpticalProperty{ 1.000293,
                                std::numeric_limits<float>::infinity() };
    }
    else if (upper_name == "OCCLUDER") {
        return OpticalProperty{ 1.0, std::numeric_limits<float>::infinity() };
    }
    else if (upper_name == "F2") {
        return OpticalProperty{ 1.620, 36.37 };
    }
    else if (upper_name == "F15") {
        return OpticalProperty{ 1.60570, 37.831 };
    }
    else if (upper_name == "UVFS") {
        return OpticalProperty{ 1.458, 67.82 };
    }
    else if (upper_name == "BK10") {
        return OpticalProperty{ 1.49780, 66.954 };
    }
    else if (upper_name == "N-BAF10") {
        return OpticalProperty{ 1.67003, 47.11 };
    }
    else if (upper_name == "N-BK7") {
        return OpticalProperty{ 1.51680, 64.17 };
    }
    else if (upper_name == "N-SF1") {
        return OpticalProperty{ 1.71736, 29.62 };
    }
    else if (upper_name == "N-SF2") {
        return OpticalProperty{ 1.64769, 33.82 };
    }
    else if (upper_name == "N-SF4") {
        return OpticalProperty{ 1.75513, 27.38 };
    }
    else if (upper_name == "N-SF5") {
        return OpticalProperty{ 1.67271, 32.25 };
    }
    else if (upper_name == "N-SF6") {
        return OpticalProperty{ 1.80518, 25.36 };
    }
    else if (upper_name == "N-SF6HT") {
        return OpticalProperty{ 1.80518, 25.36 };
    }
    else if (upper_name == "N-SF8") {
        return OpticalProperty{ 1.68894, 31.31 };
    }
    else if (upper_name == "N-SF10") {
        return OpticalProperty{ 1.72828, 28.53 };
    }
    else if (upper_name == "N-SF11") {
        return OpticalProperty{ 1.78472, 25.68 };
    }
    else if (upper_name == "SF1") {
        return OpticalProperty{ 1.71736, 29.51 };
    }
    else if (upper_name == "SF2") {
        return OpticalProperty{ 1.64769, 33.85 };
    }
    else if (upper_name == "SF4") {
        return OpticalProperty{ 1.75520, 27.58 };
    }
    else if (upper_name == "SF5") {
        return OpticalProperty{ 1.67270, 32.21 };
    }
    else if (upper_name == "SF6") {
        return OpticalProperty{ 1.80518, 25.43 };
    }
    else if (upper_name == "SF18") {
        return OpticalProperty{ 1.72150, 29.245 };
    }
    else if (upper_name == "BAF10") {
        return OpticalProperty{ 1.67, 47.05 };
    }
    else if (upper_name == "SK1") {
        return OpticalProperty{ 1.61030, 56.712 };
    }
    else if (upper_name == "SK16") {
        return OpticalProperty{ 1.62040, 60.306 };
    }
    else if (upper_name == "SSK4") {
        return OpticalProperty{ 1.61770, 55.116 };
    }
    else if (upper_name == "B270") {
        return OpticalProperty{ 1.52290, 58.50 };
    }
    else if (upper_name == "S-NPH1") {
        return OpticalProperty{ 1.8078, 22.76 };
    }
    else if (upper_name == "D-K59") {
        return OpticalProperty{ 1.5175, 63.50 };
    }
    else if (upper_name == "FLINT") {
        return OpticalProperty{ 1.6200, 36.37 };
    }
    else if (upper_name == "PMMA") {
        return OpticalProperty{ 1.491756, 58.00 };
    }
    else if (upper_name == "POLYCARB") {
        return OpticalProperty{ 1.585470, 30.00 };
    }
    else {
        return OpticalProperty{ 1.0, 0.0 };
    }
}
USTC_CG_NAMESPACE_CLOSE_SCOPE