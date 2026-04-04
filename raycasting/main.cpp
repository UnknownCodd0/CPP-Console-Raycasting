#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <thread>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <codecvt>
#include <Windows.h>
#include <fstream>

#define and &&
#define not !
#define or ||

/**
 *    ______  _____   __     _____   ___   _____ _____ _____ _   _ _____          _____
 *    | ___ \/ _ \ \ / /    /  __ \ / _ \ /  ___|_   _|_   _| \ | |  __ \        /  __ \ _     _
 *    | |_/ / /_\ \ V /_____| /  \// /_\ \\ `--.  | |   | | |  \| | |  \/        | /  \/| |_ _| |_
 *    |    /|  _  |\ /______| |    |  _  | `--. \ | |   | | | . ` | | __         | |  |_   _|_   _|
 *    | |\ \| | | || |      | \__/\| | | |/\__/ / | |  _| |_| |\  | |_\ \        | \__/\|_|   |_|
 *    \_| \_\_| |_/\_/       \____/\_| |_/\____/  \_/  \___/\_| \_/\____/         \____/
 *
 *
 *     __
 *     \ \
 *    (_) |
 *      | |
 *     _| |
 *    (_) |
 *     /_/
 *
 *     _____       _     _             _               _   _           _                            ______                            _____                _       _    ______                              _
 *    |  __ \     (_)   | |           | |          _  | | | |         | |                           |  _  \                          |  __ \              (_)     (_)   |  _  \                            | |
 *    | |  \/_   _ _  __| | ___  ___  | |__  _   _(_) | | | | ___  ___| |_ ___ ______ ___   ___ __  | | | |_____   _____ _ __ ___    | |  \/ ___ _ __ ___  _ _ __  _    | | | |___  ___ _ __  ___  ___  ___| | __
 *    | | __| | | | |/ _` |/ _ \/ __| | '_ \| | | |   | | | |/ _ \/ __| __/ _ \_  / _` \ \ / / '__| | | | / _ \ \ / / _ \ '__/ __|   | | __ / _ \ '_ ` _ \| | '_ \| |   | | | / _ \/ _ \ '_ \/ __|/ _ \/ _ \ |/ /
 *    | |_\ \ |_| | | (_| |  __/\__ \ | |_) | |_| |_  \ \_/ /  __/ (__| || (_) / / (_| |\ V /| |_   | |/ /  __/\ V /  __/ |  \__ \_  | |_\ \  __/ | | | | | | | | | |_  | |/ /  __/  __/ |_) \__ \  __/  __/   < _
 *     \____/\__,_|_|\__,_|\___||___/ |_.__/ \__, (_)  \___/ \___|\___|\__\___/___\__,_| \_/ |_( )  |___/ \___| \_/ \___|_|  |___( )  \____/\___|_| |_| |_|_|_| |_|_( ) |___/ \___|\___| .__/|___/\___|\___|_|\_( )
 *                                            __/ |                                            |/                                |/                                 |/                 | |                      |/
 *                                           |___/                                                                                                                                     |_|
 */





using namespace std;

//CONFIG
const double PI = 3.141592653589793;
const double FOV = PI / 2;
const int WIDTH = 800;
const int HEIGHT = 450;
const int CELLSIZE = 1;
const int MAP_SIZE = 10;
const double EPS = 0.00001;
const int MAX_DIST = 6;

//scale - масштаб
const int scale = 10;
//sw и sh - масштабированные ширина и высота
const int sw = WIDTH / scale, sh = HEIGHT / scale;

//Coordinates move from: 0, 0 - top left corner; 5, 5 - downmost right corner.
const string map[MAP_SIZE] = {
	"1111111111",
	"1000000001",
	"1111110101",
	"1000000101",
	"1000011101",
	"1011011101",
	"1011011101",
	"1001011101",
	"1000011101",
	"1111111101"
};


double to_rad(double deg) {
	return (deg * PI) / (double) 180;
}

double to_deg(double rad) {
	return rad * 180 / PI;
}

struct Vector2 {
	double x, y;
};

struct Player {
	//ox and oy - self coordinates; angle - direction angle (in degrees)
	double x, y;
	double angle;
	Vector2 pos{x, y};
};

//TODO: change euclidean distance after managing DDA & rendering to avoid fisheye
double dist(Vector2 p1, Vector2 p2) {
	return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

double dda(Player player) {
	//DDA
	//these two variables indicate ray's direction
	while (player.angle < 0) {
		player.angle += 360;
	}

	while (player.angle >= 360) {
		player.angle -= 360;
	}

	bool looks_up = player.angle > 180 and player.angle < 360;
	bool looks_right = not (player.angle > 90 and player.angle < 270);

	//vertical
	//EXPLANATION FOR VARIABLES
	//h/v prefix means that variable is for horizontal or vertical intersection checking
	//xn/yn are variables that mean distance from player to the nearest integer vert/horizont line
	//xs/ys are variabes that mean distance from (xn; yn) to the next integer very/horizont line

	//calculating distance
	//!!!all formulas are in that one video and Gemini chat

	double vxn = -(player.x - floor(player.x));

	//padding for segments
	const int padding = 0.1;

	if (abs(player.angle - 0) <= padding or abs(player.angle - 90) <= padding or abs(player.angle - 180) <= padding
	or abs(player.angle - 270) <= padding or abs(player.angle - 360) <= padding) {
		if (player.angle <= 0 or player.angle <= 90 or player.angle <= 180 or player.angle <= 270 or player.angle <= 360) {
			player.angle -= padding;
		} else {
			player.angle += padding;
		}
	}

	//makes formula look like vxn = CELLSIZE - player.x + floor(player.x)
	if (looks_right) {
		vxn += CELLSIZE;
	}

	//trigonometry
	double vyn = vxn * tan(to_rad(player.angle));

	//dist from (xn; yn) to next int lines
	double vxs = CELLSIZE;

	//if player looks to the left, then the closest line to the player is to the left, not right
	if (not looks_right) {
		vxs = -vxs;
	}

	//trigonometry
	double vys = vxs * tan(to_rad(player.angle));


	//---checking for collisions with walls (vert)
	//checking if we intersected with any wall that lies within an integer vertical line on our way
	bool vert_intersection = false;

	//distance is that big because later we use min function, and 1000 isn't going to litter the function if we didn't get a vert intersection
	double vertical_distance = 1000;

	//converting the dist to the integer line to int
	int closest_vx = int(player.x + vxn - (looks_right ? 0 : EPS));

	//doing that because int truncates numbers. For example, if ray hit 2, a wall is within [1, 2] but located in map as 1, then we won't be able to detect. by taking off EPS we get 1.999... which is 1 as int

	int closest_vy = int(player.y + vyn - (looks_up? EPS : 0));


	//the same things but as floats
	Vector2 closest_v{ player.x + vxn, player.y + vyn};

	//the closest_vy and vx checks are because sometimes the ray might travel past walls
	//here we check
	if (closest_vy < MAP_SIZE and closest_vx < MAP_SIZE and closest_vy >= 0 and closest_vx >= 0 and map[closest_vy][closest_vx] - '0') {
		vert_intersection = true;
		vertical_distance = dist(closest_v, player.pos);
	} else {
		Vector2 current_point = closest_v;
		int map_x = (int)(current_point.x - (looks_right ? 0 : EPS));
		int map_y = (int)(current_point.y - (looks_up ? EPS : 0));

		while (map_x < MAP_SIZE and map_y < MAP_SIZE and map_x >= 0 and map_y >= 0 and dist(current_point, player.pos) <= MAX_DIST) {
			if (map[map_y][map_x] - '0') {
				vert_intersection = true;
				vertical_distance = dist(current_point, player.pos);
				break;
			}

			current_point.x += vxs;
			current_point.y += vys;

			map_x = (int)(current_point.x - (looks_right ? 0 : EPS));
			map_y = (int)(current_point.y - (looks_up ? EPS : 0));
		}
	}


	//horizontal
	//yn/xn distance
	double hyn = -(player.y - floor(player.y));
	if (not looks_up) {
		hyn += CELLSIZE;
	}

	double hxn = hyn / tan(to_rad(player.angle));

	double hys = CELLSIZE;
	if (looks_up) {
		hys = -hys;
	}

	double hxs = hys / tan(to_rad(player.angle));

	//checking intersections
	bool hor_intersection = false;
	double horizontal_distance = 1000;

	int closest_hx = int(player.x + hxn - (looks_up? EPS : 0));
	int closest_hy = int(player.y + hyn - (looks_right? 0 : EPS));

	Vector2 closest_h{ player.x + hxn, player.y + hyn};

	if (closest_hy < MAP_SIZE and closest_hx < MAP_SIZE and closest_hy >= 0 and closest_hx >= 0 and map[closest_hy][closest_hx] - '0') {
		hor_intersection = true;
		horizontal_distance = dist(player.pos, closest_h);
	} else {
		Vector2 current_point = closest_h;

		int map_x = (int)(current_point.x + (looks_right ? 0 : -EPS));
		int map_y = (int)(current_point.y + (looks_up ? -EPS : 0));

		while (map_x < MAP_SIZE and map_y < MAP_SIZE and map_x >= 0 and map_y >= 0 and dist(current_point, player.pos) <= MAX_DIST) {
			if (map[map_y][map_x] - '0') {
				hor_intersection = true;
				horizontal_distance = dist(current_point, player.pos);
				break;
			}

			current_point.x += hxs;
			current_point.y += hys;

			map_x = (int)(current_point.x + (looks_right ? 0 : -EPS));
			map_y = (int)(current_point.y + (looks_up ? -EPS : 0));
		}


	}

	//выбираем минимальное из расстояний, если одного нет, то автоматически выбираем другое (так как в непересеченном направлении
	//стоит 1000, что больше любого расстояние на самом деле
	double return_dist = min(horizontal_distance, vertical_distance);
	return return_dist;
}

//Функция, испускающая лучи
vector<double> cast_rays(Player player) {
	const int fov_deg = to_deg(FOV);

	//Количество лучей и шаг между ними
	const double step = fov_deg / sw;
	const int num_rays = sw;

	//изначальный угол, под которым смотрел игрок
	double legacy_player = player.angle;

	//начало - слева, player.angle - середина
	const double start = player.angle - fov_deg/2;
	player.angle = start;


	vector<double> ans;

	for (int i = 0; i < num_rays; i++) {
		double euclidean_dist = dda(player);
		double final_dist = euclidean_dist * cos(to_rad(legacy_player) - to_rad(player.angle));

		ans.push_back(final_dist);
		player.angle += step;
	}

	return ans;
}

void set_cursor(int x, int y) {
	COORD coord = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}


int main() {
	//настройка вывода в консоли под UTF-16
	_setmode(_fileno(stdout), _O_U16TEXT);

	//оптимизация вывода
	cin.tie(nullptr);
	ios_base::sync_with_stdio(false);
	wcout.rdbuf()->pubsetbuf(nullptr, 4096);

	//считывание ASCII-рисунка
	vector<wstring> win_text;
	wifstream wtxt("wintext.txt");
	wstring ws_read_wtxt;

	while (getline(wtxt, ws_read_wtxt)) {
		win_text.push_back(ws_read_wtxt);
	}

	//заранее создаем кадр для оптимизации
	wstring frame;
	frame.reserve(8192);


	//задавание стартовых координат и данных игрока
	double start_angle = 0;
	Player player{ 1, 1, start_angle };

	auto current_time = chrono::steady_clock::now();

	while (true) {
		//FPS
		auto new_time = chrono::steady_clock::now();
		double delta_time = chrono::duration<double>(new_time - current_time).count();
		current_time = new_time;
		double FPS = 1.0 / delta_time;

		double rad = to_rad(player.angle);
		//скорость игрока
		const double speed = 0.1;
		const double player_size = 0.005;

		// Движение "Вперед/Назад"
		if (GetAsyncKeyState('W') & 0x8000) {
			double next_x = player.x + cos(rad) * speed;
			double next_y = player.y + sin(rad) * speed;

			next_x = (next_x + (cos(rad) > 0 ? player_size : -player_size));
			next_y = (next_y + (sin(rad) > 0 ? player_size : -player_size));

			// Простая проверка столкновений
			if (next_x < MAP_SIZE and next_y < MAP_SIZE and map[(int)next_y][(int)next_x] == '0') {
				player.x = next_x;
				player.y = next_y;
			}
		}
		if (GetAsyncKeyState('S') & 0x8000) {
			double next_x = player.x - cos(rad) * speed;
			double next_y = player.y - sin(rad) * speed;

			next_x = (next_x + (cos(rad) > 0 ? -player_size : player_size));
			next_y = (next_y + (sin(rad) > 0 ? -player_size : player_size));

			if (next_x < MAP_SIZE and next_y < MAP_SIZE and map[(int)next_y][(int)next_x] == '0') {
				player.x = next_x;
				player.y = next_y;
			}
		}
		//rotation via key arrows
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			player.angle += 5;

			if (player.angle >= 360) {
				player.angle -= 360;
			}

		} else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			player.angle -= 5;

			if (player.angle < 0) {
				player.angle += 360;
			}
		}
		

		player.pos = {player.x, player.y};

		//информация получена в гайде от: Vectozavr
		//расстояния до стен в пределах FOV
		vector<double> distances = cast_rays(player);

		//расстояние до экрана
		const int dx = 2;
		//размер стены
		const int b = HEIGHT;
		const int mid = HEIGHT / 2;

		//отображаемая в консоли сетка
		//"-" -пусто, "#" - стена
		wstring map_grid[sh];

		for (int i = 0; i < sh; i++) {
			map_grid[i] = wstring();

			for (int j = 0; j < sw; j++) {
				if (i < sh/2) {
					map_grid[i].push_back(' ');
				} else {
					map_grid[i].push_back('.');
				}
				
			}
		}

		for (int i = 0; i < sw; i++) {
			//расстояние до рассматриваемой точки
			double d = distances[i];

			//высота точки
			double line_height = b / (d + EPS);

			//верхняя и нижняя часть стены; остальное - пол/небо
			int upper_wall = mid - (line_height / 2);
			int lower_wall = mid + (line_height / 2);

			//верхняя и нижняя часть стены в масштабе
			int s_upwall = upper_wall / scale;
			int s_lowall = lower_wall / scale;

			//чем дальше стена (т.е больше d), тем бледнее символ
			//проходимся от самого верха рассчитанной стены до самого низа
			for (int y = s_upwall; y < s_lowall; y++) {
				if (y >= 0 and y < sh) {
					if (d <= 1) {
						map_grid[y][i] = L'█';
					} else if (d > 1 and d <= 2) {
						map_grid[y][i] = L'▓';
					} else if (d > 2 and d <= 3) {
						map_grid[y][i] = L'▒';
					} else if (d < MAX_DIST) {
						map_grid[y][i] = L'░';
					}
				}
				
			}
		}

		//очищаем консоль
		//собираем отрендеренный кадр
		

		for (wstring s : map_grid) {
			frame += s + L'\n';
		}

		//добавляем "мини-карту"
		std::wstring minimap[MAP_SIZE];
		
		for (int i = 0; i < MAP_SIZE; i++) {
			for (char c : map[i]) {
				minimap[i].push_back(c);
			}
		}

		minimap[(int) player.y][(int) player.x] = L'P';

		frame += L'\n\n';

		for (wstring ws : minimap) {
			frame += ws + L'\n';
		}

		//отладочная информация
		string dir_s;

		if (player.angle >= 0 and player.angle < 90) {
			dir_s = ("right and up");
		} else if (player.angle >= 90 and player.angle < 180) {
			dir_s = ("left and up");
		} else if (player.angle >= 180 and player.angle < 270) {
			dir_s = ("left and down");
		} else {
			dir_s = ("right and down");
		}

		string debug;


		debug += "DEBUG INFO: ";
		debug += "angle: "; debug += to_string(player.angle); 
		debug += " direction: "; debug += dir_s;
		debug += " x: "; debug += to_string(player.x);
		debug += " y: "; debug += to_string(player.y);
		debug += " FPS: "; debug += to_string(FPS);

		frame += wstring(debug.begin(), debug.end());

		//выводим отрендеренный кадр
		wcout << frame << endl;

		frame.clear();


		set_cursor(0, 0);

		//задерживаем время между кадрами

		//инициализация победы
		if ((int)player.x == MAP_SIZE - 2 and (int)player.y == MAP_SIZE - 1) {
			wcout << endl;
			for (wstring wsa : win_text) {
				wcout << wsa << endl;
			}

			break;
		}
	}
}