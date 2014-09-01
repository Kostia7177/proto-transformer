#pragma once

namespace ProtoTransformer
{

template<int idx>
struct BinderPos
{
    // 'length' plays a double role. it
    // calculates an index that will be passed
    // to the next field, and - at the last field
    // of hierarhy - equals an 'actual length' of
    // it (number of meaningfull fields it contains)
    enum { length = idx, pos = length };
};

template<typename T>
struct BinderData
{
    T value;
    typedef T RetType;

    BinderData(T arg) : value(arg) {}
    BinderData(){}
    // 'get' is accessible at the last
    // field of hierarchy - see (***). within all
    // other cases it is hiden by same name
    // whithin derived;
    template<int> RetType get() const { return value; }
};

}
