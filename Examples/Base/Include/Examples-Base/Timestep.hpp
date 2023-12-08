#pragma once

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