#include <string_view.hpp>

namespace conftaal {

enum class order {
	left,
	right,
	unordered
};

// A lower value means a higher precedence.
int get_precedence(string_view op, bool unary);

order get_associativity(int precedence);

order higher_precedence(
	string_view left_op, bool left_op_unary,
	string_view right_op
);

}
