#include "ExampleFeatures.hpp"

std::ostream &operator<<(
    std::ostream &stream,
    const Answer &answer)
{
    stream << answer.get<textField>() << "\t";
    if (answer.get<modeField>() == 'x') { stream << std::hex; }
    else if (answer.get<modeField>() == 'd') { stream << std::dec; }
    for (size_t idx = 0; idx < answer.get<numOfInts>() && idx < answer.get<intField>().size; ++ idx)
    {
        if (idx) { stream << ", "; }
        if (answer.get<modeField>() == 't')
        {
            time_t startedAt = answer.get<intField>()[idx];
            stream << ctime(&startedAt);
        }
        else
        {
            stream << (uint32_t)answer.get<intField>()[idx];
        }
    }
    return stream;
}
