#pragma once
/*
    Easy way to make your own application-layer protocol. Like a robot-transformer
    Copyright (C) 2014  Konstantin U. Zozoulia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "detail/SettingSelector.hpp"

namespace ProtoTransformer
{

template<class... Params> struct Proto : public SettingSelector<0, Params...> {};

template<template<class, class> class T, class Policy> struct UsePolicy : public T<Policy, DefaultSettingsList::Type> {};

}
