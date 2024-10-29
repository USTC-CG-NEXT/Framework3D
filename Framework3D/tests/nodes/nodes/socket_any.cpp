#include <iostream>
#include <boost/python/dict.hpp>

int main()
{
    // An example of how to use boost::python::dict

    Py_Initialize();

    try {
        auto dict = boost::python::dict();
        dict["key1"] = 1;
        dict["key2"] = 2;

        auto c = dict.keys();

    }
    catch (const boost::python::error_already_set&) {
        PyErr_Print();
    }
}