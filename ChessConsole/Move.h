#pragma once
#include <cstdint>
#include <cstddef>
#include "ChessConstants.hpp"
#include <iostream>

constexpr std::size_t target_shift{ 6 };
constexpr std::size_t promoted_shift{ 12 };
constexpr std::size_t enpassant_shift{ 13 };
constexpr std::size_t double_shift{ 14 };
constexpr std::size_t castle_shift{ 15 };
constexpr std::size_t piece_shift{ 16 };
constexpr std::size_t capture_shift{ 20 };
constexpr std::size_t value_shift{ 24 };

constexpr std::uint32_t source_mask         { 0b00000000000000000000000000111111 };
constexpr std::uint32_t target_mask         { 0b00000000000000000000111111000000 };
constexpr std::uint32_t promoted_mask		{ 0b00000000000000000001000000000000 };
constexpr std::uint32_t enpassant_mask		{ 0b00000000000000000010000000000000 };
constexpr std::uint32_t double_mask			{ 0b00000000000000000100000000000000 };
constexpr std::uint32_t castle_mask         { 0b00000000000000001000000000000000 };
constexpr std::uint32_t piece_mask          { 0b00000000000011110000000000000000 };
constexpr std::uint32_t capture_mask		{ 0b00000000111100000000000000000000 };
constexpr std::uint32_t value_mask          { 0b11111111000000000000000000000000 };



struct Move
{
private:
	std::uint32_t m_data;

public:

	//constructors
	
	//pawn promoted         
	Move(const std::size_t source, const std::size_t target, const Piece piece, const Piece capture_piece, const bool capture);
	//enpassant
	Move(const std::size_t source, const std::size_t target, const Piece captured_piece);
	//castle
	Move(const std::size_t target);
	//double pawn push
	Move(const std::size_t source, const std::size_t target); 
	//other         
	Move(const std::size_t source, const std::size_t target, const Piece piece, const Piece captured_piece);
	//default
	Move();



	//getters
	std::size_t source() const;

	std::size_t target() const;

	bool promoted() const;

	bool enpassant() const;

	bool doublePawnPush() const;

	bool castle() const;

	Piece piece() const;

	Piece capture() const;

	std::uint32_t value() const;



	//helpers
	Move& operator=(const Move& other);

	void print() const;

	template <Castle C>
	static Move createCastleMove()
	{
		if constexpr (C == Castle::BK)
		{
			return Move(g8);
		}

		if constexpr (C == Castle::BQ)
		{
			return Move(c8);
		}

		if constexpr (C == Castle::WK)
		{
			return Move(g1);
		}

		if constexpr (C == Castle::WQ)
		{
			return Move(c1);
		}
	}
};