#include "ExampleFeatures.hpp"

std::ostream &operator<<(
    std::ostream &stream,
    const Answer &answer)
{
    stream << answer.get<textField>() << "\t";
    for (size_t idx = 0; idx < answer.get<numOfInts>() && idx < answer.get<intField>().size; ++ idx)
    {
        if (idx) { stream << ", "; }
        stream << answer.get<intField>()[idx];
    }
    return stream;
}
