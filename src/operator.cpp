#include <cassert>

#include <string_view.hpp>

#include "operator.hpp"

namespace conftaal {

int get_precedence(operator_ op) {
	switch (op) {
		case operator_::dot:
		case operator_::call:
		case operator_::index:
			return 1;
		case operator_::unary_plus:
		case operator_::unary_minus:
		case operator_::complement:
		case operator_::not_:
			return 3;
		case operator_::power:
			return 4;
		case operator_::times:
		case operator_::divide:
		case operator_::modulo:
			return 5;
		case operator_::plus:
		case operator_::minus:
			return 6;
		case operator_::left_shift:
		case operator_::right_shift:
			return 7;
		case operator_::greater:
		case operator_::less:
		case operator_::greater_or_equal:
		case operator_::less_or_equal:
			return 8;
		case operator_::equal:
		case operator_::inequal:
			return 9;
		case operator_::bit_and:
			return 10;
		case operator_::bit_xor:
			return 11;
		case operator_::bit_or:
			return 12;
		case operator_::and_:
			return 13;
		case operator_::or_:
			return 14;
	}
	assert(false);
}

order get_associativity(int precedence) {
	switch (precedence) {
		case 4: // **
			return order::right;
		case 8: // > < >= <=
		case 9: // != ==
			return order::unordered;
		default:
			return order::left;
	}
}

order higher_precedence(operator_ left_op, operator_ right_op) {
	int left = get_precedence(left_op);
	int right = get_precedence(right_op);
	if (left < right) return order::left;
	if (left > right) return order::right;
	return get_associativity(left);
}

}
