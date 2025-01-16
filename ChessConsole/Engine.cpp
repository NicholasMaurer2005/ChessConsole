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
	return 0;
}

int Engine::negamax(State& state, const std::uint32_t depth, int alpha, int beta) //alpha = INT_MIN, beta = INT_MAX in initial call
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
		if (kingInCheck(state, state.whiteToMove()))
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
		//try to make move and get find highest score
		if (makeLegal(state, move))
		{
			//flip side to move
			state.flipSide();

			//find the best score for all the branches of this move
			int score{ -negamax(state, depth - 1, -beta, -alpha) };
			highest_score = std::max(highest_score, score);

			//reset state
			state.flipSide();
			state.unmakeMove(move);

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
	m_state.printBoard(flip_board, no_sqr);
	m_depth = depth;

	while (true)
	{
		const auto start_time = std::chrono::high_resolution_clock::now();

		if (m_state.whiteToMove() == engine_side_white)//TODO: clean up
		{
			if constexpr (PLAYER_PLAY_ITSELF)
			{
				//player move
				MoveList list;
				m_moveGen.generateMoves(m_state, list);

				Move move;

				while (true)
				{
					if (inputAndParseMove(list, move))
					{
						if (makeLegal(m_state, move))
						{
							move.print();
							m_bestMove = move;
							break;
						}
					}

					std::cout << "move does not exist" << std::endl;
				}
			}
			else
			{
				//engine move
				std::cout << "thinking" << std::endl;
				negamax(m_state, m_depth, INT_MIN, INT_MAX);
				makeLegal(m_state, m_bestMove);
			}
		}
		else
		{
			if (ENGINE_PLAY_ITSELF)
			{ 
				//engine move
				std::cout << "thinking" << std::endl;
				negamax(m_state, m_depth, INT_MIN, INT_MAX);
				makeLegal(m_state, m_bestMove);
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
						if (makeLegal(m_state, move))
						{
							move.print();
							m_bestMove = move;
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
		m_state.printBoard(flip_board, no_sqr);

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

bool Engine::kingInCheck(const State& state, const bool whiteSide) const
{
	if (whiteSide)
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
	using namespace std::literals::string_literals;

	std::string input;
	std::getline(std::cin, input);
	//std::transform(input.begin(), input.end(), input.begin(), ::tolower);


	if (input == "tb"s)
	{
		m_state.unmakeMove(m_bestMove);
	}
	else if (input == "wk"s)
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

bool Engine::makeLegal(State& state, const Move move) const
{
	state.makeMove(move);

	if (kingInCheck(state, state.whiteToMove()))
	{
		state.unmakeMove(move);
		return false;
	}
	else
	{
		return true;
	}
}











