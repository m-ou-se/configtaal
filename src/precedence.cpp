#include <cassert>

#include <string_view.hpp>

#include "precedence.hpp"

namespace conftaal {

int get_precedence(string_view op, bool unary) {
	if (unary) {
		return 3;
	} else {
		if (!op.empty()) switch (op[0]) {
			case '.': case '(': case '[':
				return 1; // . ( [
			case '*':
				if (op.size() > 1 && op[1] == '*') return 4; // **
			case '/': case '%':
				return 5; // * / %
			case '+': case '-':
				return 6; // + -
			case '<': case '>':
				if (op.size() > 1 && op[1] == op[0]) return 7; // >> <<
				return 8; // > < >= <=
			case '=': case '!':
				return 9; // != ==
			case '&':
				if (op.size() > 1 && op[1] == '&') return 13; // &&
				return 10; // &
			case '^':
				return 11; // ^
			case '|':
				if (op.size() > 1 && op[1] == '|') return 14; // ||
				return 12; // |
		}
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

order higher_precedence(string_view left_op, bool left_op_unary, string_view right_op) {
	int left = get_precedence(left_op, left_op_unary);
	int right = get_precedence(right_op, false);
	if (left < right) return order::left;
	if (left > right) return order::right;
	return get_associativity(left);
}

}
