#pragma once

#include "../../DefaultSettingsList.hpp"

namespace ProtoTransformer
{

struct DefaultCfg : virtual public DefaultSettingsList::Type {};

template<class Base, int, int> struct BaseUniquizer : public Base {};

template<int idx,
         class T1 = DefaultCfg,
         class T2 = DefaultCfg,
         class T3 = DefaultCfg,
         class T4 = DefaultCfg,
         class T5 = DefaultCfg,
         class T6 = DefaultCfg,
         class T7 = DefaultCfg,
         class T8 = DefaultCfg>
struct SettingSelector 
    : public BaseUniquizer<T1, 1, idx>,
      public BaseUniquizer<T2, 2, idx>,
      public BaseUniquizer<T3, 3, idx>,
      public BaseUniquizer<T4, 4, idx>,
      public BaseUniquizer<T5, 5, idx>,
      public BaseUniquizer<T6, 6, idx>,
      public BaseUniquizer<T7, 7, idx>,
      public BaseUniquizer<T8, 8, idx>
{
    enum { selectorIdx = idx };
};

}
