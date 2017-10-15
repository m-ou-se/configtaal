#include <cassert>

#include "operator.hpp"

namespace conftaal {

std::string to_string(Operator op) {
	switch (op) {
		case Operator::dot:                 return ".";
		case Operator::index:               return "[]";
		case Operator::call:                return "()";
		case Operator::colon:               return ":";
		case Operator::equal:               return "==";
		case Operator::inequal:             return "!=";
		case Operator::greater:             return ">";
		case Operator::less:                return "<";
		case Operator::greater_or_equal:    return ">=";
		case Operator::less_or_equal:       return "<=";
		case Operator::unary_plus:          return "+";
		case Operator::unary_minus:         return "-";
		case Operator::complement:          return "~";
		case Operator::logical_not:         return "!";
		case Operator::plus:                return "+";
		case Operator::minus:               return "-";
		case Operator::times:               return "*";
		case Operator::divide:              return "/";
		case Operator::modulo:              return "%";
		case Operator::power:               return "**";
		case Operator::left_shift:          return "<<";
		case Operator::right_shift:         return ">>";
		case Operator::bit_and:             return "&";
		case Operator::bit_or:              return "|";
		case Operator::bit_xor:             return "^";
		case Operator::logical_and:         return "&&";
		case Operator::logical_or:          return "||";
	}
	return "unknown-op:" + std::to_string(int(op));
}

int get_precedence(Operator op) {
	switch (op) {
		case Operator::dot:
		case Operator::colon:
		case Operator::call:
		case Operator::index:
			return 1;
		case Operator::unary_plus:
		case Operator::unary_minus:
		case Operator::complement:
		case Operator::logical_not:
			return 3;
		case Operator::power:
			return 4;
		case Operator::times:
		case Operator::divide:
		case Operator::modulo:
			return 5;
		case Operator::plus:
		case Operator::minus:
			return 6;
		case Operator::left_shift:
		case Operator::right_shift:
			return 7;
		case Operator::greater:
		case Operator::less:
		case Operator::greater_or_equal:
		case Operator::less_or_equal:
			return 8;
		case Operator::equal:
		case Operator::inequal:
			return 9;
		case Operator::bit_and:
			return 10;
		case Operator::bit_xor:
			return 11;
		case Operator::bit_or:
			return 12;
		case Operator::logical_and:
			return 13;
		case Operator::logical_or:
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

order higher_precedence(Operator left_op, Operator right_op) {
	int left = get_precedence(left_op);
	int right = get_precedence(right_op);
	if (left < right) return order::left;
	if (left > right) return order::right;
	return get_associativity(left);
}

}
