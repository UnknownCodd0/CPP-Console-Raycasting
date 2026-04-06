//гюцнкнбнвмши тюик хцпнбшу рхонб дюммшу
#pragma once

namespace game {
	enum Color {
		BLACK = 30, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
	};

	struct Vector2 {
		double x, y;
	};

	struct Player {
		//ox and oy - self coordinates; angle - direction angle (in degrees)
		double x, y;
		double angle;
		Vector2 pos{ x, y };
	};
}