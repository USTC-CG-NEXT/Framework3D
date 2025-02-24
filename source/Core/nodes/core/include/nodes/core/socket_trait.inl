#pragma once
template<typename T>
struct ValueTrait {
    static constexpr bool has_min = false;
    static constexpr bool has_max = false;
    static constexpr bool has_default = false;
};

template<>
struct ValueTrait<bool> {
    static constexpr bool has_min = false;
    static constexpr bool has_max = false;
    static constexpr bool has_default = true;
};

template<>
struct ValueTrait<int> {
    static constexpr bool has_min = true;
    static constexpr bool has_max = true;
    static constexpr bool has_default = true;
};

template<>
struct ValueTrait<float> {
    static constexpr bool has_min = true;
    static constexpr bool has_max = true;
    static constexpr bool has_default = true;
};

template<>
struct ValueTrait<double> {
    static constexpr bool has_min = true;
    static constexpr bool has_max = true;
    static constexpr bool has_default = true;
};

template<>
struct ValueTrait<std::string> {
    static constexpr bool has_min = false;
    static constexpr bool has_max = false;
    static constexpr bool has_default = true;
};