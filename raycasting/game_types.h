//ЗАГОЛОВОЧНЫЙ ФАЙЛ ПРОСТЕЙШИХ ИГРОВЫХ ТИПОВ ДАННЫХ
//TODO: перенести класс Pistol сюда
#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <utility>

//const int WIDTH = 800;
//const int HEIGHT = 450;

namespace game {
	enum Color {
		BLACK = 30, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE
	};

	struct Vector2 {
		double x, y;
	};

	struct Vector2i {
		int x, y;
	};

	struct Player {
		//ox and oy - self coordinates; angle - direction angle (in degrees)
		double x, y;
		double angle;
		game::Vector2 pos{ x, y };
	};

	struct RayResult {
		double dist;

		//0 - nothing, 1 - wall, 2 - enemy
		int type;
		int intersection_x, intersection_y;
		bool was_vertical;
		double d_ix, d_iy;
	};

	struct Texture {
		//Само представление текстуры как 2-мерный цветов
		std::vector<std::vector<game::Color>> texture;

		//Размеры текстуры. Константы!
		int texture_hor;
		int texture_vert;

		Texture() {

		}

		Texture(std::string filename) {
			//Инициализируем логику текстур стен
			//Открываем файл с текстурой стены
			std::ifstream wall_texture_file(filename);
			std::string line_wall_file;

			while (std::getline(wall_texture_file, line_wall_file)) {
				//Разделяем все отдельные слова при помощи stringstream
				std::stringstream extractable_row(line_wall_file);
				//В это строку записывается текущий цвет из стрингстрем
				std::string temporary;
				//Временный вектор из всех текущих цветов в формате string temporary
				std::vector<game::Color> temp_vec;

				//Читаем все слова из текущей строки
				while (extractable_row >> temporary) {
					game::Color color_to_add;

					if (temporary == "GREEN") {
						color_to_add = game::GREEN;
					} else if (temporary == "RED") {
						color_to_add = game::RED;
					} else if (temporary == "BLACK") {
						color_to_add = game::BLACK;
					} else if (temporary == "BLUE") {
						color_to_add = game::BLUE;
					} else if (temporary == "YELLOW") {
						color_to_add = game::YELLOW;
					} else if (temporary == "CYAN") {
						color_to_add = game::CYAN;
					} else if (temporary == "MAGENTA") {
						color_to_add = game::MAGENTA;
					} else {
						color_to_add = game::WHITE;
					}

					temp_vec.push_back(color_to_add);
				}

				texture.push_back(temp_vec);
			}

			wall_texture_file.close();

			//Все горизонтальные полоски изображения имеют равную длину. Длина горизонтальной полоски равна длине одного из векторов внутри вектора
			texture_hor = texture[0].size();
			texture_vert = texture.size();
		}
	};

	class Enemy {
	public:
		int x, y;
		game::Vector2i pos;

		//айди врага
		int id;

		//Время в секундах, константное время между ходами врага
		double MOVE_TIME = 1.5;

		//Сколько осталось до следующего хода
		double move_cooldown;

		//Текстура спрайта
		Texture sprite;

		Enemy() {

		}

		//TODO: УБРАТЬ!!!! ЭТО ЛЕГАСИ КОД
		Enemy(int x, int y, int id, double move_cooldown) {
			this->x = x;
			this->y = y;
			this->id = id;
			this->pos = { x, y };
			this->move_cooldown = move_cooldown;
		}

		Enemy(int x, int y, int id, double move_cooldown, std::string texture_name) {
			this->x = x;
			this->y = y;
			this->id = id;
			this->pos = {x, y};
			this->move_cooldown = move_cooldown;
			sprite = Texture(texture_name);
		}

		//Функция находит кратчайший путь врагу до игрока по карте
		std::vector<std::pair<int, int>> bfs_pathfind(game::Player player, const int MAP_SIZE, const std::string map[]) {
			int start_x = this->x;
			int start_y = this->y;

			//Вектор всех посещенных вершин
			std::vector<std::vector<short>> visited(MAP_SIZE, std::vector<short>(MAP_SIZE));

			//Реализация BFS с поиском кратчайшего пути
			std::queue<std::pair<int, int>> vertices;
			std::vector<std::vector<int>> distances(MAP_SIZE, std::vector<int>(MAP_SIZE, 1000));
			std::vector<std::vector<std::pair<int, int>>> previous_v(MAP_SIZE, std::vector<std::pair<int, int>>(MAP_SIZE));

			vertices.push({ start_x, start_y });
			distances[start_y][start_x] = 0;
			previous_v[start_y][start_x] = {start_x, start_y};

			int player_x = player.x, player_y = player.y;
			int dist_path = 0;

			while (!vertices.empty()) {
				std::pair<int, int> vertice = vertices.front();
				vertices.pop();

				int v_x = vertice.first, v_y = vertice.second;

				visited[v_y][v_x] = 1;

				if (v_x == player_x && v_y == player_y) {
					break;
				}

				//враг может двигаться в 4 направлениях
				std::pair<int, int> r = { v_x + 1, v_y };
				std::pair<int, int> l = { v_x - 1, v_y };
				std::pair<int, int> up = { v_x, v_y - 1 };
				std::pair<int, int> down = { v_x, v_y + 1 };

				auto check_dir = [&](std::pair<int, int> dir) {
					if (dir.first >= 0 and dir.second >= 0 and dir.first < MAP_SIZE and dir.second < MAP_SIZE and map[dir.second][dir.first] == '0' and !visited[dir.second][dir.first]) {
						vertices.push(dir);
						distances[dir.second][dir.first] = distances[v_y][v_x]+1;
						previous_v[dir.second][dir.first] = vertice;
						visited[dir.second][dir.first] = 1;
					}
				};

				check_dir(r); check_dir(l); check_dir(up); check_dir(down);
			}

			//Восстанавливаем путь
			std::vector<std::pair<int, int>> path;
			int path_x = player_x, path_y = player_y;

			if (distances[player_y][player_x] == 1000) {
				return {};
			}

			path.push_back({player_x, player_y});

			while (path_x != start_x or path_y != start_y) {
				path.push_back(previous_v[path_y][path_x]);
				path_x = previous_v[path_y][path_x].first;
				path_y = previous_v[path_y][path_x].second;
			}

			if (!path.empty()) {
				path.pop_back();
			}

 			return path;
		}

		//Заставляет врага делать 1 ход. Считается по bfs_pathfind
		void follow_player(game::Player player, const int MAP_SIZE, const std::string map[]) {
			std::vector<std::pair<int, int>> path = bfs_pathfind(player, MAP_SIZE, map);

			if (!path.empty()) {
				x = path.back().first;
				y = path.back().second;
			}

			this->move_cooldown = MOVE_TIME;
		}
	};
};