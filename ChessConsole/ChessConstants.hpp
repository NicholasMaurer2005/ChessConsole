#pragma once

#include <cstdint>
#include <string>
#include <array>
#include <string_view>
#include <cstddef>

constexpr std::uint64_t single_bit									= 1;
constexpr std::size_t   RANK_MAX									= 8;
constexpr std::size_t   FILE_MAX									= 8;
constexpr std::size_t   MAX_BOARD_POSITIONS = FILE_MAX * RANK_MAX; //64
constexpr std::size_t   WHITE_COLOR									= 0;
constexpr std::size_t   BLACK_COLOR									= 1;
constexpr std::size_t   MAX_COLORS									= 2;
constexpr std::size_t   MAGIC_ITERATION_COUNT						= 1000000;
constexpr std::size_t   MAX_BISHOP_ATTACKS							= 512;
constexpr std::size_t   MAX_ROOK_ATTACKS							= 4096;
constexpr std::size_t   PIECE_COUNT									= 12;
constexpr std::size_t   MAX_MOVELIST_COUNT							= 256;
constexpr std::uint32_t MAX_MINIMAX_DEPTH							= INT_MAX - 1;
constexpr std::size_t   TIME_EVALUATION_NODE_DELAY					= 1000;
constexpr std::size_t   MAX_EVALUATION_TIME_SECONDS					= 1; //maybe remove
constexpr int           CHECKMATE_SCORE					            = INT_MIN + 1; 

constexpr bool USING_PREGENERATED_MAGICS = true;
constexpr bool PRINT_GENERATED_MAGICS = false;
constexpr bool ENGINE_PLAY_ITSELF = false;
constexpr bool PLAYER_PLAY_ITSELF = true;

constexpr std::string_view start_position_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
constexpr std::string_view tricky_position_fen = "r3k2r/p11pqpb1/bn2pnp1/2pPN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";

//most valuable victim - least valuable aggressor [capturing piece][captured piece]
constexpr std::array<std::array<std::uint32_t, PIECE_COUNT>, PIECE_COUNT> mvv_lva = {{
	{{ 16, 26, 36, 46, 56, 66,  16, 26, 36, 46, 56, 66, }},
	{{ 15, 25, 35, 45, 55, 65,  15, 25, 35, 45, 55, 65, }},
	{{ 14, 24, 34, 44, 54, 64,  14, 24, 34, 44, 54, 64, }},
	{{ 13, 23, 33, 43, 53, 63,  13, 23, 33, 43, 53, 63, }},
	{{ 12, 22, 32, 42, 52, 62,  12, 22, 32, 42, 52, 62, }},
	{{ 11, 21, 31, 41, 51, 61,  11, 21, 31, 41, 51, 61, }},

	{{ 16, 26, 36, 46, 56, 66,  16, 26, 36, 46, 56, 66, }},
	{{ 15, 25, 35, 45, 55, 65,  15, 25, 35, 45, 55, 65, }},
	{{ 14, 24, 34, 44, 54, 64,  14, 24, 34, 44, 54, 64, }},
	{{ 13, 23, 33, 43, 53, 63,  13, 23, 33, 43, 53, 63, }},
	{{ 12, 22, 32, 42, 52, 62,  12, 22, 32, 42, 52, 62, }},
	{{ 11, 21, 31, 41, 51, 61,  11, 21, 31, 41, 51, 61, }}
}};

enum Piece {
	PAWN = 0,
	KNIGHT = 1,
	BISHOP = 2,
	ROOK = 3,
	QUEEN = 4,
	KING = 5,
	BPAWN = 6,
	BKNIGHT = 7,
	BBISHOP = 8,
	BROOK = 9,
	BQUEEN = 10,
	BKING = 11,
	NO_PIECE = 12
};

consteval std::array<std::size_t, 128> create_char_to_piece()
{
	std::array< std::size_t, 128> table = { 0 };
	table['P'] = Piece::PAWN;
	table['N'] = Piece::KNIGHT;
	table['B'] = Piece::BISHOP;
	table['R'] = Piece::ROOK;
	table['Q'] = Piece::QUEEN;
	table['K'] = Piece::KING;
	table['p'] = Piece::BPAWN;
	table['n'] = Piece::BKNIGHT;
	table['b'] = Piece::BBISHOP;
	table['r'] = Piece::BROOK;
	table['q'] = Piece::BQUEEN;
	table['k'] = Piece::BKING;
	return table;
}
constexpr std::array<std::size_t, 128> char_to_piece = create_char_to_piece();

constexpr std::array<char, PIECE_COUNT> piece_to_char = { 'P', 'N', 'B', 'R', 'Q', 'K', 'X', 'n', 'b', 'r', 'q', 'k' };

constexpr std::array<std::string_view, 64> index_to_rf = {
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

constexpr std::array<std::uint8_t, MAX_BOARD_POSITIONS> castling_rights = {
	 7, 15, 15, 15,  3, 15, 15, 11,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	13, 15, 15, 15, 12, 15, 15, 14
};

constexpr std::array<int, PIECE_COUNT> piece_value = {
	100, //white pawn
	300, //white knight
	300, //white bishop
	500, //white rook
	900, //white queen
	0,   //white king
	-100, //black pawn
	-300, //black knight
	-300, //black bishop
	-500, //black rook
	-900, //black queen
	-0,   //black king
};

enum Castle {
	WK = 0b0001,
	WQ = 0b0010, 
	BK = 0b0100,
	BQ = 0b1000
};

enum Color {
	WHITE = 0,
	BLACK = 1
};

enum Occupancy {
	WHITEOCC = 0,
	BLACKOCC = 1,
	BOTH = 2
};

enum RF {
	a8, b8, c8, d8, e8, f8, g8, h8,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a1, b1, c1, d1, e1, f1, g1, h1, no_sqr
};

enum MoveType {
	QUIET,
	CAPTURE,
	QUIET_PROMOTE,
	PROMOTE,
	ENPASSANT,
	DOUBLE_PAWN
};