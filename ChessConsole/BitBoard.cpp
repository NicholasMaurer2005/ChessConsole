#include "BitBoard.h"
#include <bit>

BitBoard::BitBoard()
	: m_board() {}

BitBoard::BitBoard(std::uint64_t board)
	: m_board(board) {}

std::uint64_t BitBoard::board() const 
{ 
	return m_board; 
}

void BitBoard::setBoard(std::uint64_t board) 
{ 
	m_board = board; 
}

void BitBoard::print() const
{
	//loop over ranks 
	for (std::size_t r{}; r < RANK_MAX; r++)
	{
		std::cout << RANK_MAX - r << "    ";
		//loop over files
		for (std::size_t f{}; f < FILE_MAX; f++)
		{
			const std::uint32_t bit{ static_cast<std::uint32_t>(test(bitset_index(f, r)) ? 1 : 0 )};
			std::cout << bit << " ";
		}
		//end the line
		std::cout << std::endl;
	}
	std::cout << "\n     A B C D E F G H\n\n";
}

std::size_t BitBoard::bitCount() const
{
	return ::std::popcount(m_board);
}

size_t BitBoard::find_1lsb() const
{
	auto const index = ::std::countr_zero(m_board);
	constexpr decltype(m_board) zero = 0;
	constexpr auto max = ::std::countr_zero(zero);

	if (index < max)
	{
		return static_cast<std::size_t>(index);
	}
	else
	{
		return SIZE_MAX;
	}
}

bool BitBoard::set_rf_safe(const std::size_t r, const std::size_t f)
{
	if (f >= 0 && f < FILE_MAX && r >= 0 && r < RANK_MAX)
	{
		const std::size_t index{ bitset_index(f, r) };
		set(index);
		return true;
	}
	else
	{
		return false;
	}
}

bool BitBoard::test_rf(const std::size_t r, const std::size_t f) const
{
	return test(bitset_index(f, r));
}

void BitBoard::set_rf(const std::size_t r, const std::size_t f)
{
	const std::size_t index{ bitset_index(f, r) };
	set(index);
}

std::size_t BitBoard::bitset_index(const std::size_t x, const std::size_t y)
{
	return y * FILE_MAX + x;
}

void BitBoard::set(const std::size_t i)
{
	constexpr std::size_t single_bit{ 1 };
	m_board |= (single_bit << i);
}

void BitBoard::reset(const std::size_t i)
{
	constexpr std::size_t single_bit{ 1 };
	m_board ^= (single_bit << i);
}

bool BitBoard::test(const std::size_t i) const
{
	constexpr std::size_t single_bit{ 1 };
	return (m_board & (single_bit << i)) != 0;
}

static size_t find_1lsb(std::uint64_t num)
{
	BitBoard board;
	board.setBoard(num);

	return board.find_1lsb();
}