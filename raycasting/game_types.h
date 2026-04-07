//ЗАГОЛОВОЧНЫЙ ФАЙЛ ПРОСТЕЙШИХ ИГРОВЫХ ТИПОВ ДАННЫХ
//TODO: перенести класс Pistol сюда
#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <fstream>

//const int WIDTH = 800;
//const int HEIGHT = 450;

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

	struct RayResult {
		double dist;
		//0 - nothing, 1 - wall, 2 - enemy
		int type;
	};

	struct Enemy {
		//размер противника по ширине
		const double ENEMY_SIZE = 0.5;
		//const int ENEMY_HEIGHT = HEIGHT/4;
		double x, y;
		Vector2 pos{ x, y };
		int id;
	};
};