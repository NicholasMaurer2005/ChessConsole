#pragma once

#include <ctime>
#include <cstdlib>
#include <cstdint>
#include "ChessConstants.hpp"
#include "BitBoard.h"
#include <random>

class Random
{
private:
    std::mt19937_64 m_rng;

public:
    Random();

    std::uint32_t next_uint32(const std::uint32_t max)  const;

    std::uint64_t next_uint64() const;

    BitBoard next_bitboard() const;

    BitBoard next_bitboard_minbits_mt();

    std::uint64_t next_uint64_minbits() const;

    std::uint64_t next_uint64_minbits_mt();
};
