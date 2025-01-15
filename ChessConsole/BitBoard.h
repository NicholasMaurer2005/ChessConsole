#pragma once

#include <string>
#include "ChessConstants.hpp"
#include <iostream>
#include <array>
#include <cstddef>

struct BitBoard
{
private:
	std::uint64_t m_board;

public:
	BitBoard();

	BitBoard(std::uint64_t board);

	std::uint64_t board() const;

	std::size_t bitCount() const;

	void setBoard(std::uint64_t board);

	void print() const;

	void set(const std::size_t i);

	void set_rf(const std::size_t r, const std::size_t f);

	bool set_rf_safe(const std::size_t r, const std::size_t f);

	void reset(const std::size_t i);

	bool test(const std::size_t i) const;

	bool test_rf(const std::size_t r, const std::size_t f) const;

	std::size_t find_1lsb() const;

	static size_t bitset_index(const std::size_t x, const std::size_t y);
};