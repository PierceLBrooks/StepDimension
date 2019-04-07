
// Author: Pierce Brooks

#include "Log.hpp"

idc::Log* idc::Log::self = nullptr;

idc::Log::Log() :
    ss(new std::stringstream())
{
    self = this;
}

idc::Log::~Log()
{
    delete ss;
}

idc::Log* idc::Log::getSelf()
{
    if (self == nullptr)
    {
        new Log();
    }
    return self;
}
