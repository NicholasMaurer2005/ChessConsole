#include "MoveGen.h"



MoveGen::MoveGen()
	: m_preGen() {}


void MoveGen::generateMoves(const State& state, MoveList& moveList)
{
	BitBoard board;

	for (std::size_t piece{}; piece < PIECE_COUNT; piece++)
	{
		//copy
		board.setBoard(state.positions()[piece].board());

		if (state.whiteToMove())
		{
			//white pawn
			if (piece == Piece::PAWN)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					const int target_square{ static_cast<int>(source_square) - 8 };

					//white pawn quiet
					if (target_square > 0 && !state.occupancy()[Occupancy::BOTH].test(target_square))
					{
						//promotion
						if (source_square >= a7 && source_square <= h7)
						{
							moveList.addMove<MoveType::QUIET_PROMOTE, Piece::QUEEN>(source_square, target_square, Piece::NO_PIECE);
							moveList.addMove<MoveType::QUIET_PROMOTE, Piece::ROOK>(source_square, target_square, Piece::NO_PIECE);
							moveList.addMove<MoveType::QUIET_PROMOTE, Piece::BISHOP>(source_square, target_square, Piece::NO_PIECE);
							moveList.addMove<MoveType::QUIET_PROMOTE, Piece::KNIGHT>(source_square, target_square, Piece::NO_PIECE);
						}
						else
						{
							//one square
							moveList.addMove<MoveType::QUIET, Piece::PAWN>(source_square, target_square, Piece::NO_PIECE);

							//two square
							if ((source_square >= a2 && source_square <= h2) && !state.occupancy()[Occupancy::BOTH].test(target_square - 8))
							{
								moveList.addMove<MoveType::DOUBLE_PAWN, Piece::PAWN>(source_square, target_square - 8, Piece::NO_PIECE);
							}
						}
					}

					//white pawn captures
					BitBoard attacks{ m_preGen.pawnAttacks()[Color::WHITE][source_square].board() & state.occupancy()[Occupancy::BLACKOCC].board() };

					while (attacks.board())
					{
						const std::size_t attack_target{ attacks.find_1lsb() };
						const Piece target_piece{ state.testPieceType(attack_target) };

						if (source_square >= a7 && source_square <= h7)
						{
							//promotion
							moveList.addMove<MoveType::PROMOTE, Piece::QUEEN>(source_square, attack_target, target_piece);
							moveList.addMove<MoveType::PROMOTE, Piece::ROOK>(source_square, attack_target, target_piece);
							moveList.addMove<MoveType::PROMOTE, Piece::BISHOP>(source_square, attack_target, target_piece);
							moveList.addMove<MoveType::PROMOTE, Piece::KNIGHT>(source_square, attack_target, target_piece);

						}
						else
						{

							//non promote
							moveList.addMove<MoveType::CAPTURE, Piece::PAWN>(source_square, attack_target, target_piece);

						}
						
						attacks.reset(attack_target);
					}

					//enpessant
					if (state.enpassantSquare() != no_sqr)
					{
						const BitBoard pawn_attack{ m_preGen.pawnAttacks()[Color::WHITE][source_square] };
						const bool enpassant_attack{ pawn_attack.test(state.enpassantSquare()) };

						if (enpassant_attack)
						{
							moveList.addMove<MoveType::ENPASSANT, Piece::PAWN>(source_square, state.enpassantSquare(), Piece::PAWN);
						}
					}

					board.reset(source_square);
				}
			}

			//white knight
			if (piece == Piece::KNIGHT)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					BitBoard knight_attacks{ m_preGen.knightAttacks()[source_square].board() & ~state.occupancy()[Color::WHITE].board() };

					while (knight_attacks.board())
					{
						const std::size_t target_square{ knight_attacks.find_1lsb() };

						if (state.occupancy()[Color::BLACK].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::KNIGHT>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::KNIGHT>(source_square, target_square, Piece::NO_PIECE);
						}

						knight_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//white bishop
			if (piece == Piece::BISHOP)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					BitBoard bishop_attacks{ getBishopAttack(source_square, state.occupancy()[Occupancy::BOTH]).board() & ~state.occupancy()[Color::WHITE].board()};

					while (bishop_attacks.board())
					{
						const std::size_t target_square{ bishop_attacks.find_1lsb() };

						if (state.occupancy()[Color::BLACK].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::BISHOP>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::BISHOP>(source_square, target_square, Piece::NO_PIECE);
						}

						bishop_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//white rook
			if (piece == Piece::ROOK)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					BitBoard rook_attacks{ getRookAttack(source_square, state.occupancy()[Occupancy::BOTH]).board() & ~state.occupancy()[Color::WHITE].board() };

					while (rook_attacks.board())
					{
						const std::size_t target_square{ rook_attacks.find_1lsb() };


						if (state.occupancy()[Color::BLACK].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::ROOK>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::ROOK>(source_square, target_square, Piece::NO_PIECE);
						}

						rook_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//white queen
			if (piece == Piece::QUEEN)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					const BitBoard rook_attacks{ getRookAttack(source_square, state.occupancy()[Occupancy::BOTH]).board() };
					const BitBoard bishop_attacks{ getBishopAttack(source_square, state.occupancy()[Occupancy::BOTH]).board() };
					BitBoard queen_attacks{ (rook_attacks.board() | bishop_attacks.board()) & ~state.occupancy()[Color::WHITE].board() };

					while (queen_attacks.board())
					{
						const std::size_t target_square{ queen_attacks.find_1lsb() };


						if (state.occupancy()[Color::BLACK].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::QUEEN>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::QUEEN>(source_square, target_square, Piece::NO_PIECE);
						}

						queen_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//white king
			if (piece == Piece::KING)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					BitBoard king_attacks{ m_preGen.kingAttacks()[source_square].board() & ~state.occupancy()[Color::WHITE].board() };

					while (king_attacks.board())
					{
						const std::size_t target_square{ king_attacks.find_1lsb() };


						if (state.occupancy()[Color::BLACK].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::KING>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::KING>(source_square, target_square, Piece::NO_PIECE);
						}

						king_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//white castle
			if (piece == Piece::KING)
			{
				const BitBoard occupancy{ state.occupancy()[Occupancy::BOTH] };

				//white king side
				if (state.testCastleRights(Castle::WK))
				{
					//path is clear
					if (!occupancy.test(f1) && !occupancy.test(g1))
					{
						//not under attack
						if (!isSquareAttacked(state, e1, Color::WHITE) && !isSquareAttacked(state, f1, Color::WHITE))
						{
							moveList.addCastleMove<Castle::WK>();
						}
					}
				}

				//white queen side
				if (state.testCastleRights(Castle::WQ))
				{
					//path is clear
					if (!occupancy.test(d1) && !occupancy.test(c1) && !occupancy.test(b1))
					{
						//not under attack
						if (!isSquareAttacked(state, e1, Color::WHITE) && !isSquareAttacked(state, c1, Color::WHITE))
						{
							moveList.addCastleMove<Castle::WQ>();
						}
					}
				}
			}
		}
		else
		{
			//black pawn 
			if (piece == Piece::BPAWN)
			{
				while (board.board())
				{

					const std::size_t source_square = board.find_1lsb();
					const int target_square{ static_cast<int>(source_square) + 8 };

					//black pawn quite
					if (target_square < 64 && !state.occupancy()[Occupancy::BOTH].test(target_square))
					{
						//black pawn promotion
						if (source_square >= a2 && source_square <= h2)
						{
						
							if ((source_square >= a7 && source_square <= h7) && !state.occupancy()[Occupancy::BOTH].test(target_square + 8))
							{
								moveList.addMove<QUIET_PROMOTE, BQUEEN>(source_square, target_square, Piece::NO_PIECE);
								moveList.addMove<QUIET_PROMOTE, BROOK>(source_square, target_square, Piece::NO_PIECE);
								moveList.addMove<QUIET_PROMOTE, BBISHOP>(source_square, target_square, Piece::NO_PIECE);
								moveList.addMove<QUIET_PROMOTE, BKNIGHT>(source_square, target_square, Piece::NO_PIECE);
							}
						}
						else
						{
							//one square
							moveList.addMove<QUIET, BPAWN>(source_square, target_square, Piece::NO_PIECE);

							//two square
							if ((source_square >= a7 && source_square <= h7) && !state.occupancy()[Occupancy::BOTH].test(target_square + 8))
							{
								moveList.addMove<MoveType::DOUBLE_PAWN, Piece::BPAWN>(source_square, target_square + 8, Piece::NO_PIECE);//TODO: maybe switch to variable because add twice
							}
						}
					}

					//black pawn captures
					BitBoard attacks{ m_preGen.pawnAttacks()[Color::BLACK][source_square].board() & state.occupancy()[Occupancy::WHITEOCC].board() };

					while (attacks.board())
					{
						const std::size_t attack_target{ attacks.find_1lsb() };
						const Piece target_piece{ state.testPieceType(attack_target) };

						//promotion
						if (source_square >= a2 && source_square <= h2)
						{
							moveList.addMove<MoveType::PROMOTE, Piece::BQUEEN>(source_square, attack_target, target_piece);
							moveList.addMove<MoveType::PROMOTE, Piece::BROOK>(source_square, attack_target, target_piece);
							moveList.addMove<MoveType::PROMOTE, Piece::BBISHOP>(source_square, attack_target, target_piece);
							moveList.addMove<MoveType::PROMOTE, Piece::BKNIGHT>(source_square, attack_target, target_piece);
						}
						else
						{
							//one square
							moveList.addMove<MoveType::CAPTURE, Piece::BPAWN>(source_square, attack_target, target_piece);
						}
						
						attacks.reset(attack_target);
					}

					//enpessant
					if (state.enpassantSquare() != no_sqr)
					{
						const BitBoard pawn_attack{ m_preGen.pawnAttacks()[Color::BLACK][source_square] };
						const bool enpassant_attack{ pawn_attack.test(state.enpassantSquare()) };

						if (enpassant_attack)
						{
							moveList.addMove<MoveType::ENPASSANT, Piece::BPAWN>(source_square, state.enpassantSquare(), Piece::PAWN);
						}
					}

					board.reset(source_square);
				}
			}

			//black knight
			if (piece == Piece::BKNIGHT)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					BitBoard knight_attacks{ m_preGen.knightAttacks()[source_square].board() & ~state.occupancy()[Color::BLACK].board() };

					while (knight_attacks.board())
					{
						const std::size_t target_square{ knight_attacks.find_1lsb() };
						const Piece target_piece{ state.testPieceType(target_square) };

						if (state.occupancy()[Color::WHITE].test(target_square))
						{
							//captures
							moveList.addMove<MoveType::CAPTURE, Piece::BKNIGHT>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::BKNIGHT>(source_square, target_square, target_piece);
						}

						knight_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//black bishop
			if (piece == Piece::BBISHOP)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					BitBoard bishop_attacks{ getBishopAttack(source_square, state.occupancy()[Occupancy::BOTH]).board() & ~state.occupancy()[Color::BLACK].board() };

					while (bishop_attacks.board())
					{
						const std::size_t target_square{ bishop_attacks.find_1lsb() };

						if (state.occupancy()[Color::WHITE].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::BBISHOP>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::BBISHOP>(source_square, target_square, Piece::NO_PIECE);
						}

						bishop_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//black rook
			if (piece == Piece::BROOK)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					BitBoard rook_attacks{ getRookAttack(source_square, state.occupancy()[Occupancy::BOTH]).board() & ~state.occupancy()[Color::BLACK].board() };

					while (rook_attacks.board())
					{
						const std::size_t target_square{ rook_attacks.find_1lsb() };

						if (state.occupancy()[Color::WHITE].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::BROOK>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::BROOK>(source_square, target_square, Piece::NO_PIECE);
						}

						rook_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//black queen
			if (piece == Piece::BQUEEN)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					const BitBoard rook_attacks{ getRookAttack(source_square, state.occupancy()[Occupancy::BOTH]).board() };
					const BitBoard bishop_attacks{ getBishopAttack(source_square, state.occupancy()[Occupancy::BOTH]).board() };
					BitBoard queen_attacks{ (rook_attacks.board() | bishop_attacks.board()) & ~state.occupancy()[Color::BLACK].board() };

					while (queen_attacks.board())
					{
						const std::size_t target_square{ queen_attacks.find_1lsb() };

						if (state.occupancy()[Color::WHITE].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::BQUEEN>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::BQUEEN>(source_square, target_square, Piece::NO_PIECE);
						}

						queen_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//black king
			if (piece == Piece::BKING)
			{
				while (board.board())
				{
					const std::size_t source_square{ board.find_1lsb() };
					BitBoard king_attacks{ m_preGen.kingAttacks()[source_square].board() & ~state.occupancy()[Color::BLACK].board() };

					while (king_attacks.board())
					{
						const std::size_t target_square{ king_attacks.find_1lsb() };

						if (state.occupancy()[Color::WHITE].test(target_square))
						{
							//captures
							const Piece target_piece{ state.testPieceType(target_square) };
							moveList.addMove<MoveType::CAPTURE, Piece::BKING>(source_square, target_square, target_piece);
						}
						else
						{
							//quite
							moveList.addMove<MoveType::QUIET, Piece::BKING>(source_square, target_square, Piece::NO_PIECE);
						}

						king_attacks.reset(target_square);
					}

					board.reset(source_square);
				}
			}

			//black castle
			if (piece == Piece::BKING)
			{
				const BitBoard occupancy = state.occupancy()[Occupancy::BOTH];

				//black king side
				if (state.testCastleRights(Castle::BK))
				{
					//path is clear
					if (!occupancy.test(f8) && !occupancy.test(g8))
					{
						//not under attack
						if (!isSquareAttacked(state, e8, Color::BLACK) && !isSquareAttacked(state, f8, Color::BLACK))
						{
							moveList.addCastleMove<Castle::BK>();
						}
					}
				}

				//black queen side
				if (state.testCastleRights(Castle::BQ))
				{
					//path is clear
					if (!occupancy.test(d8) && !occupancy.test(c8) && !occupancy.test(b8))
					{
						//not under attack
						if (!isSquareAttacked(state, e8, Color::BLACK) && !isSquareAttacked(state, c8, Color::BLACK))
						{
							moveList.addCastleMove<Castle::BQ>();
						}
					}
				}
			}
		}
	}
}

//color represents defending side
bool MoveGen::isSquareAttacked(const State& state, const std::size_t square, const Color side) const
{
	//white pawn
	const BitBoard pawn_attack{ m_preGen.pawnAttacks()[Color::BLACK][square] };
	const BitBoard pawn{ state.positions()[Piece::PAWN] };
	if ((side == Color::BLACK) && (pawn_attack.board() & pawn.board()))
	{
		return true;
	}

	//black pawn
	const BitBoard bpawn_attack{ m_preGen.pawnAttacks()[Color::WHITE][square] };
	const BitBoard bpawn{ state.positions()[Piece::BPAWN] };
	if ((side == Color::WHITE) && (bpawn_attack.board() & bpawn.board()))
	{
		return true;
	}

	//knight
	const BitBoard knight_attack{ m_preGen.knightAttacks()[square] };
	const BitBoard knight{ state.positions()[side == Color::WHITE ? Piece::BKNIGHT : Piece::KNIGHT] };
	if (knight_attack.board() & knight.board())
	{
		return true;
	}

	//king
	const BitBoard king_attack{ m_preGen.kingAttacks()[square] };
	const BitBoard king{ state.positions()[side == Color::WHITE ? Piece::BKING : Piece::KING] };
	if (king_attack.board() & king.board())
	{
		return true;
	}

	//bishop
	const BitBoard bishop_attack{ getBishopAttack(square, state.occupancy()[Occupancy::BOTH])};
	const BitBoard bishop{ state.positions()[side == Color::WHITE ? Piece::BBISHOP : Piece::BISHOP] };
	if (bishop_attack.board() & bishop.board())
	{
		return true;
	}

	//rook
	const BitBoard rook_attack{ getRookAttack(square, state.occupancy()[Occupancy::BOTH]) };
	const BitBoard rook{ state.positions()[side == Color::WHITE ? Piece::BROOK : Piece::ROOK] };
	if (rook_attack.board() & rook.board())
	{
		return true;
	}

	//queen
	const BitBoard queen_attack_b{ getBishopAttack(square, state.occupancy()[Occupancy::BOTH]) };
	const BitBoard queen_attack_r{ getRookAttack(square, state.occupancy()[Occupancy::BOTH]) };
	const BitBoard queen_attack{ queen_attack_b.board() | queen_attack_r.board() };
	const BitBoard queen{ state.positions()[side == Color::WHITE ? Piece::BQUEEN : Piece::QUEEN] };
	if (queen_attack.board() & queen.board())
	{
		return true;
	}

	return false;
}

BitBoard MoveGen::getBishopAttack(const std::size_t square, const BitBoard occupancy) const
{
	const std::size_t magic_number = static_cast<size_t>(m_preGen.bishopMagics()[square]);
	const std::uint64_t masked_occupancy = m_preGen.maskRelevantBits<Piece::BISHOP>(occupancy, square);
	const std::size_t magic_index = (masked_occupancy * magic_number) >> (MAX_BOARD_POSITIONS - m_preGen.bishopBitCount()[square]);

	return m_preGen.bishopAttacks()[square][magic_index];
}

BitBoard MoveGen::getRookAttack(const std::size_t square, const BitBoard occupancy) const
{
	const std::size_t magic_number = static_cast<size_t>(m_preGen.rookMagics()[square]);
	const std::uint64_t masked_occupancy = m_preGen.maskRelevantBits<Piece::ROOK>(occupancy, square);
	const std::size_t magic_index = (masked_occupancy * magic_number) >> (MAX_BOARD_POSITIONS - m_preGen.rookBitCount()[square]);

	return m_preGen.rookAttacks()[square][magic_index];
}

template<Color C>
BitBoard MoveGen::getPawnAttack(const std::size_t square) const
{
	return m_preGen.pawnAttacks()[C][square];
}

BitBoard MoveGen::getKnightAttack(const std::size_t square) const
{
	return m_preGen.knightAttacks()[square];
}

BitBoard MoveGen::getKingAttack(const std::size_t square) const
{
	return m_preGen.kingAttacks()[square];
}

BitBoard MoveGen::getPieceAttack(const std::size_t P, std::size_t square, const State& state)
{
	square = square % 6;

	if (square == 0)
	{
		return m_preGen.pawnAttacks()[state.whiteToMove() ? 0 : 1][square];
	}
	else if (square == 1)
	{
		return m_preGen.knightAttacks()[square];
	}
	else if (square == 2)
	{
		return m_preGen.kingAttacks()[square];
	}
	else if (square == 3)
	{
		return getBishopAttack(square, state.occupancy()[Occupancy::BOTH]);
	}
	else if (square == 4)
	{
		return getRookAttack(square, state.occupancy()[Occupancy::BOTH]);
	}
	else if (square == 5)
	{
		const BitBoard bishop_attack = getBishopAttack(square, state.occupancy()[Occupancy::BOTH]);
		const BitBoard rook_attack = getRookAttack(square, state.occupancy()[Occupancy::BOTH]);

		return BitBoard{ bishop_attack.board() | rook_attack.board() };
	}
}