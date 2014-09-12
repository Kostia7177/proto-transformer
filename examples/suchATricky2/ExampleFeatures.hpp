#pragma once

#include <cstddef>
#include "../../include/ProtoTransformer/detail/Serializer.hpp"

enum { sumNumbers, sortNumbers, terminateSession, logRequest, echo, numOfOpts };
enum { pauseField, optField, sizeField };

namespace Serializer = ProtoTransformer::Serializer;

template<typename T, int pos> using SerFD = Serializer::FieldDesc<T, pos>;

typedef Serializer::IntegralsOnly
    <
        Serializer::fit2Calculated,

        SerFD<uint32_t, pauseField>,
        SerFD<uint32_t, sizeField>,
        SerFD<uint32_t, optField>
    >
    ::Buffer AnyHdr;

enum { textField, intField, numOfInts, modeField };

typedef Serializer::IntegralsOnly
    <
        Serializer::fit2Calculated,

        SerFD<char[1024], textField>,
        SerFD<char, modeField>,
        SerFD<uint32_t[10], intField>,
        SerFD<uint32_t, numOfInts>
    >
    ::Buffer Answer;

struct AnyHdrWrapped
{
    typedef AnyHdr Itself;

    static uint32_t getSize(const AnyHdr &hdr){ return hdr.get<sizeField>(); }
    static void setSize2(uint32_t size, AnyHdr &hdr){ hdr.set<sizeField>(size); }
};

std::ostream &operator<<(std::ostream &, const Answer &);
