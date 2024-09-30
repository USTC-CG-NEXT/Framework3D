#include <boost/python/dict.hpp>

int main()
{
    // An example of how to use boost::python::dict

    Py_Initialize();
    auto dict = boost::python::dict();
    dict["key1"] = 1;
    dict["key2"] = 2;

    dict.itervalues();
}