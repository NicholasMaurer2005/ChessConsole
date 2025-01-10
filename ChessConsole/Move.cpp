#include "Move.h"


//pawn promoted
Move::Move(const std::size_t source, const std::size_t target, const Piece piece, const Piece captured_piece, const bool capture)
{
	constexpr std::uint8_t promote_score = 100;

	if (capture)
	{
		//score is mvv_lva + promote_score
		m_data = static_cast<uint32_t>(source)
			| (static_cast<std::uint32_t>(target) << target_shift)
			| (static_cast<std::uint32_t>(piece) << piece_shift)
			| (static_cast<std::uint32_t>(single_bit) << promoted_shift)
			| (static_cast<std::uint32_t>(single_bit) << capture_shift)
			| ((promote_score + mvv_lva[Piece::PAWN][captured_piece]) << value_shift);
	}
	else
	{
		//score is promote score
		m_data = static_cast<uint32_t>(source)
			| (static_cast<std::uint32_t>(target) << target_shift)
			| (static_cast<std::uint32_t>(piece) << piece_shift)
			| (static_cast<std::uint32_t>(single_bit) << promoted_shift)
			| (promote_score << value_shift);
	}
}

//enpassant //TODO: enpassant is always pawn capturing pawn
Move::Move(const std::size_t source, const std::size_t target, const Piece captured_piece)
{
	//score is mvv_lva
	m_data = static_cast<uint32_t>(source) 
		| (static_cast<std::uint32_t>(target) << target_shift) 
		| (static_cast<std::uint32_t>(single_bit) << enpassant_shift) 
		| (mvv_lva[Piece::PAWN][Piece::PAWN] << value_shift);
}

//castle
Move::Move(const std::size_t square)
{
	constexpr std::uint8_t byte_score = 1;

	//score is 1
	m_data = static_cast<uint32_t>(square) 
		| (static_cast<std::uint32_t>(single_bit) << castle_shift) 
		| (byte_score << value_shift);
}

//double pawn push
Move::Move(const std::size_t source, const std::size_t target) //arbitrary bool to call constructor
{
	constexpr std::uint8_t byte_score = 2;

	//score is 2
	m_data = static_cast<uint32_t>(source) 
		| (static_cast<std::uint32_t>(target) << target_shift) 
		| (static_cast<std::uint32_t>(Piece::PAWN) << piece_shift) 
		| (static_cast<std::uint32_t>(single_bit) << double_shift) 
		| (byte_score << value_shift);
}

//other 
Move::Move(const std::size_t source, const std::size_t target, const Piece piece, const Piece captured_piece)
{
	if (captured_piece == Piece::NO_PIECE)
	{
		//move score is 0
		m_data = static_cast<uint32_t>(source)
			| (static_cast<std::uint32_t>(target) << target_shift)
			| (static_cast<std::uint32_t>(piece) << piece_shift);
	}
	else
	{
		//move score is mvv_lva
		m_data = static_cast<uint32_t>(source)
			| (static_cast<std::uint32_t>(target) << target_shift)
			| (static_cast<std::uint32_t>(piece) << piece_shift)
			| (static_cast<std::uint32_t>(single_bit) << capture_shift)
			| (mvv_lva[piece][captured_piece] << value_shift);
	}
}

Move::Move()
	: m_data() {}



std::size_t Move::source() const
{
	const std::size_t source_data{ static_cast<std::size_t>(m_data & source_mask) };
	return source_data;
}

std::size_t Move::target() const
{
	const std::size_t target_data{ static_cast<std::size_t>((m_data & target_mask) >> target_shift) };
	return target_data;
}

Piece Move::piece() const
{
	const Piece data{ static_cast<Piece>((m_data & piece_mask) >> piece_shift) };
	return data;
}

bool Move::promoted() const
{
	const bool promoted_data{ static_cast<bool>((m_data & promoted_mask)) };
	return promoted_data;
}

bool Move::capture() const
{
	const bool capture_data{ static_cast<bool>((m_data & capture_mask)) };
	return capture_data;
}

bool Move::enpassant() const
{
	const bool enpassant_data{ static_cast<bool>((m_data & enpassant_mask)) };
	return enpassant_data;
}

bool Move::castle() const
{
	const bool castle_data{ static_cast<bool>((m_data & castle_mask)) };
	return castle_data;
}

std::uint32_t Move::value() const
{
	const std::uint32_t value_data{ static_cast<std::uint32_t>((m_data & value_mask) >> value_shift) };
	return value_data;
}

bool Move::doublePawnPush() const
{
	const bool castle_data{ static_cast<bool>((m_data & double_mask)) };
	return castle_data;
}

void Move::print() const
{
	const std::size_t source_p{ source() };
	const std::size_t target_p{ target() };
	const bool promoted_p{ promoted() };
	const bool capture_p{ capture() };
	const bool enpassant_p{ enpassant() };
	const bool castle_p{ castle() };

	std::cout << index_to_rf[source_p] << (capture_p ? "x" : "") << index_to_rf[target_p] << " - pr:" << promoted_p << " ca:" << capture_p << " en:" << enpassant_p << " cas:" << castle_p << std::endl;
}

Move& Move::operator=(const Move& other)
{
	m_data = other.m_data;
	return *this;
}