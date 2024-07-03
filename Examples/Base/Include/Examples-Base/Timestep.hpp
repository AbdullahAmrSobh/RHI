#pragma once

#include <cstdint>
#include <chrono>

class Timestep
{
public:
    Timestep(double timestep)
        : m_timestep(timestep)
    {
    }

    inline operator double() const
    {
        return m_timestep;
    }

    inline double Seconds() const
    {
        return Miliseconds() / 1000.0;
    }

    inline double Miliseconds() const
    {
        return m_timestep;
    }

private:
    double m_timestep;
};

class Timepoint
{
public:
    Timepoint(uint64_t timepoint);

    inline operator uint64_t() const { return m_timepoint; }

    static Timepoint Now()
    {
        return Timepoint(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    }

    inline Timestep operator-(Timepoint other)
    {
        return (other.m_timepoint - m_timepoint);
    }

private:
    uint64_t m_timepoint;
};