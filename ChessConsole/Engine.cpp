#include "Engine.h"

Engine::Engine()
	: m_moveGen(), m_state(), m_bestMove(), m_evaluations(), m_nodes(), m_prunes(), m_seconds(), m_mates(), m_depth() {}

Engine::Engine(std::string_view fen)
	: m_moveGen(), m_state(State::parse_fen(fen)), m_bestMove(), m_evaluations(), m_nodes(), m_prunes(), m_seconds(), m_mates(), m_depth() {}



void Engine::setState(std::string_view fen)
{
	m_state = State::parse_fen(fen);
}

int Engine::evaluate(const State& state)
{
	m_evaluations++;

	int evaluation{};

	for (std::size_t piece{}; piece < PIECE_COUNT; piece++)
	{
		BitBoard piece_board = state.positions()[piece];

		while (piece_board.board())
		{
			const std::size_t square = piece_board.find_1lsb();
			const BitBoard attack = m_moveGen.getPieceAttack(piece, square, state);

			evaluation += piece_value[piece];
			evaluation += static_cast<int>(attack.bitCount());

			piece_board.reset(square);
		}
	}


	return evaluation;
}

int queiscence(const State& state, int alpha, int beta)
{

}

int Engine::minimax(const State& state, const std::uint32_t depth, int alpha, int beta)
{
	m_nodes++;

	if (depth == 0)
	{
		return evaluate(state);
	}

	if (state.whiteToMove())
	{
		MoveList moves;
		m_moveGen.generateMoves(state, moves);
		moves.sortMoveList();

		int max_eval = INT_MIN;
		bool anyLegalMoves = false;

		for (Move move : moves.moves())
		{
			State new_state{ state };

			if (makeMove(move, new_state))
			{
				anyLegalMoves = true;

				new_state.flipSide();

				const int eval = minimax(new_state, depth - 1, alpha, beta);

				if (eval > max_eval)
				{
					max_eval = eval;

					if (depth == m_depth)
					{
						m_bestMove = move;
					}
				}

				if (alpha < eval)
				{
					alpha = eval;
				}

				if (beta <= alpha)
				{
					m_prunes++;
					break;
				}
			}
		}
		
		if (anyLegalMoves)
		{
			return max_eval;
		}
		else
		{
			if (kingInCheck(state))
			{
				//white checkmate
				m_mates++;
				return INT_MIN + depth + 1;
			}
			else
			{
				//white stalemate
				return 0;
			}
		}
	}
	else
	{
		MoveList moves;
		m_moveGen.generateMoves(state, moves);
		moves.sortMoveList();

		int min_eval = INT_MAX;
		bool anyLegalMoves = false;

		for (Move move : moves.moves())
		{
			State new_state{ state };

			if (makeMove(move, new_state))
			{
				anyLegalMoves = true;

				new_state.flipSide();
				const int eval = minimax(new_state, depth - 1, alpha, beta);

				if (eval < min_eval)
				{
					min_eval = eval;

					if (depth == m_depth)
					{
						m_bestMove = move;
					}
				}

				if (beta > eval)
				{
					beta = eval;
				}

				if (beta <= alpha)
				{
					m_prunes++;
					break;
				}
			}
		}
		if (anyLegalMoves)
		{
			return min_eval;
		}
		else
		{
			if (kingInCheck(state))
			{
				//black checkmate
				m_mates++;
				return INT_MAX - depth - 1;
			}
			else
			{
				//black stalemate
				return 0;
			}
		}
	}
}

int Engine::negamax(const State& state, const std::uint32_t depth, int alpha, int beta) //alpha = INT_MIN, beta = INT_MAX in initial call
{
	//time cutoff logic

	//leaf of search
	if (depth == 0)
	{
		return evaluate(state);
	}

	//find all pseudo moves for given state
	MoveList moves;
	m_moveGen.generateMoves(state, moves);
	moves.sortMoveList();

	//test for checkmate or stalemate
	if (moves.count() == 0)
	{
		if (kingInCheck(state))
		{
			//checkmate
			return CHECKMATE_SCORE;
		}
		else
		{
			//stalemate
			return 0;
		}
	}

	//find the highest score for all moves
	int highest_score{ INT_MIN };

	for (Move move : moves.moves())
	{
		//copy state to try to make move
		State new_state{ state };

		//try to make move and get find highest score
		if (makeMove(move, new_state))
		{
			//flip side to move
			new_state.flipSide();

			//find the best score for all the branches of this move
			int score{ -negamax(new_state, depth - 1, -beta, -alpha) };
			highest_score = std::max(highest_score, score);

			//alpha-beta pruning, stop searching if a better move is already guarenteed
			alpha = std::max(alpha, score);

			if (alpha >= beta)
			{
				break;
			}
		}
	}

	return highest_score;
}

void Engine::step(const bool engine_side_white, const bool flip_board, const std::uint32_t depth)
{
	m_state.printBoard(flip_board);
	m_depth = depth;

	while (true)
	{
		const auto start_time = std::chrono::high_resolution_clock::now();

		if (m_state.whiteToMove() == engine_side_white)
		{
			//engine move
			std::cout << "thinking" << std::endl;
			minimax(m_state, m_depth, INT_MIN, INT_MAX);
			makeMove(m_bestMove, m_state);
		}
		else
		{
			if constexpr (ENGINE_PLAY_ITSELF)
			{
				//engine move
				std::cout << "thinking" << std::endl;
				minimax(m_state, m_depth, INT_MIN, INT_MAX);
				makeMove(m_bestMove, m_state);
			}
			else
			{
				//player move
				MoveList list;
				m_moveGen.generateMoves(m_state, list);

				Move move;
				while (true)
				{
					if (inputAndParseMove(list, move))
					{
						State new_state{ m_state };
						new_state.printBoard(flip_board);
						move.print();

						if (makeMove(move, new_state))
						{
							m_state = new_state;
							break;
						}
					}

					std::cout << "move does not exist" << std::endl;
				}
			}
		}

		const auto end_time = std::chrono::high_resolution_clock::now();

		const std::chrono::duration<double> duration = end_time - start_time;
		system("cls");
		m_state.printBoard(flip_board);

		std::cout << "move: ";
		m_bestMove.print();

		std::cout << "nodes: " << m_nodes << std::endl;
		std::cout << "evaluations: " << m_evaluations << std::endl;
		std::cout << "prunes: " << m_prunes << std::endl;
		std::cout << "mates: " << m_mates << std::endl;
		std::cout << duration.count() << " seconds" << std::endl;

		m_nodes = 0;
		m_evaluations = 0;
		m_prunes = 0;
		m_mates = 0;

		m_state.flipSide();
	}
}

bool Engine::kingInCheck(const State& state) const
{
	if (state.whiteToMove())
	{
		const std::size_t king_square = state.positions()[KING].find_1lsb();

		if (king_square == SIZE_MAX)
		{
			return true;
		}
		else
		{
			return m_moveGen.isSquareAttacked(state, king_square, Color::WHITE);
		}
	}
	else
	{
		const std::size_t king_square = state.positions()[BKING].find_1lsb();

		if (king_square == SIZE_MAX)
		{
			return true;
		}
		else
		{
			return m_moveGen.isSquareAttacked(state, king_square, Color::BLACK);
		}
	}
}

void Engine::printBoard(const bool flipped) const
{
	m_state.printBoard(flipped, no_sqr);
}

void Engine::printAllBoardAttacks(Color C) const
{
	BitBoard b;
	for (int i = 0; i < 64; i++)
	{
		if (m_moveGen.isSquareAttacked(m_state, i, C))
		{
			b.set(i);
		}
	}
	b.print();
}

bool Engine::inputAndParseMove(MoveList& list, Move& move)
{
	std::string input;
	std::getline(std::cin, input);
	//std::transform(input.begin(), input.end(), input.begin(), ::tolower);

	if (input == "wk"s)
	{
		if (list.findCastleMove(g1))
		{
			move = Move::createCastleMove<Castle::WK>();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (input == "wq"s)
	{
		if (list.findCastleMove(c1))
		{
			move = Move::createCastleMove<Castle::WQ>();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (input == "bk"s)
	{
		if (list.findCastleMove(g8))
		{
			move = Move::createCastleMove<Castle::BK>();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (input == "bq"s)
	{
		if (list.findCastleMove(c8))
		{
			move = Move::createCastleMove<Castle::BQ>();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (input.length() == 4)
	{
		const std::string source{ input.substr(0, 2) };
		const std::string target{ input.substr(2, 2) };
		const std::size_t source_square{ Engine::squareToIndex(source) };
		const std::size_t target_square{ Engine::squareToIndex(target) };

		if (list.findMove(source_square, target_square, move))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

std::size_t Engine::squareToIndex(std::string_view square)
{
	const std::size_t rank{ 7 - static_cast<std::size_t>(square[1] - '1') };
	const std::size_t file{ static_cast<std::size_t>(square[0] - 'a') };

	return rank * RANK_MAX + file;
}

bool Engine::makeMove(const Move move, State& state) const
{
	state.setEnpassantSquare(no_sqr);

	//unpack
	const std::size_t source = move.source();
	const std::size_t target = move.target();
	const Piece piece = move.piece();
	const bool promoted = move.promoted();
	const bool capture = move.capture();
	const bool double_pawn = move.doublePawnPush();
	const bool enpassant = move.enpassant();
	const bool castle = move.castle();

	//if statements in most efficient order for least number of branching
	if (castle)
	{
		if (state.whiteToMove())
		{
			state.moveQuiet(KING, e1, source);

			if (source == g1)
			{
				state.moveQuiet(ROOK, h1, f1);
				state.setCastleRights(h1);
			}
			else
			{
				state.moveQuiet(ROOK, a1, d1);
				state.setCastleRights(a1);
			}
		}
		else
		{
			state.moveQuiet(BKING, e8, source);

			if (source == g8)
			{
				state.moveQuiet(BROOK, h8, f8);
				state.setCastleRights(h8);
			}
			else
			{
				state.moveQuiet(BROOK, a8, d8);
				state.setCastleRights(a8);
			}
		}
	}
	else
	{
		state.setCastleRights(source);
		state.setCastleRights(target);

		//captures
		if (capture)
		{
			if (promoted)
			{
				state.popPiece(state.whiteToMove() ? Piece::PAWN : Piece::BPAWN, source);
				state.popSquare(target);
				state.setPiece(piece, target);
			}
			else if (enpassant)
			{
				state.moveCapture(Piece::PAWN, source, target);
			}
			else
			{
				state.moveCapture(piece, source, target);
			}
		}
		//quiets
		else
		{
			if (double_pawn)
			{
				state.moveQuiet(state.whiteToMove() ? PAWN : BPAWN, source, target);
				state.setEnpassantSquare(state.whiteToMove() ? source - 8 : source + 8);
			}
			else if (promoted)
			{
				state.popPiece(state.whiteToMove() ? Piece::PAWN : Piece::BPAWN, source);
				state.setPiece(piece, target);
			}
			else
			{
				state.moveQuiet(piece, source, target);
			}
		}
	}

	if (kingInCheck(state))
	{
		return false;
	}
	else
	{
		return true;
	}
	
}











