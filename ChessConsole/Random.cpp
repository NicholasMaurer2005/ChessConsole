#include "Random.h"

Random::Random()
    : m_rng(std::time(NULL))
{
    srand(static_cast<unsigned int>(std::time(NULL)));
}

std::uint64_t Random::next_uint64_minbits_mt()
{
    const std::uint64_t p1{ m_rng() };
    const std::uint64_t p2{ m_rng() };
    const std::uint64_t p3{ m_rng() };
    return p1 & p2;

}

std::uint32_t Random::next_uint32(const std::uint32_t max) const
{
    const std::uint16_t low_half{ static_cast<std::uint16_t>(max & 0xFFFF) };
    const uint16_t high_half{ static_cast<std::uint32_t>((max >> 16) & 0xFFFF) };

    const std::uint16_t p1{ static_cast<std::uint16_t>(rand() % low_half) };
    const std::uint16_t p2{ static_cast<std::uint16_t>(rand() % high_half) };
    const std::uint32_t shifted_p2{ static_cast<std::uint32_t>(p2) << 16};

    const std::uint32_t rnd_uint{ p1 | shifted_p2 };
    return rnd_uint;
}

std::uint64_t Random::next_uint64() const
{
    const std::uint32_t p1{ next_uint32(UINT32_MAX) };
    const std::uint32_t p2{ next_uint32(UINT32_MAX) };
    const std::uint64_t shifted_p2{ static_cast<std::uint64_t>(p2) << 32 };

    const std::uint64_t rnd_u64{ shifted_p2 | p1 };

    return rnd_u64;
}

BitBoard Random::next_bitboard() const
{
    const std::uint64_t rnd_bitset{ next_uint64() };

    BitBoard board;
    board.setBoard(rnd_bitset);

    return board;
}

BitBoard Random::next_bitboard_minbits_mt()
{
    const std::uint64_t rnd_bits = next_uint64_minbits_mt();

    BitBoard board;
    board.setBoard(rnd_bits);

    return board;
}

std::uint64_t Random::next_uint64_minbits() const
{
    const std::uint64_t p1{ next_uint64() };
    const std::uint64_t p2{ next_uint64() };

    const std::uint64_t rnd_minbits{ p1 & p2 };

    return rnd_minbits;
}
