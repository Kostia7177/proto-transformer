#pragma once

struct GccBug47226Satellite
{   // sadly, but the g++ compiler does not expands
    // a parameter pack inside lambdas. (g++ bug 47226, see
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47226 )
    //
    // comment this structure when the problem will be solved
    // to see all the places where the problem is overriden
    // by temporary patches;
};
