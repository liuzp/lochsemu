#include "stdafx.h"
#include "taint.h"


Taint::Taint( )
{
    //m_data          = new byte[nIndices];
    //ZeroMemory(m_data, sizeof(byte) * Width);
    ZeroMemory(m_data, sizeof(m_data));
}

Taint::Taint( const Taint &t )
{
    //m_nIndices      = t.m_nIndices;
    //m_data          = new byte[m_nIndices];
    memcpy(m_data, t.m_data, Width);
}

// Taint::Taint( Taint &&t )
// {
//     m_data          = t.m_data;
//     m_nIndices      = t.m_nIndices;
// 
//     t.m_data        = NULL;
//     t.m_nIndices    = 0;
// }

Taint & Taint::operator=( Taint rhs )
{
    //std::swap(m_nIndices, rhs.m_nIndices);
    //std::swap(m_data, rhs.m_data);
    memcpy(m_data, rhs.m_data, Width);
    return *this;
}

Taint::~Taint()
{
    //SAFE_DELETE_ARRAY(m_data);
}

Taint Taint::operator&( const Taint &rhs ) const
{
    Taint res = *this;
    res &= rhs;
    return res;
}

Taint Taint::operator|( const Taint &rhs ) const
{
    Taint res = *this;
    res |= rhs;
    return res;
}

Taint Taint::operator^( const Taint &rhs ) const
{
    Taint res = *this;
    res ^= rhs;
    return res;
}

Taint& Taint::operator&=( const Taint &rhs )
{
    //Assert(m_nIndices == rhs.m_nIndices);
    for (int i = 0; i < Width; i++)
        m_data[i] &= rhs.m_data[i];
    return *this;
}

Taint& Taint::operator|=( const Taint &rhs )
{
    //Assert(m_nIndices == rhs.m_nIndices);
    for (int i = 0; i < Width; i++)
        m_data[i] |= rhs.m_data[i];
    return *this;
}

Taint& Taint::operator^=( const Taint &rhs )
{
    //Assert(m_nIndices == rhs.m_nIndices);
    for (int i = 0; i < Width; i++)
        m_data[i] ^= rhs.m_data[i];
    return *this;
}

std::string Taint::ToString() const
{
    std::string r;
    for (int i = 0; i < Width; i++)
        r += ('0' + m_data[i]);
    return r;
}

Taint Taint::FromBinString( const std::string &s )
{
    Taint r;
    if (s.size() > Width) {
        LxFatal("Size of string exceeded width of Taint\n");
    }
    for (uint i = 0; i < s.size(); i++) {
        int val = s[i] - '0';
        if (val != 0 && val != 1)
            LxFatal("Invalid character %c in string %s\n", s[i], s);
        if (val != 0) r.Set(i);
    }
    return r;
}

// 
// Taint4 Taint4::operator&( const Taint4 &rhs ) const
// {
//     Taint4 res = *this;
//     res &= rhs;
//     return res;
// }
// 
// Taint4 Taint4::operator|( const Taint4 &rhs ) const
// {
//     Taint4 res = *this;
//     res |= rhs;
//     return res;
// }
// 
// Taint4 Taint4::operator^( const Taint4 &rhs ) const
// {
//     Taint4 res = *this;
//     res ^= rhs;
//     return res;
// }
// 
// Taint4& Taint4::operator&=( const Taint4 &rhs )
// {
//     T[0] &= rhs.T[0];
//     T[1] &= rhs.T[1];
//     T[2] &= rhs.T[2];
//     T[3] &= rhs.T[3];
//     return *this;
// }
// 
// Taint4& Taint4::operator|=( const Taint4 &rhs )
// {
//     T[0] |= rhs.T[0];
//     T[1] |= rhs.T[1];
//     T[2] |= rhs.T[2];
//     T[3] |= rhs.T[3];
//     return *this;
// }
// 
// Taint4& Taint4::operator^=( const Taint4 &rhs )
// {
//     T[0] ^= rhs.T[0];
//     T[1] ^= rhs.T[1];
//     T[2] ^= rhs.T[2];
//     T[3] ^= rhs.T[3];
//     return *this;
// }

// Taint * TaintFactory::Create()
// {
//     return m_factory.Keep(new Taint);
// }

// Taint * TaintFactory::Create( int nIndices )
// {
//     return m_factory.Keep(new Taint(nIndices));
// }

