#include "PreGen.h"



PreGen::PreGen()
	: m_pawnAttackMasks(), m_knightAttackMasks(), m_kingAttackMasks(), m_bishopAttackMask(), m_rookAttackMask(),

	m_bishopRelevantBits(), m_rookRelevantBits(), 

	m_bishopMagics(), m_rookMagics(),

	m_northEdgeMask(), m_eastEdgeMask(), m_southEdgeMask(), m_westEdgeMask(),

	m_bishopBitCount(), m_rookBitCount()
{
	//slider masks 
	createBishopAttackMasks();
	createRookAttackMasks();

	//magics
	createBishopBitCount();
	createRookBitCount();
	InitMagicNumbers();

	std::cout << "Generating Tables" << std::endl;
	//bishop and rook attacks
	createBishopAttacks();
	createRookAttacks();

	//non slider attacks
	createPawnAttackMasks();
	createKnightAttackMasks();
	createKingAttackMasks();
	createEdgeMasks();

	std::cout << "Tables Generated" << std::endl;
}

// Getters
const std::array<std::array<BitBoard, MAX_BOARD_POSITIONS>, 2>& PreGen::pawnAttacks() const
{
	return m_pawnAttackMasks;
}

const std::array<BitBoard, MAX_BOARD_POSITIONS>& PreGen::knightAttacks() const
{
	return m_knightAttackMasks;
}

const std::array<BitBoard, MAX_BOARD_POSITIONS>& PreGen::kingAttacks() const
{
	return m_kingAttackMasks;
}

const std::array<std::array<BitBoard, MAX_BISHOP_ATTACKS>, MAX_BOARD_POSITIONS>& PreGen::bishopAttacks() const
{
	return m_bishopAttackMask;
}

const std::array<std::array<BitBoard, MAX_ROOK_ATTACKS>, MAX_BOARD_POSITIONS>& PreGen::rookAttacks() const
{
	return m_rookAttackMask;
}

const std::array<std::uint64_t, MAX_BOARD_POSITIONS>& PreGen::bishopMagics() const
{
	return m_bishopMagics;
}

const std::array<std::uint64_t, MAX_BOARD_POSITIONS>& PreGen::rookMagics() const
{
	return m_rookMagics;
}

const std::array<std::size_t, MAX_BOARD_POSITIONS>& PreGen::bishopBitCount() const
{
	return m_bishopBitCount;
}

const std::array<std::size_t, MAX_BOARD_POSITIONS>& PreGen::rookBitCount() const
{
	return m_rookBitCount;
}



// Non Slider Piece Creation
void PreGen::createPawnAttackMasks()
{
	//white
	for (std::size_t r{}; r < RANK_MAX; r++)
	{
		for (std::size_t f{}; f < FILE_MAX; f++)
		{
			m_pawnAttackMasks[Color::WHITE][indexAttackTable(r, f)].set_rf_safe(r - 1, f + 1);
			m_pawnAttackMasks[Color::WHITE][indexAttackTable(r, f)].set_rf_safe(r - 1, f - 1);
		}
	}

	//black
	for (std::size_t r{}; r < RANK_MAX; r++)
	{
		for (std::size_t f{}; f < FILE_MAX; f++)
		{
			m_pawnAttackMasks[Color::BLACK][indexAttackTable(r, f)].set_rf_safe(r + 1, f + 1);
			m_pawnAttackMasks[Color::BLACK][indexAttackTable(r, f)].set_rf_safe(r + 1, f - 1);
		}
	}
}

void PreGen::createKnightAttackMasks()
{
	for (std::size_t r{}; r < RANK_MAX; r++)
	{
		for (std::size_t f{}; f < FILE_MAX; f++)
		{
			//NORTH
			m_knightAttackMasks[indexAttackTable(r, f)].set_rf_safe(r + 2, f + 1);
			m_knightAttackMasks[indexAttackTable(r, f)].set_rf_safe(r + 2, f - 1);

			//EAST		  
			m_knightAttackMasks[indexAttackTable(r, f)].set_rf_safe(r + 1, f + 2);
			m_knightAttackMasks[indexAttackTable(r, f)].set_rf_safe(r - 1, f + 2);

			//SOUTH		  
			m_knightAttackMasks[indexAttackTable(r, f)].set_rf_safe(r - 2, f + 1);
			m_knightAttackMasks[indexAttackTable(r, f)].set_rf_safe(r - 2, f - 1);

			//WEST		  
			m_knightAttackMasks[indexAttackTable(r, f)].set_rf_safe(r + 1, f - 2);
			m_knightAttackMasks[indexAttackTable(r, f)].set_rf_safe(r - 1, f - 2);
		}
	}
}

void PreGen::createKingAttackMasks()
{
	for (std::size_t r{}; r < RANK_MAX; r++)
	{
		for (std::size_t f{}; f < FILE_MAX; f++)
		{
			m_kingAttackMasks[indexAttackTable(r, f)].set_rf_safe(r + 1, f); //N
			m_kingAttackMasks[indexAttackTable(r, f)].set_rf_safe(r + 1, f + 1); //NE
			m_kingAttackMasks[indexAttackTable(r, f)].set_rf_safe(r, f + 1); //E
			m_kingAttackMasks[indexAttackTable(r, f)].set_rf_safe(r - 1, f + 1); //SE
			m_kingAttackMasks[indexAttackTable(r, f)].set_rf_safe(r - 1, f); //S
			m_kingAttackMasks[indexAttackTable(r, f)].set_rf_safe(r - 1, f - 1); //SW
			m_kingAttackMasks[indexAttackTable(r, f)].set_rf_safe(r, f - 1); //W
			m_kingAttackMasks[indexAttackTable(r, f)].set_rf_safe(r + 1, f - 1); //NW
		}
	}
}

void PreGen::createEdgeMasks()
{
	for (size_t r{}; r < RANK_MAX; r++)
	{
		for (size_t f{}; f < FILE_MAX; f++)
		{
			//north
			if (r == 0)
			{
				m_northEdgeMask.set_rf_safe(r, f);
			}

			//east
			if (f == 7)
			{
				m_eastEdgeMask.set_rf_safe(r, f);
			}

			//south
			if (r == 7)
			{
				m_southEdgeMask.set_rf_safe(r, f);
			}

			//west
			if (f == 0)
			{
				m_westEdgeMask.set_rf_safe(r, f);
			}
		}
	}
}



// Slider Piece Creation
void PreGen::createBishopAttackMasks()
{
	for (std::size_t tr{}; tr < RANK_MAX; tr++)
	{
		for (std::size_t tf{}; tf < FILE_MAX; tf++)
		{
			std::int64_t r{};
			std::int64_t f{};

			for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) m_bishopRelevantBits[indexAttackTable(tr, tf)].set_rf(r, f);
			for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) m_bishopRelevantBits[indexAttackTable(tr, tf)].set_rf(r, f);
			for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) m_bishopRelevantBits[indexAttackTable(tr, tf)].set_rf(r, f);
			for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) m_bishopRelevantBits[indexAttackTable(tr, tf)].set_rf(r, f);
		}
	}
}

void PreGen::createRookAttackMasks()
{
	for (std::size_t tr{}; tr < RANK_MAX; tr++)
	{
		for (std::size_t tf{}; tf < FILE_MAX; tf++)
		{
			std::int64_t r{};
			std::int64_t f{};

			for (r = tr + 1; r <= 6; r++) m_rookRelevantBits[indexAttackTable(tr, tf)].set_rf(r, tf);
			for (r = tr - 1; r >= 1; r--) m_rookRelevantBits[indexAttackTable(tr, tf)].set_rf(r, tf);
			for (f = tf + 1; f <= 6; f++) m_rookRelevantBits[indexAttackTable(tr, tf)].set_rf(tr, f);
			for (f = tf - 1; f >= 1; f--) m_rookRelevantBits[indexAttackTable(tr, tf)].set_rf(tr, f);
		}
	}
}

void PreGen::createBishopBitCount()
{
	for (std::size_t i{}; i < MAX_BOARD_POSITIONS; i++)
	{
		m_bishopBitCount[i] = (__popcnt64(m_bishopRelevantBits[i].board()));
	}
}

void PreGen::createRookBitCount()
{
	for (std::size_t i{}; i < MAX_BOARD_POSITIONS; i++)
	{
		m_rookBitCount[i] = (__popcnt64(m_rookRelevantBits[i].board()));
	}
}

void PreGen::createBishopAttacks()
{
	for (std::size_t i{}; i < MAX_BOARD_POSITIONS; i++)
	{
		const std::size_t magic_number{ m_bishopMagics[i] };

		const std::vector<BitBoard> occupancies{ createOccupancies(m_bishopRelevantBits[i].board(), m_bishopBitCount[i], single_bit << m_bishopBitCount[i]) };

		for (std::size_t j{}; j < occupancies.size(); j++)
		{
			const BitBoard attack{ createSliderAttack<Piece::BISHOP>(i, occupancies[j]) };

			const std::size_t magic_index{ (occupancies[j].board() * magic_number) >> (MAX_BOARD_POSITIONS - m_bishopBitCount[i]) };

			m_bishopAttackMask[i][magic_index] = attack;
		}
	}
}

void PreGen::createRookAttacks()
{
	for (std::size_t i{}; i < MAX_BOARD_POSITIONS; i++)
	{
		const std::size_t magic_number{ m_rookMagics[i] };

		const std::vector<BitBoard> occupancies{ createOccupancies(m_rookRelevantBits[i].board(), m_rookBitCount[i], single_bit << m_rookBitCount[i]) };

		for (std::size_t j{}; j < occupancies.size(); j++)
		{
			const BitBoard attack{ createSliderAttack<Piece::ROOK>(i, occupancies[j]) };

			const std::size_t magic_index{ (occupancies[j].board() * magic_number) >> (MAX_BOARD_POSITIONS - m_rookBitCount[i]) };

			m_rookAttackMask[i][magic_index] = attack;
		}
	}
}



// Magic Numbers
void PreGen::InitMagicNumbers()
{
	if constexpr (USING_PREGENERATED_MAGICS)
	{
		std::cout << "Using Pregenerated Magics" << std::endl;

		m_bishopMagics = pregen::bishop_magics;
		m_rookMagics = pregen::rook_magics;
	}
	else
	{
		std::cout << "Not Using Pregenerated Magics" << std::endl;

		createBishopMagics();
		createRookMagics();
	}

	if constexpr (PRINT_GENERATED_MAGICS)
	{
		std::cout << std::endl << "Bishop Magics:" << std::endl;
		for (std::size_t magic : m_bishopMagics)
		{
			std::cout << magic << "," << std::endl;
		}

		std::cout << std::endl << "Rook Magics:" << std::endl;
		for (std::size_t magic : m_rookMagics)
		{
			std::cout << magic << "," << std::endl;
		}
	}
}

void PreGen::createBishopMagics()
{
	std::cout << "Generating Bishop Magics";

	for (std::size_t i{}; i < MAX_BOARD_POSITIONS; i++)
	{
		m_bishopMagics[i] = (findMagicNumber(i, Piece::BISHOP));
	}

	std::cout << " Done" << std::endl;
}

void PreGen::createRookMagics()
{
	std::cout << "Generating Rook Magics";

	for (std::size_t i{}; i < MAX_BOARD_POSITIONS; i++)
	{
		m_rookMagics[i] = (findMagicNumber(i, Piece::ROOK));
	}

	std::cout << " Done" << std::endl;
}

std::vector<BitBoard> PreGen::createOccupancies(const std::uint64_t mask, const std::size_t num_bits, const std::size_t num_occupancies)
{
	std::vector<BitBoard> occupancies;
	occupancies.reserve(num_occupancies); // Pre-allocate memory for all occupancies

	for (std::uint64_t i{}; i < num_occupancies; i++)
	{
		BitBoard occupancy{};
		std::size_t bit_position{}; // Position of set bits in the mask

		// Iterate over each square on the board
		for (std::size_t square{}; square < MAX_BOARD_POSITIONS; square++)
		{
			// Check if the current square is in the mask
			if (mask & (single_bit << square))
			{
				// Check if the current bit in 'i' is set and set the corresponding square in occupancy
				if (i & (single_bit << bit_position))
				{
					occupancy.set(square); // Set the corresponding square in the occupancy BitBoard
				}
				bit_position++;
			}
		}

		occupancies.push_back(occupancy);
	}

	return occupancies;
}

std::uint64_t PreGen::findMagicNumber(const std::size_t square, const Piece piece)
{
	Random rnd;

	const std::uint64_t mask{ piece == Piece::BISHOP ? m_bishopRelevantBits[square].board() : m_rookRelevantBits[square].board() };

	const std::size_t num_bits{ static_cast<std::size_t>(__popcnt64(mask)) };
	const std::size_t num_occupancies{ single_bit << num_bits };

	const std::vector<BitBoard> occupancies{ createOccupancies(mask, num_bits, num_occupancies) };
	
	while (true)
	{
		const std::uint64_t magic{ rnd.next_uint64_minbits() };

		//skip inappropriate magic numbers
		/*if (__popcnt64((mask * magic) & 0xFF00000000000000ULL) < 6)
		{
			continue;
		}*/

		bool bad_magic{ false };
		std::bitset<MAX_ROOK_ATTACKS> index_tracker;

		for (auto& occupancy : occupancies)
		{
			const std::size_t index{ static_cast<std::size_t>((occupancy.board() * magic) >> (MAX_BOARD_POSITIONS - num_bits)) };

			//check if index exists
			if (!index_tracker.test(index))
			{
				index_tracker.set(index);
			}
			else
			{
				bad_magic = true;
				break;
			}
		}

		if (!bad_magic)
		{
			return magic;
		}
	}
}

template<Piece P>
BitBoard PreGen::createSliderAttack(const std::size_t square, const BitBoard occupancy)
{
	BitBoard mask;

	int tr{ static_cast<int>(square / 8) };
	int tf{ static_cast<int>(square % 8) };

	int r{};
	int f{};

	if constexpr (P == Piece::BISHOP)
	{
		for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
		{
			mask.set_rf(r, f);

			if (occupancy.test_rf(r, f))
			{
				break;
			}
		}

		for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
		{
			mask.set_rf(r, f);

			if (occupancy.test_rf(r, f))
			{
				break;
			}
		}

		for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
		{
			mask.set_rf(r, f);

			if (occupancy.test_rf(r, f))
			{
				break;
			}
		}
		for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
		{
			mask.set_rf(r, f);

			if (occupancy.test_rf(r, f))
			{
				break;
			}
		}
	}
	else
	{
		for (r = tr + 1; r <= 7; r++)
		{
			mask.set_rf(r, tf);

			if (occupancy.test_rf(r, tf))
			{
				break;
			}
		}

		for (r = tr - 1; r >= 0; r--)
		{
			mask.set_rf(r, tf);

			if (occupancy.test_rf(r, tf))
			{
				break;
			}
		}

		for (f = tf + 1; f <= 7; f++)
		{
			mask.set_rf(tr, f);

			if (occupancy.test_rf(tr, f))
			{
				break;
			}
		}

		for (f = tf - 1; f >= 0; f--)
		{
			mask.set_rf(tr, f);

			if (occupancy.test_rf(tr, f))
			{
				break;
			}
		}
	}

	return mask;
}



// Helper Functions
void PreGen::printMasks(const Piece P) const
{
	if (P == Piece::KNIGHT)
	{
		int count{};
		for (auto &b : m_knightAttackMasks)
		{
			std::cout << "==========[" << count << "]==========" << std::endl;
			b.print();
			count++;
		}
	}
	else if (P == Piece::BISHOP)
	{
		int count{};
		for (auto &b : m_bishopRelevantBits)
		{
			std::cout << "==========[" << count << "]==========" << std::endl;
			b.print();
			count++;
		}
	}
	else if (P == Piece::ROOK)
	{
		int count{};
		for (auto &b : m_rookRelevantBits)
		{
			std::cout << "==========[" << count << "]==========" << std::endl;
			b.print();
			count++;
		}
	}
	else if (P == Piece::QUEEN)
	{
		std::cout << "broken" << std::endl;
	}
	else if (P == Piece::KING)
	{
		int count{};
		for (auto &b : m_kingAttackMasks)
		{
			std::cout << "==========[" << count << "]==========" << std::endl;
			b.print();
			count++;
		}
	}
}

std::size_t PreGen::indexAttackTable(const std::size_t rank, const std::size_t file) 
{
	const std::size_t index{ rank * RANK_MAX + file };
	return index;
}
