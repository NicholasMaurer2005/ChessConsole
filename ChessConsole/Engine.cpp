#include "Engine.h"

Engine::Engine()
	: m_moveGen(), m_state(), m_bestMove(), m_evaluations(), m_nodes(), m_prunes(), m_seconds(), m_mates(), m_depth(), m_depthSearched(), m_stopSearch(), m_timeCheckCount(), 
	m_bestMoveFinal(), m_moveSource() {}

Engine::Engine(std::string_view fen)
	: m_moveGen(), m_state(State::parse_fen(fen)), m_bestMove(), m_evaluations(), m_nodes(), m_prunes(), m_seconds(), m_mates(), m_depth(), m_depthSearched(), m_stopSearch(), 
	m_timeCheckCount(), m_bestMoveFinal(), m_moveSource() {}



void Engine::setState(std::string_view fen)
{
	// m_state = State::parse_fen(fen); // Old version
	// New version to parse full FEN:
	std::vector<std::string> parts = State::split_fen(fen);
	m_state = State::parse_fen(parts[0]); // Piece placements

	if (parts.size() > 1 && !parts[1].empty()) {
		m_state.setWhiteToMove(parts[1] == "w");
	}
	if (parts.size() > 2) {
		m_state.setCastleRightsFromFen(parts[2]);
	}
	if (parts.size() > 3 && parts[3] != "-") {
		m_state.setEnpassantSquare(Engine::squareToIndex(parts[3]));
	} else {
		m_state.setEnpassantSquare(no_sqr);
	}
	// Ignoring halfmove (parts[4]) and fullmove (parts[5]) for now as engine doesn't use them.
}

// Helper to convert square index to algebraic notation like "e2", "h8"
// This must match the logic of squareToIndex: index = (7-rank)*8 + file
std::string indexToSquareString(std::size_t index) {
    if (index == no_sqr) return "-";
    std::size_t rank_from_bottom = index / 8; // 0 for 1st rank (a1-h1), 7 for 8th rank (a8-h8)
                                          // This is if a1=0. Engine uses a8=0.
                                          // Engine's internal indexing: (7-rank)*RANK_MAX + file where rank is 0-7 (1-8)
                                          // So, if index = (7-rank_char_val)*8 + file_char_val
    std::size_t internal_rank = index / 8; // This is 0 for '8' row, 7 for '1' row
    std::size_t file = index % 8;        // 0 for 'a', 7 for 'h'
    char file_char = 'a' + static_cast<char>(file);
    char rank_char = '8' - static_cast<char>(internal_rank);
    std::string s;
    s += file_char;
    s += rank_char;
    return s;
}

// Implementation of new methods
std::string Engine::getFEN() const {
    std::string fen = "";
    // 1. Piece placement
    for (std::size_t r = 0; r < RANK_MAX; ++r) { // Iterates 0 to 7, representing ranks 8 down to 1
        int empty_squares = 0;
        for (std::size_t f = 0; f < FILE_MAX; ++f) { // Iterates 0 to 7, files a to h
            std::size_t square_idx = r * RANK_MAX + f;
            Piece p = m_state.testPieceType(square_idx);
            if (p == Piece::EMPTY) {
                empty_squares++;
            } else {
                if (empty_squares > 0) {
                    fen += std::to_string(empty_squares);
                    empty_squares = 0;
                }
                fen += piece_to_char_fen[static_cast<int>(p)];
            }
        }
        if (empty_squares > 0) {
            fen += std::to_string(empty_squares);
        }
        if (r < RANK_MAX - 1) {
            fen += '/';
        }
    }

    // 2. Active color
    fen += m_state.whiteToMove() ? " w" : " b";

    // 3. Castling availability
    std::string castle_str = "";
    if (m_state.testCastleRights(Castle::WK)) castle_str += 'K';
    if (m_state.testCastleRights(Castle::WQ)) castle_str += 'Q';
    if (m_state.testCastleRights(Castle::BK)) castle_str += 'k';
    if (m_state.testCastleRights(Castle::BQ)) castle_str += 'q';
    fen += " " + (castle_str.empty() ? "-" : castle_str);

    // 4. En passant target square
    fen += " " + indexToSquareString(m_state.enpassantSquare());

    // 5. Halfmove clock (engine doesn't track, use 0)
    fen += " 0";
    // 6. Fullmove number (engine doesn't track, use 1)
    fen += " 1";

    return fen;
}

bool Engine::parseMoveString(const std::string& moveStr, Move& outMove) {
    MoveList legal_moves;
    m_moveGen.generateMoves(m_state, legal_moves); // Generate legal moves for current state

    // Handle castling first
    if (moveStr == "wk" || moveStr == "e1g1") { // Assuming WK
        if (m_state.whiteToMove() && m_state.testCastleRights(Castle::WK)) {
             if (legal_moves.findCastleMove(g1, outMove)) return true; // Use specific target for castle
        }
    } else if (moveStr == "wq" || moveStr == "e1c1") { // Assuming WQ
         if (m_state.whiteToMove() && m_state.testCastleRights(Castle::WQ)) {
            if (legal_moves.findCastleMove(c1, outMove)) return true;
        }
    } else if (moveStr == "bk" || moveStr == "e8g8") { // Assuming BK
        if (!m_state.whiteToMove() && m_state.testCastleRights(Castle::BK)) {
            if (legal_moves.findCastleMove(g8, outMove)) return true;
        }
    } else if (moveStr == "bq" || moveStr == "e8c8") { // Assuming BQ
        if (!m_state.whiteToMove() && m_state.testCastleRights(Castle::BQ)) {
            if (legal_moves.findCastleMove(c8, outMove)) return true;
        }
    }

    // Standard moves (e.g., "e2e4", "e7e8q")
    if (moveStr.length() >= 4 && moveStr.length() <= 5) {
        std::string source_str = moveStr.substr(0, 2);
        std::string target_str = moveStr.substr(2, 2);
        std::size_t source_sq = Engine::squareToIndex(source_str);
        std::size_t target_sq = Engine::squareToIndex(target_str);

        Piece promotion_piece_type = Piece::EMPTY;
        if (moveStr.length() == 5) {
            char prom_char = moveStr[4];
            if (m_state.whiteToMove()) {
                if (prom_char == 'q') promotion_piece_type = Piece::QUEEN;
                else if (prom_char == 'r') promotion_piece_type = Piece::ROOK;
                else if (prom_char == 'b') promotion_piece_type = Piece::BISHOP;
                else if (prom_char == 'n') promotion_piece_type = Piece::KNIGHT;
            } else {
                if (prom_char == 'q') promotion_piece_type = Piece::BQUEEN;
                else if (prom_char == 'r') promotion_piece_type = Piece::BROOK;
                else if (prom_char == 'b') promotion_piece_type = Piece::BBISHOP;
                else if (prom_char == 'n') promotion_piece_type = Piece::BKNIGHT;
            }
        }

        for (const auto& legal_move : legal_moves.moves()) {
            if (legal_move.source() == source_sq && legal_move.target() == target_sq) {
                if (legal_move.promoted()) {
                    if (legal_move.piece() == promotion_piece_type) { // piece() stores promoted piece type
                        outMove = legal_move;
                        return true;
                    }
                } else {
                    if (moveStr.length() == 4) { // Not a promotion
                        outMove = legal_move;
                        return true;
                    }
                }
            }
        }
    }
    return false; // Move string doesn't match any legal move
}


std::string Engine::moveToString(const Move& move) const {
    if (move.castle()) {
        // Determine type of castle by target square (already encoded in move's source by current convention)
        // or by the original king position and target.
        // For simplicity, let's use the standard castle notation if possible.
        // The `Move` object for castle stores the *target* square of the KING in `source()` by current convention in `inputAndParseMove`
        // e.g. `Move::createCastleMove<Castle::WK>()` returns `Move(g1)`
        std::size_t king_target_sq = move.source(); // This is unconventional, source() usually means source.
                                                   // Let's assume move.target() is the king's target for castles for now.
                                                   // Re-evaluating Move::Move(target) constructor. It sets m_data = target | castle_flag
                                                   // So move.source() would be (target | castle_flag) & source_mask which is target if target < 64

        // A better way: check target square of the king.
        // Standard castle notation is what we want, e.g. "e1g1" for white kingside.
        // The `Move` object itself doesn't explicitly store "wk", "wq" etc.
        // It stores king's source and target. Let's use that.
        Piece pieceMoved = move.piece(); // This should be KING or BKING for castles if encoded that way
                                         // The current Move constructor for castle Move(target_sq) does not set piece.
                                         // The Move::print() also has complex logic.
        // Let's try to reconstruct based on known castle target squares.
        // The `move.source()` for a castle move created by `Move(target_square_of_king)` is indeed that target square.
        // Let's try to get the actual source/target from the move object.
        // For now, let's stick to simple source-target notation.
        // The `Move` struct needs to be more explicit about king's source/target for castles.
        // Given the current `Move::print`, it also prints source/target.

        // If `Move` has source and target correctly set for castles (e.g. e1g1)
        // this will work. The current `Move(target_sq_for_castle)` is problematic.
        // Let's assume `move.source()` and `move.target()` are the actual start/end squares of the piece moved.
        // For castling, `inputAndParseMove` creates `Move::createCastleMove<Castle::WK>()` which is `Move(g1)`.
        // This `Move(std::size_t target)` constructor sets:
        // m_data = target | (1 << castle_shift) | (value << value_shift);
        // So `move.source()` would be `(m_data & source_mask)` which is `target & source_mask`.
        // And `move.target()` would be `((m_data & target_mask) >> target_shift)` which is 0. This is not good.

        // Let's try to deduce castle type from the target square stored in move.source()
        // (as per current `Move::createCastleMove` and `Engine::inputAndParseMove` behavior)
        std::size_t king_final_sq = move.source(); // This is where the king lands.
        if (king_final_sq == g1) return "e1g1"; // White Kingside (conventionally O-O)
        if (king_final_sq == c1) return "e1c1"; // White Queenside (O-O-O)
        if (king_final_sq == g8) return "e8g8"; // Black Kingside
        if (king_final_sq == c8) return "e8c8"; // Black Queenside
        // Fallback if somehow it's a castle move but not one of these targets
        return indexToSquareString(move.source()) + indexToSquareString(move.target());

    }

    std::string str = indexToSquareString(move.source()) + indexToSquareString(move.target());
    if (move.promoted()) {
        Piece p = move.piece(); // This is the promoted piece type
        // Convert piece type to its character representation (lowercase for black, uppercase for white)
        // For promotion, standard is lowercase char for the piece type (q, r, b, n)
        if (p == QUEEN || p == BQUEEN) str += 'q';
        else if (p == ROOK || p == BROOK) str += 'r';
        else if (p == BISHOP || p == BBISHOP) str += 'b';
        else if (p == KNIGHT || p == BKNIGHT) str += 'n';
    }
    return str;
}


Move Engine::getBestMoveFinal() const {
    return m_bestMoveFinal;
}

void Engine::calculateBestMove(std::uint32_t search_depth) {
    m_depth = search_depth; // Set the search depth for minimax

    // iterativeMinimax already uses m_state, m_depth and updates m_bestMoveFinal
    // It also handles time limits.
    // We need to ensure m_state is correctly set (side to move, etc.) before calling this.
    iterativeMinimax(m_state);
}

bool Engine::isWhiteToMove() const {
    return m_state.whiteToMove();
}

State& Engine::getCurrentState() {
    return m_state;
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

int Engine::minimax(const State& state, const std::uint32_t depth, int alpha, int beta)
{
	m_nodes++;

	if (depth == 0)
	{
		return evaluate(state);
	}

	//time cutoff for iterative deepening
	if (m_stopSearch)
	{
		return state.whiteToMove() ? INT_MAX : INT_MIN;
	}

	//only test time every 1000 nodes to avoid frequent system calls
	if (m_timeCheckCount >= TIME_EVALUATION_NODE_DELAY)
	{
		const auto now{ std::chrono::high_resolution_clock::now() };
		const std::chrono::duration<float> duration{ now - m_searchStartTime };

		if (static_cast<size_t>(duration.count()) >= MAX_EVALUATION_TIME_SECONDS)
		{
			m_stopSearch = true;
			return state.whiteToMove() ? INT_MAX : INT_MIN;
		}

		m_timeCheckCount = 0;
	}

	m_timeCheckCount++;

	if (state.whiteToMove())
	{
		MoveList moves;
		m_moveGen.generateMoves(state, moves);
		moves.sortMoveList();

		int max_eval{ INT_MIN };
		bool anyLegalMoves{ false };

		for (Move move : moves.moves())
		{
			State new_state{ state };

			if (makeMove(move, new_state))
			{
				anyLegalMoves = true;

				new_state.flipSide();

				const int eval = minimax(new_state, depth - 1, alpha, beta);

				//time cutoff for iterative deepening
				if (m_stopSearch)
				{
					return state.whiteToMove() ? INT_MAX : INT_MIN;
				}

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

		int min_eval{ INT_MAX };
		bool anyLegalMoves{ false };

		for (Move move : moves.moves())
		{
			State new_state{ state };

			if (makeMove(move, new_state))
			{
				anyLegalMoves = true;

				new_state.flipSide();
				const int eval = minimax(new_state, depth - 1, alpha, beta);

				//time cutoff for iterative deepening
				if (m_stopSearch)
				{
					return state.whiteToMove() ? INT_MAX : INT_MIN;
				}

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

void Engine::iterativeMinimax(const State& state)
{
	std::uint32_t depth{ 1 };
	m_searchStartTime = std::chrono::high_resolution_clock::now();
	m_timeCheckCount = 0;
	m_stopSearch = false;

	while (!m_stopSearch)
	{
		m_depth = depth;
		minimax(m_state, depth, INT_MIN, INT_MAX);
		m_depthSearched = depth; //TODO: fix these vars there should only be one
		depth++;

		if (!m_stopSearch)
		{
			m_bestMoveFinal = m_bestMove;
		}
	}
}

void Engine::step(const bool engine_side_white, const bool flip_board, const std::uint32_t depth)
{
	m_state.printBoard(flip_board, RF::no_sqr);
	m_depth = depth;

	while (true)
	{
		const auto start_time = std::chrono::high_resolution_clock::now();

		if (m_state.whiteToMove() == engine_side_white)
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
						State new_state{ m_state };
						new_state.printBoard(flip_board, m_moveSource);
						move.print();

						if (makeMove(move, new_state))
						{
							m_state = new_state;
							m_moveSource = move.source();
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
				iterativeMinimax(m_state);
				makeMove(m_bestMoveFinal, m_state);

				m_moveSource = m_bestMoveFinal.source();
			}
		}
		else
		{
			if constexpr (ENGINE_PLAY_ITSELF)
			{
				//engine move
				std::cout << "thinking" << std::endl;
				iterativeMinimax(m_state);
				makeMove(m_bestMoveFinal, m_state);

				m_moveSource = m_bestMoveFinal.source();
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
						new_state.printBoard(flip_board, m_moveSource);
						move.print();

						if (makeMove(move, new_state))
						{
							m_state = new_state;
							m_moveSource = move.source();
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
		m_state.printBoard(flip_board, m_moveSource);

		std::cout << "move: ";
		m_bestMoveFinal.print();

		std::cout << "depth: " << m_depthSearched << std::endl;
		std::cout << "nodes: " << m_nodes << std::endl;
		std::cout << "evaluations: " << m_evaluations << std::endl;
		std::cout << "prunes: " << m_prunes << std::endl;
		std::cout << "mates: " << m_mates << std::endl;
		std::cout << duration.count() << " seconds" << std::endl;

		m_nodes = 0;
		m_evaluations = 0;
		m_prunes = 0;
		m_mates = 0;
		m_depthSearched = 0;

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
	m_state.printBoard(flipped, RF::no_sqr);
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
	if (castle)//TODO: remove moveQuiet and moveCapture they have unnessesary loops and checks. make template function
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
			else if (enpassant)//TODO: maybe make move enpassant and other compile time known piece movers
			{
				if (state.whiteToMove())
				{
					state.popPiece(Piece::PAWN, source);
					state.popPiece(Piece::BPAWN, target + 8);
					state.setPiece(Piece::PAWN, target);
				}
				else
				{
					state.popPiece(Piece::BPAWN, source);
					state.popPiece(Piece::PAWN, target - 8);
					state.setPiece(Piece::BPAWN, target);
				}
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
			{//TODO: we know its a pawn we dont have to loop through pieces
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











