#pragma once

#include "ChessConstants.hpp"
#include "BitBoard.h"
#include <array>
#include <vector>
#include <intrin.h>
#include <iostream>
#include "Random.h"
#include <unordered_set>
#include "PregeneratedMagics.hpp"
#include <bitset>
#include <cstddef>


class PreGen
{
private:
	// Non Slider Attack Masks
	std::array<std::array<BitBoard, MAX_BOARD_POSITIONS>, 2> m_pawnAttackMasks;
	std::array<BitBoard, MAX_BOARD_POSITIONS> m_knightAttackMasks;
	std::array<BitBoard, MAX_BOARD_POSITIONS> m_kingAttackMasks;

	// Slider Attack Masks
	std::array<std::array<BitBoard, MAX_BISHOP_ATTACKS>, MAX_BOARD_POSITIONS> m_bishopAttackMask;
	std::array<std::array<BitBoard, MAX_ROOK_ATTACKS>, MAX_BOARD_POSITIONS> m_rookAttackMask;

	std::array<BitBoard, MAX_BOARD_POSITIONS> m_bishopRelevantBits;
	std::array<BitBoard, MAX_BOARD_POSITIONS> m_rookRelevantBits;

	std::array<std::size_t, MAX_BOARD_POSITIONS> m_bishopBitCount;
	std::array<std::size_t, MAX_BOARD_POSITIONS> m_rookBitCount;
	
	// Magic Numbers
	std::array<std::uint64_t, MAX_BOARD_POSITIONS> m_bishopMagics;
	std::array<std::uint64_t, MAX_BOARD_POSITIONS> m_rookMagics;

	// Edges
	BitBoard m_northEdgeMask;
	BitBoard m_eastEdgeMask;
	BitBoard m_southEdgeMask;
	BitBoard m_westEdgeMask;

public:

	PreGen();

	const std::array<std::array<BitBoard, MAX_BOARD_POSITIONS>, 2>& pawnAttacks() const;

	const std::array<BitBoard, MAX_BOARD_POSITIONS>& knightAttacks() const;

	const std::array<BitBoard, MAX_BOARD_POSITIONS>& kingAttacks() const;

	const std::array<std::array<BitBoard, MAX_BISHOP_ATTACKS>, MAX_BOARD_POSITIONS>& bishopAttacks() const;

	const std::array<std::array<BitBoard, MAX_ROOK_ATTACKS>, MAX_BOARD_POSITIONS>& rookAttacks() const;

	const std::array<std::uint64_t, MAX_BOARD_POSITIONS>& bishopMagics() const;

	const std::array<std::uint64_t, MAX_BOARD_POSITIONS>& rookMagics() const;

	const std::array<std::size_t, MAX_BOARD_POSITIONS>& bishopBitCount() const;

	const std::array<std::size_t, MAX_BOARD_POSITIONS>& rookBitCount() const;
	


	// Non Slider Piece Creation
	void createPawnAttackMasks();

	void createKnightAttackMasks();

	void createKingAttackMasks();

	void createEdgeMasks();



	// Slider Piece Creation
	void createBishopAttackMasks();

	void createRookAttackMasks();

	void createBishopBitCount();

	void createRookBitCount();

	void createBishopAttacks();

	void createRookAttacks();



	// Magic Numbers
	void InitMagicNumbers();

	void createBishopMagics();

	void createRookMagics();

	static std::vector<BitBoard> createOccupancies(const std::uint64_t mask, const std::size_t num_bits, const std::size_t num_occupancies);

	std::uint64_t findMagicNumber(const std::size_t square, const Piece piece);

	template<Piece P>
	BitBoard createSliderAttack(const std::size_t square, const BitBoard occupancy);


	
	// Helper Funcitons
	template<Piece P>
	std::uint64_t maskRelevantBits(const BitBoard occupancy, const std::size_t square) const
	{
		if constexpr (P == Piece::BISHOP)
		{
			return occupancy.board() & m_bishopRelevantBits[square].board();
		}
		else if constexpr (P == Piece::ROOK)
		{
			return occupancy.board() & m_rookRelevantBits[square].board();
		}
		else
		{
			return occupancy.board();
		}
	}

	static std::size_t indexAttackTable(const std::size_t rank, const std::size_t file);

	void printMasks(Piece P) const;
};
