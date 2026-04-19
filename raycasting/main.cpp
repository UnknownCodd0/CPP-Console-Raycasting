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
#include <cassert>
#include <thread>
#include <utility>
#include <locale>
#include <random>
#include <sstream>


//свои заголовочные файлы
//файл игровых типов (game::Color, game::Player, game::Vector2)
#include "game_types.h"
#include "welcome_output.h"


/*#define and &&
#define not !
#define or ||*/

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
const double FOV = PI / 2.5;
const int WIDTH = 800;
const int HEIGHT = 450;
const int CELLSIZE = 1;
const int MAP_SIZE = 20;
const double EPS = 0.0000001;
const int MAX_DIST = 6;
const int FPS_CAP = 120;
const double TARGET_FRAME_TIME = 1.0 / FPS_CAP;

//scale - масштаб. Должно изменяться ТОЛЬКО и ТОЛЬКО в main() при начальном экране при конфигурации игроком
int scale = 7;
//sw и sh - масштабированные ширина и высота. Должны быть пересчитаны в main при изменении scale
int sw = WIDTH / scale, sh = HEIGHT / scale;

//Количество живых врагов
int enemys_left = 8;

//Отладочная переменная. Если true, то играется начальный экран, иначе нет. По умолчанию должно стоять true
const bool play_intro = true;

//Карта в начале игры
const string start_map[MAP_SIZE] = {
	"11111111111111111111",
	"10000000000000000001",
	"10000001011101111111",
	"10000001011101111111",
	"10000011011100000001",
	"10110011000000000001",
	"10110000000000000001",
	"10010000011101101101",
	"10000000011111101101",
	"10111111011100000101",
	"10000111011100010101",
	"10010001100001001101",
	"10000011100001101101",
	"10110000000000101101",
	"10110111111111101101",
	"10000000000000001101",
	"10000000000000001101",
	"10000000000111111101",
	"11111111111111111101",
	"11111111111111111101"
};

//Coordinates move from: 0, 0 - top left corner; 5, 5 - downmost right corner.
//0 - пусто, 1 - стена, 2 - враг
string game_map[MAP_SIZE] = {
	"11111111111111111111",
	"10000000000000000001",
	"10000001011101111111",
	"10000001011101111111",
	"10000011011100000001",
	"10110011000000000001",
	"10110000000000000001",
	"10010000011101101101",
	"10000000011111101101",
	"10111111011100000101",
	"10000111011100010101",
	"10010001100001001101",
	"10000011100001101101",
	"10110000000000101101",
	"10110111111111101101",
	"10000000000000001101",
	"10000000000000001101",
	"10000000000111111101",
	"11111111111111111101",
	"11111111111111111101"
};

//Звуки. Частота звуков
const DWORD sound_footstep = 100;
const DWORD sound_shoot = 80;
const DWORD notifcation_exit = 1000;

const int sound_length_step = 10;
const int sound_length_shoot = 200;
const int sound_length_notifcation = 100;


//!!!!!!!!!!!!!!!УТИЛИТНЫЕ ФУНКЦИИ & КЛАССЫ

//colored output logic
//next two functions use ANSI logic to color the output
wstring SetColor(game::Color textColor)
{
	return L"\033[" + to_wstring((int) textColor) + L"m";
}

wstring ResetColor() { 
	return L"\033[0m"; 
}


double to_rad(double deg) {
	return (deg * PI) / (double) 180;
}

double to_deg(double rad) {
	return rad * 180 / PI;
}


double dist(game::Vector2 p1, game::Vector2 p2) {
	return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

//Асинхронный Beep
void AsyncBeep(DWORD freq, int duration) {
	std::thread([freq, duration]() {
		Beep(freq, duration);
	}).detach();
}


//!!!!!!!!!!!ОСНОВНАЯ ЛОГИКА

//"Сердце" всего raycasting. Расчитывает длину до ближайшего объекта (точки 1 или 2 на карте)
//Возвращает расстояние до объекта и его тип
game::RayResult dda(game::Player player) {
	//DDA

	//заключаем угол между 0 и 360
	while (player.angle < 0) {
		player.angle += 360;
	}

	while (player.angle >= 360) {
		player.angle -= 360;
	}

	//смотрит ли игрок влево/вправо
	bool looks_up = player.angle > 180 and player.angle < 360;
	bool looks_right = not (player.angle > 90 and player.angle < 270);

	//vertical
	//EXPLANATION FOR VARIABLES
	//h/v prefix means that variable is for horizontal or vertical intersection checking
	//xn/yn are variables that mean distance from player to the nearest integer vert/horizont line
	//xs/ys are variabes that mean distance from (xn; yn) to the next integer very/horizont line

	//calculating distance
	//!!!all formulas are in that one video and Gemini and Deepseek chat

	double vxn = -(player.x - floor(player.x));

	//padding for segments
	const double padding = 0.05;
	
	//if tan is 0/90/180/270/360 we get an error. so we slightly change it
	if (abs(player.angle - 0) <= padding or abs(player.angle - 90) <= padding or abs(player.angle - 180) <= padding
		or abs(player.angle - 270) <= padding or abs(player.angle - 360) <= padding) {
		//the closest wrong angle to the player
		int closest = -1;

		//checking the closest layer by using the same interval method
		if (abs(player.angle - 0) <= padding) {
			closest = 0;
		} else if (abs(player.angle - 90) <= padding) {
			closest = 90;
		} else if (abs(player.angle - 180) <= padding) {
			closest = 180;
		} else if (abs(player.angle - 270) <= padding) {
			closest = 270;
		} else if (abs(player.angle - 360) <= padding) {
			closest = 360;
		}

		//fixing the player angle
		//making sure the player.angle is never within the dangerous zone
		//while (abs(player.angle - closest) <= padding) {
		if (player.angle >= closest) {
			player.angle -= padding;
		} else {
			player.angle += padding;
		}
		//}
	}

	//заключаем угол между 0 и 360 (заново, после корректировок)
	while (player.angle < 0) {
		player.angle += 360;
	}

	while (player.angle >= 360) {
		player.angle -= 360;
	}

	//Возвращаемый объект. Содержит расстояние до объекта и тип объекта
	game::RayResult ret_value = {1000, 0, -1, -1};

	//Копии таких объектов для вертикали и горизонтали
	game::RayResult vray = {1000, 0, -1, -1};
	game::RayResult hray = {1000, 0, -1, -1};

	//!!МАТЕМАТИЧЕСКИЙ РАСЧЕТ

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
	game::Vector2 closest_v{ player.x + vxn, player.y + vyn};

	//the closest_vy and vx checks are because sometimes the ray might travel past walls
	//here we check if the closest integer tile for a wall
	if (closest_vy < MAP_SIZE and closest_vx < MAP_SIZE and closest_vy >= 0 and closest_vx >= 0 and game_map[closest_vy][closest_vx] - '0') {
		vert_intersection = true;
		vertical_distance = dist(closest_v, player.pos);
		vray = { vertical_distance, game_map[closest_vy][closest_vx] - '0', closest_vx, closest_vy, true, closest_v.x, closest_v.y};
	//if we couldn't find, then we look through the map
	} else {
		//текущая точка
		game::Vector2 current_point = closest_v;

		//координаты целых чисел, которые проверяем. Отнимаем эпсилон, если смотрит влево/вверх, чтобы "заглянуть" внутрь клетки из-за 
		//целочисленной логики
		int map_x = (int)(current_point.x - (looks_right ? 0 : EPS));
		int map_y = (int)(current_point.y - (looks_up ? EPS : 0));

		while (map_x < MAP_SIZE and map_y < MAP_SIZE and map_x >= 0 and map_y >= 0 and dist(current_point, player.pos) <= MAX_DIST) {
			//если мы врезались в какой-то объект (>0)
			if (game_map[map_y][map_x] - '0') {
				vert_intersection = true;
				vertical_distance = dist(current_point, player.pos);
				vray = {vertical_distance, game_map[map_y][map_x] - '0', map_x, map_y, true, current_point.x, current_point.y};
				break;
			}

			//прибавляем расстояние, т.е двигаем луч
			current_point.x += vxs;
			current_point.y += vys;

			//обновляем
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

	game::Vector2 closest_h{ player.x + hxn, player.y + hyn};

	if (closest_hy < MAP_SIZE and closest_hx < MAP_SIZE and closest_hy >= 0 and closest_hx >= 0 and game_map[closest_hy][closest_hx] - '0') {
		hor_intersection = true;
		horizontal_distance = dist(player.pos, closest_h);
		hray = { horizontal_distance, game_map[closest_hy][closest_hx] - '0', closest_hx, closest_hy, false, closest_h.x, closest_h.y};
	} else {
		game::Vector2 current_point = closest_h;

		int map_x = (int)(current_point.x + (looks_right ? 0 : -EPS));
		int map_y = (int)(current_point.y + (looks_up ? -EPS : 0));

		while (map_x < MAP_SIZE and map_y < MAP_SIZE and map_x >= 0 and map_y >= 0 and dist(current_point, player.pos) <= MAX_DIST) {
			if (game_map[map_y][map_x] - '0') {
				hor_intersection = true;
				horizontal_distance = dist(current_point, player.pos);

				hray = {horizontal_distance, game_map[map_y][map_x] - '0', map_x, map_y, false, current_point.x, current_point.y};

				break;
			}

			current_point.x += hxs;
			current_point.y += hys;

			map_x = (int)(current_point.x + (looks_right ? 0 : -EPS));
			map_y = (int)(current_point.y + (looks_up ? -EPS : 0));
		}


	}
	//выбираем минимальное из расстояний, если одного нет, то выбираем другое. Если оба не пересеклись, то возвращаем {1000, 0}, что гарантирует,
	//что стена не будет отрендерена
	if (hor_intersection and not vert_intersection) {
		ret_value = hray;
	} else if (vert_intersection and not hor_intersection) {
		ret_value = vray;
	} else {
		if (vray.dist < hray.dist) {
			ret_value = vray;
		} else {
			ret_value = hray;
		}
	}

	return ret_value;
}


class Pistol {
public:
	//размеры обычного пистолета
	const int pistol_width = 19;
	const int pistol_height = 21;

	//высота стреляющего пистолета (ширина обоих равна)
	const int pistolfire_height = 23;

	//длина луча выстрела пистолета
	const int MAX_SHOOT = 4;

	//ASCII-содержание рисунка
	std::vector<std::wstring> pistol_body;
	//файл с рисунком
	std::wifstream gun_file;
	//Вектор, содержащий целочисленные значения индексов последних реальных символ строки пистолета. 
	//Дальше идут пробелы, которые есть чисто ради выравнивания длин строк, и которые не должны быть закрашены.
	std::vector<int> pistol_last_symbol;

	//Те же самые 3 переменные, но для ASCII-арта стреляющего пистолета
	std::vector<std::wstring> pistol_fire_body;
	std::ifstream gun_fire_file;
	std::vector<int> pistol_fire_last_symbol;
	
	//координаты, с которых начинается пистолет
	int pistol_start_y;
	int pistol_start_x;

	int pistol_fire_start_y;

	//координаты, с которых стреляет (пускает луч) пистолет (близко к дулу)
	int shoot_x = pistol_start_x + sw / 15;
	int shoot_y = pistol_start_y;

	//Перезарядка пистолета
	const int COOLDOWN_TIME = 5;
	double current_cd = 0;

	bool shooting = false;
	double shooting_animation_length = 0.5;
	double shooting_animation_for = 0;


	//Конструктор
	Pistol(std::string filename, std::string filename_fire) {
		//открываем файл с указанным названием
		gun_file.open(filename);
		gun_fire_file.open(filename_fire);

		//Текущая строка в файле
		std::wstring current_line_gun;
		std::string current_line_gunfire;


		//считываем аски-рисунок из файла
		while (std::getline(gun_file, current_line_gun)) {
			pistol_body.push_back(std::wstring(current_line_gun.begin(), current_line_gun.end()));
			//добавляем в вектор индекс последнего встречного непробельного символа
			pistol_last_symbol.push_back(pistol_body.back().size() - 1);

			//выравниваем все строки пробелами под один и тот же размер (макс. строки). Посчитал вручную размер максимальной строки
			while (pistol_body.back().size() < pistol_width) {
				pistol_body.back().push_back(' ');
			}
		}

		//Закрываем открытый файл
		gun_file.close();

		//та же логика, но для стреляющего пистоля
		while (std::getline(gun_fire_file, current_line_gunfire)) {
			pistol_fire_body.push_back(std::wstring(current_line_gunfire.begin(), current_line_gunfire.end()));
			pistol_fire_last_symbol.push_back(pistol_fire_body.back().size() - 1);

			while (pistol_fire_body.back().size() < pistol_width) {
				pistol_fire_body.back().push_back(' ');
			}
		}

		gun_fire_file.close();

		//гарантируем, что пистолет поместится и будет чуть выше низа/правого конца экрана на 1/10 часть
		pistol_start_y = sh - pistol_height - sh / 10;
		pistol_start_x = sw - pistol_height - sw / 10;

		pistol_fire_start_y = sh - pistolfire_height - sh/10;
	}


	//Функция, реализующая выстрел пистолета
	//Напрямую изменяет карту, если мы попали
	void shoot(game::Player player, vector<game::Enemy> &enemys) {
		//Если пистолет не перезарядился, то он не стреляет
		if (current_cd > 0) {
			return;
		}

		shooting = true;
		

		//Включаем звук выстрела
		AsyncBeep(sound_shoot, sound_length_shoot);

		//Обновляем перезарядку
		current_cd = COOLDOWN_TIME;

		//Испускаем луч и измеряем расстояние до ближайшего объекта (может быть и стеной и врагом)
		game::RayResult result = dda(player);
		double dist = result.dist;

		//Если расстояние больше допустимого, до значит точно не попал
		if (dist > this->MAX_SHOOT) {
			return;
		}

		//Проверяем, попали ли мы по врагу
		if (result.type == 2) {
			game_map[result.intersection_y][result.intersection_x] = '0';
			enemys_left--;

			if (enemys_left == 0) {
				AsyncBeep(notifcation_exit, sound_length_notifcation);
			}

			//Удаляем врага из списка
			for (game::Enemy en : enemys) {
				//Проверяем, попали ли мы по итерируемому врагу
				if (en.x == result.intersection_x and en.y == result.intersection_y) {
					//Тогда мы уже проходим переменной i, и если враг совпал с итерируемым сверху врагом, значит этот враг сидит на
					//Позиции i в списке врагов, и тогда мы его удаляем по итератору begin() + i
					for (int i = 0; i < enemys.size(); i++) {
						if (enemys[i].id == en.id) {
							enemys.erase(enemys.begin() + i);

							break;
						}
					}
					break;
				}
			}
		}

		return;
	}

	//Перезарядка пистолета после выстрела. Вызывается каждый кадр, реальная перезарядка идет только если она не закончилась, т.е. больше нуля
	void reload(double deltaTime) {
		if (current_cd > 0) {
			current_cd -= deltaTime;
		}
	}
};




//Функция, испускающая множество лучей в зоне видимости (FOV) игрока
vector <game::RayResult> cast_rays(game::Player player) {
	const int fov_deg = to_deg(FOV);

	//заключаем угол игрока между 0 и 360
	while (player.angle < 0) {
		player.angle += 360;
	}

	while (player.angle >= 360) {
		player.angle -= 360;
	}

	//Количество лучей и шаг между ними
	const double step = double(fov_deg) / double(sw);
	const int num_rays = sw;

	//изначальный угол, под которым смотрел игрок
	double legacy_player = player.angle;

	//начало - слева, player.angle - середина
	double start = player.angle - fov_deg/2;

	//нормализуем стартовый угол
	while (start < 0) {
		start += 360;
	}

	while (start >= 360) {
		start -= 360;
	}

	//назначаем угол игрока в стартовый, потом будем его двигать по всему FOV.
	player.angle = start;

	//массив расстояний лучей и тип пересечения
	vector<game::RayResult> ans;

	//пускаем лучи
	for (int i = 0; i < num_rays; i++) {
		//сначала считаем евклидово расстояние
		game::RayResult current_intersection = dda(player);
		double euclidean_dist = current_intersection.dist;

		//умножаем на косинус, чтобы избежать "рыбьего глаза", то есть разного расстояние до одной и той же стены
		double final_dist = euclidean_dist * cos(to_rad(legacy_player) - to_rad(player.angle));

		ans.push_back({final_dist, current_intersection.type, current_intersection.intersection_x, current_intersection.intersection_y, current_intersection.was_vertical, current_intersection.d_ix, current_intersection.d_iy});

		//двигаем игрока
		player.angle += step;
	}

	return ans;
}



//Функиця скоростного очищения экрана
void set_cursor(int x, int y) {
	COORD coord = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}


//Функция отвечает за вывод начального экрана и интерактива с ним. Также устанавливает размер экрана и количество врагов
void start_menu() {
	if (play_intro) {
		DWORD written = 0;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		while (true) {
			WriteConsoleA(hConsole, start_controls_welcome.c_str(), start_controls_welcome.size(), &written, NULL);
			set_cursor(0, 0);

			if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
				system("cls");
				break;
			}
		}

		this_thread::sleep_for(chrono::milliseconds(200));
		string current_choose_string = difficulty_easy;

		while (true) {
			WriteConsoleA(hConsole, current_choose_string.c_str(), current_choose_string.size(), &written, NULL);

			set_cursor(0, 0);

			if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
				if (current_choose_string == difficulty_easy) {
					current_choose_string = difficulty_medium;
					system("cls");
				} else if (current_choose_string == difficulty_medium) {
					current_choose_string = difficulty_hard;
					system("cls");
				}
			}

			if (GetAsyncKeyState(VK_UP) & 0x8000) {
				if (current_choose_string == difficulty_hard) {
					current_choose_string = difficulty_medium;
					system("cls");
				} else if (current_choose_string == difficulty_medium) {
					current_choose_string = difficulty_easy;
					system("cls");
				}
			}

			if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
				system("cls");
				break;
			}
		}

		enemys_left = configuration_table.at(current_choose_string);

		this_thread::sleep_for(chrono::milliseconds(200));
		current_choose_string = resolution_low;

		while (true) {
			WriteConsoleA(hConsole, current_choose_string.c_str(), current_choose_string.size(), &written, NULL);

			set_cursor(0, 0);

			if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
				if (current_choose_string == resolution_low) {
					current_choose_string = resolution_normal;
					system("cls");
				} else if (current_choose_string == resolution_normal) {
					current_choose_string = resolution_high;
					system("cls");
				}
			}

			if (GetAsyncKeyState(VK_UP) & 0x8000) {
				if (current_choose_string == resolution_high) {
					current_choose_string = resolution_normal;
					system("cls");
				} else if (current_choose_string == resolution_normal) {
					current_choose_string = resolution_low;
					system("cls");
				}
			}

			if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
				system("cls");
				break;
			}
		}

		scale = configuration_table.at(current_choose_string);
		sh = HEIGHT / scale;
		sw = WIDTH / scale;
	}
}


int main() {
	start_menu();

	srand(time(NULL));

	//РЕНДЕРИНГ && ДВИЖЕНИЕ
	//Оптимизация вывода
	cin.tie(nullptr);
	ios_base::sync_with_stdio(false);
	wcout.rdbuf()->pubsetbuf(nullptr, 8192);

	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // Включает поддержку \033
	dwMode |= DISABLE_NEWLINE_AUTO_RETURN;       // Убирает перенос строки, который портит последний символ
	SetConsoleMode(hOut, dwMode);

	//заранее создаем кадр для оптимизации вывода
	wstring frame;

	//освобождаем достаточно места
	frame.reserve(65536 * 2);

	//задавание стартовых координат и данных игрока
	double start_angle = 0;
	//1.1 чтобы игрок не спавнился в стене
	game::Player player{ 1.1, 1.1, start_angle };
	const double speed = 1.2;
	const double rot_speed = 60;
	const double player_size = EPS;

	auto current_time = chrono::steady_clock::now();

	//очищаем консоль от прочего текста и переносим курсор в ноль
	system("cls");
	set_cursor(0, 0);

	//Инициализируем класс пистолета
	Pistol pistol{ "gun.txt", "gun_fire.txt" };

	//Вектор ascii прямоугольников рендеринга
	//Чем больше значение в квадратных скобках, тем бледнее квадрат. Всего 4.
	vector<wchar_t> rectangles = { L'█', L'▓', L'▒', L'░' };

	//Перезарядка звука шага
	const double STEP_COOLDOWN = 0.7;
	double current_step_cooldown = 0;

	//Текстура врага (одинаковая у всех)
	game::Texture enemy_texture("enemy_texture.txt");

	//инициализация врагов. 9 врагов. В зависимости от сложности несколько из них рандомно попадут в игру
	game::Enemy enemy1(18, 1, 0, 1, enemy_texture);
	game::Enemy enemy2(8, 1, 1, 0.5, enemy_texture);
	game::Enemy enemy3(4, 5, 2, 0.7, enemy_texture);
	game::Enemy enemy4(12, 11, 3, 0.2, enemy_texture);
	game::Enemy enemy5(9, 16, 4, 0, enemy_texture);
	game::Enemy enemy6(1, 10, 5, 0.9, enemy_texture);
	game::Enemy enemy7(14, 5, 6, 0.3, enemy_texture);
	game::Enemy enemy8(18, 18, 7, 1, enemy_texture);
	game::Enemy enemy9(13, 13, 8, 0.1, enemy_texture);

	vector<game::Enemy> all_enemys = { enemy1, enemy2, enemy3, enemy4, enemy5, enemy6, enemy7, enemy8, enemy9 };

	//Список всех живых на данный момент врагов. При смерти врага он будет удален из списка в классе Pistol
	vector<game::Enemy> enemys;

	//Инициализируем логику текстур стен
	game::Texture wall_texture("wall_texture.txt");

	//Собираем список всех врагов при помощи рандома
	while (enemys.size() < enemys_left) {
		int random_enemy = rand() % 9;
		game::Enemy enemy_to_add;

		bool chosen_first = false;

		//Проверяем, не выбрали ли мы этого врага. Если нет, то берем и удаляем из списка доступных.
		for (int i = 0; i < all_enemys.size(); i++) {
			if (random_enemy == all_enemys[i].id) {
				enemy_to_add = all_enemys[i];
				all_enemys.erase(all_enemys.begin() + i);
				chosen_first = true;
				break;
			}
		}

		//Если уже выбрали, то добавляем, и будем пытаться добавить заново в следующем проходе
		if (chosen_first) {
			enemys.push_back(enemy_to_add);
			continue;
		}

		bool cont = true;

		//Если вдруг с первого раза не получилось выбрать уникального, то повторяем так, пока не выберем
		while (cont) {
			int random_enemy = rand() % 9;

			for (int i = 0; i < all_enemys.size(); i++) {
				if (random_enemy == all_enemys[i].id) {
					enemy_to_add = all_enemys[i];
					all_enemys.erase(all_enemys.begin() + i);
					cont = false;
					break;
				}
			}
		}

		enemys.push_back(enemy_to_add);
	}

	//Означает, выиграл ли игрок или проиграл
	bool won = true;


	//Основной цикл игры (mainloop)
	while (true) {
		//FPS
		//время сейчас
		auto new_time = chrono::steady_clock::now();
		//Разница во времени между текущим и прошлым кадром
		double delta_time = chrono::duration<double>(new_time - current_time).count();
		//ставим прошлый кадр = текущий, чтобы потом на следующем кадре провернуть вышеуказанный код
		current_time = new_time;

		//мгновенный FPS
		double FPS = 1.0 / delta_time;
		//направление игрока в радианах
		double rad = to_rad(player.angle);
		//скорость игрока
		double real_speed = speed * delta_time;
		//скорость игрока в данном кадре
		double real_rotspeed = rot_speed * delta_time;

		//ПЕРЕЗАРЯДКИ
		//Перезаряжаем пистолет каждый кадр
		pistol.reload(delta_time);

		//"Перезаряжаем" время между шагами игрока
		if (current_step_cooldown > 0) {
			current_step_cooldown -= delta_time;
		}

		//Сокращаем время показа стреляющего пистолета
		if (pistol.shooting) {
			if (pistol.shooting_animation_for < pistol.shooting_animation_length) {
				pistol.shooting_animation_for += delta_time;
			} else {
				pistol.shooting = false;
				pistol.shooting_animation_for = 0;
			}

		}

		//Добавляем на карту врагов, чтобы при рассчетах они не врезались друг в друга
		for (game::Enemy enemy : enemys) {
			game_map[enemy.y][enemy.x] = '2';
		}

		//Позволяем врагам делать ходы и попутно рассчитываем расстояние
		for (game::Enemy& enemy : enemys) {
			enemy.move_cooldown -= delta_time;

			if (enemy.move_cooldown < 0) {
				int prev_x = enemy.x, prev_y = enemy.y;
				enemy.follow_player(player, MAP_SIZE, game_map);

				game_map[prev_y][prev_x] = '0';
			}
		}

		//Обнуляем карту и показываем врагов на новых координатах
		copy(start_map->begin(), start_map->end(), game_map->begin());

		for (game::Enemy enemy : enemys) {
			game_map[enemy.y][enemy.x] = '2';
		}

		//Проверяем, не попал ли враг на блок игрока?
		if (game_map[(int)player.y][(int)player.x] == '2') {
			//Если попал, то игрок проиграл
			won = false;
			break;
		}


		// Движение "Вперед/Назад" по вектору направления
		if (GetAsyncKeyState('W') & 0x8000) {
			//Издаем звук каждую секунду
			if (current_step_cooldown <= 0) {
				AsyncBeep(sound_footstep, sound_length_step);
				current_step_cooldown = STEP_COOLDOWN;
			}

			//двигаем игрока по вектору
			double next_x = player.x + cos(rad) * real_speed;
			double next_y = player.y + sin(rad) * real_speed;

			//Не позволяем игроку вплотную подойти к стене, отнимаем маленькое число, если он пожошел
			next_x = (next_x + (cos(rad) > 0 ? player_size : -player_size));
			next_y = (next_y + (sin(rad) > 0 ? player_size : -player_size));

			// Простая проверка столкновений
			if (next_x < MAP_SIZE and next_y < MAP_SIZE and game_map[(int)next_y][(int)next_x] == '0') {
				//Откатываем изменения, чтобы игрок шел плавно
				next_x = (next_x - (cos(rad) > 0 ? player_size : -player_size));
				next_y = (next_y - (sin(rad) > 0 ? player_size : -player_size));

				player.x = next_x;
				player.y = next_y;
			}
		}
		//Та же логика, что и при движении вперед, только наоборот: двигаемся назад и проверяем стену сзади
		if (GetAsyncKeyState('S') & 0x8000) {
			//Издаем звук каждую секунду
			if (current_step_cooldown <= 0) {
				AsyncBeep(sound_footstep, sound_length_step);
				current_step_cooldown = STEP_COOLDOWN;
			}

			double next_x = player.x - cos(rad) * real_speed;
			double next_y = player.y - sin(rad) * real_speed;

			next_x = (next_x + (cos(rad) > 0 ? -player_size : player_size));
			next_y = (next_y + (sin(rad) > 0 ? -player_size : player_size));

			if (next_x < MAP_SIZE and next_y < MAP_SIZE and game_map[(int)next_y][(int)next_x] == '0') {
				next_x = (next_x - (cos(rad) > 0 ? -player_size : player_size));
				next_y = (next_y - (sin(rad) > 0 ? -player_size : player_size));

				player.x = next_x;
				player.y = next_y;
			}
		}
		//Поворот "Влево-вправо"
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			player.angle += real_rotspeed;

			if (player.angle >= 360) {
				player.angle -= 360;
			}

		} else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
			player.angle -= real_rotspeed;

			if (player.angle < 0) {
				player.angle += 360;
			}
		}

		//Проверка нажатия на пробел и выстрела пистолета
		if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
			pistol.shoot(player, enemys);
		}

		//обновляем game::Vector2 позицию игрока
		player.pos = { player.x, player.y };


		/////!!!!!РЕНДЕРИНГ
		//информация получена в гайде от: Vectozavr
		//расстояния до стен в пределах FOV и их тип пересечения
		vector<game::RayResult> distances = cast_rays(player);

		//расстояние до экрана
		const int dx = 1;
		//размер стены
		const int b = HEIGHT;

		//центр экрана
		const int mid = HEIGHT / 2;

		//Сердце ренгеринга. Хранит в себе данные о небе, поле и стенах.
		vector<vector<wstring>> map_grid(sh, vector<wstring>(sw));

		//Хранит в себе данные о цвете пикселя
		vector<vector<game::Color>> frame_colors_pixels(sh, vector<game::Color>(sw));

		//заполняем небом и полом
		for (int i = 0; i < sh; i++) {
			for (int j = 0; j < sw; j++) {
				//верхняя половина - небо
				if (i < sh / 2) {
					frame_colors_pixels[i][j] = game::CYAN;
					map_grid[i][j] = rectangles[3];
					//пол
				} else {
					frame_colors_pixels[i][j] = game::YELLOW;
					map_grid[i][j] = rectangles[3];
				}

			}
		}

		game::Vector2i last_enemy_pos = { -1, -1 };
		vector<pair<int, game::RayResult>> last_enemy_rays;
		vector<vector<pair<int, game::RayResult>>> enemys_and_their_rays;

		//Заранее предпосчитаем те лучи, которые попали на врагов, и изменяем их до одного расстояния минимального из всех лучей,
		//Чтобы создать плоскую проекцию
		for (int i = 0; i < sw; i++) {
			//Текущий луч
			game::RayResult rr = distances[i];

			//Если луч попал на врага
			if (rr.type == 2) {
				//Если луч не принадлежит прошлому врагу и не принадлежит начальному значению
				if ((rr.intersection_x != last_enemy_pos.x or rr.intersection_y != last_enemy_pos.y)
					and (last_enemy_pos.x != -1 or last_enemy_pos.y != -1)) {
					
					//Обновляем прошлого врага на текущего
					last_enemy_pos.x = rr.intersection_x;
					last_enemy_pos.y = rr.intersection_y;

					//Высчитываем луч с минимальным расстоянием
					double min_ray_dist = 1e9;
					game::RayResult min_ray;

					for (auto j : last_enemy_rays) {
						if (j.second.dist < min_ray_dist) {
							min_ray_dist = j.second.dist;
							min_ray = j.second;
						}
					}

					//Приравниваем расстояния всех лучей к этому для создания плоского изображения
					for (int i = 0; i < last_enemy_rays.size(); i++) {
						auto curr_ray = last_enemy_rays[i].second;
						curr_ray.dist = min_ray_dist;
						//curr_ray.d_ix = min_ray.d_ix;
						//curr_ray.d_iy = min_ray.d_iy;
						//curr_ray.intersection_x = min_ray.intersection_x;
						//curr_ray.intersection_y = min_ray.intersection_y;
						curr_ray.was_vertical = min_ray.was_vertical;
						//auto curr_ray = min_ray;

						last_enemy_rays[i].second = curr_ray;
					}

					//Добавляем группу лучей, указывающих всех на 1 врага
					enemys_and_their_rays.push_back(last_enemy_rays);
					last_enemy_rays.clear();
				} else {
					if (last_enemy_pos.x == -1 and last_enemy_pos.y == -1) {
						last_enemy_pos.x = rr.intersection_x;
						last_enemy_pos.y = rr.intersection_y;
					}

					//Добавляем луч в группу всех лучей, принадлежащих 1 врагу
					last_enemy_rays.push_back({ i, rr });
				}
			}
		}

		//Если мы не смогли записать группу в список всех групп, то записываем её
		if (last_enemy_rays.size() > 0) {
			enemys_and_their_rays.push_back(last_enemy_rays);
			last_enemy_rays.clear();
		}

		//Собираем все пиксели стен на экране (sw - ширина экрана в "пикселях"), но не врагов!
		for (int i = 0; i < sw; i++) {
			//расстояние до рассматриваемой точки
			double d = distances[i].dist;

			//вид объекта, с которым мы столкнулись
			int collision_type = distances[i].type;

			//высота точки. Исходит из подобия треугольников. line_height/dx = HEIGHT/d
			double line_height = b * dx / (d + EPS);

			//верхняя и нижняя часть стены; остальное - пол/небо
			int upper_wall = mid - (line_height / 2);
			int lower_wall = mid + (line_height / 2);

			//верхняя и нижняя часть стены в масштабе
			int s_upwall = upper_wall / scale;
			int s_lowall = lower_wall / scale;

			//Логика с текстурированием. Так как DDA всегда находит целочисленные координаты пересечения либо по X, либо по Y,
			//Мы ориентируемся по нецелой координате, и именно она показывает в какую часть стены по процентам мы попали
			double hit_coord;

			//Если луч попал по вертикальной клетке, то значит, что X - целое, а Y - дробное
			if (distances[i].was_vertical) {
				//Мы берем именно дробную версию Y, чтобы дальше рассчитать по формуле
				hit_coord = distances[i].d_iy;
			} else {
				// Если в горизонтальную — вдоль оси X
				hit_coord = distances[i].d_ix;
			}

			//Начало так называемого UV-маппинга. U - это полоска, по которой попал луч. fmod находит остаток. например,
			//fmod(5.5, 1) = 0.5, т.е мы попали в 50% стены (середину). Это позволяет наложить текстуры на сам объект,
			//А не на его представление в экране
			double U = fmod(hit_coord, 1.0);

			//Координата текстуры, из которой берем пиксель. Умножаем общую ширину текстуры на относительную координату
			//попадания
			int tex_x = (int)(U * wall_texture.texture_hor);


			//чем дальше стена (т.е больше d), тем бледнее символ
			//проходимся от самого верха рассчитанной стены до самого низа
			//в map_grid[y][i] хранится wstring, состоящий из: ANSI-префикс цвета, отрендеренный символ, ANSI-суффикс цвета
			//На самом деле это координата Z, т.е. высоты
			for (int y = s_upwall; y < s_lowall; y++) {
				//V мы пересчитываем в цикле, так как у всех стен равная высота, и чтобы проецировать реальные 3D-лучи,
				//Мы должны пройтись по всей высоте проецируемой стены

				//Немного другая логика: мы делим то, насколько далеко ушли от начала по y (y - s_upwall) на
				//Общую высоту стены (lowall - upwall)
				double V = (double)(y - s_upwall) / (double)(s_lowall - s_upwall);

				//Находим ту же координату по y
				int tex_y = (int)(V * wall_texture.texture_vert);


				//Проверяем, не вылетели ли мы за границы текстуры: не стало ли меньше нуля или больше размера текстуры
				tex_y = max(0, min(wall_texture.texture_vert, tex_y));
				tex_x = max(0, min(wall_texture.texture_hor, tex_x));

				//Проверяем, не вылетел ли y за границы
				if (y >= 0 and y < sh) {
					if (collision_type == 1) {
						if (d <= 1) {
							frame_colors_pixels[y][i] = wall_texture.texture[tex_y][tex_x];
							map_grid[y][i] = rectangles[0];
						} else if (d > 1 and d <= 2) {
							frame_colors_pixels[y][i] = wall_texture.texture[tex_y][tex_x];
							map_grid[y][i] = rectangles[1];
						} else if (d > 2 and d <= 3) {
							frame_colors_pixels[y][i] = wall_texture.texture[tex_y][tex_x];
							map_grid[y][i] = rectangles[2];
						} else if (d < MAX_DIST) {
							frame_colors_pixels[y][i] = wall_texture.texture[tex_y][tex_x];
							map_grid[y][i] = rectangles[3];
						}
					}
				}
			}
		}

		//Теперь мы добавляем врагов на карту по высчитанным лучам
		for (auto enemy_rays : enemys_and_their_rays) {
			//Расстояние до врага (равно для всех лучей одной группы)
			double dist_enemy = enemy_rays[0].second.dist;

			//Индекс текущего луча
			int ray_pos_vec = 0;

			//Рассчитываем ширину спрайта в пикселях экрана (т.е. кол-во лучей в группе)
			int sprite_screen_width = enemy_rays.size();

			for (auto current_en_ray : enemy_rays) {
				//Примерно такая же логика, как и со стенами
				double line_height = b * dx / (dist_enemy + EPS);

				//верхняя и нижняя часть стены; остальное - пол/небо
				int upper_wall = mid - (line_height / 2);
				int lower_wall = mid + (line_height / 2);

				//верхняя и нижняя часть стены в масштабе
				int s_upwall = upper_wall / scale;
				int s_lowall = lower_wall / scale;

				//Так как мы оперируем 2D-объектом, то мы наклеиваем на него текстуры чуть проще (иначе текстуры
				//будут накладываться как на 3D-объект)

				//Вычисляем относительную позицию текущего луча относительно проекции спрайта
				int tex_x = (ray_pos_vec * enemy_texture.texture_hor) / sprite_screen_width;

				for (int y = s_upwall; y < s_lowall; y++) {
					//Логики по вертикали такая же, как и со стенами
					double V = (double)(y - s_upwall) / (double)(s_lowall - s_upwall);
					int tex_y = (int)(V * (enemy_texture.texture_vert));

					tex_y = max(0, min(enemy_texture.texture_vert-1, tex_y));
					tex_x = max(0, min(enemy_texture.texture_hor-1, tex_x));

					if (y >= 0 and y < sh) {
						frame_colors_pixels[y][current_en_ray.first] = enemy_texture.texture[tex_y][tex_x]; //game::RED;
						map_grid[y][current_en_ray.first] = rectangles[0];
					}
				}

				ray_pos_vec++;
			}
		}



		//!!!собираем отрендеренный кадр

		//собираем "мини-карту"
		std::wstring minimap[MAP_SIZE];

		for (int i = 0; i < MAP_SIZE; i++) {
			for (char c : game_map[i]) {
				minimap[i].push_back(c);
			}
		}

		//клетка с игроком
		minimap[(int)player.y][(int)player.x] = L'P';
		//итератор по мини-карте
		int ws_i = 0;
		int ws_y = 0;

		//итератор по пистолету. py и px - это относительные координаты относительно начала пистолета
		int p_y = 0, p_x = 0;

		//Эта переменная говорит, встречали ли мы такую часть пистолета, которая не является пустым местом? Если да, то мы закрашиваем пробелы. Иначе нет
		bool met_pistol_parts = false;

		int prev_colour = -1;

		//сбор основного кадра + мини-карты справа
		//sh и sw - это наше разрешение, которое должно быть заполнено на 100%
		for (int y = 0; y < sh; y++) {
			for (int x = 0; x < sw; x++) {
				//Логика для не стреляющего пистолета
				if (!pistol.shooting) {
					//проверяем, должны ли мы нарисовать пистолет: есть ли по pistol_start_x/y пистолет, не вылез ли он за свой размер/размер экрана
					if (y >= pistol.pistol_start_y and x >= pistol.pistol_start_x
						and p_x < min(pistol.pistol_width, sw) and p_y < min(pistol.pistol_height, sh)) {
						//Проверяем, не является ли символ пробелом. Если нет, то в кадр добавляем часть пистолета, иначе - карту
						if (pistol.pistol_body[p_y][p_x] != ' ') {
							if (!met_pistol_parts) {
								frame += ResetColor();
							}
							frame += pistol.pistol_body[p_y][p_x];
							met_pistol_parts = true;
							//закрашиваем внутренность пистолета, если мы встречали символы-не-пробелы, т.е. мы в середине пистолета, где пробелы означают заливку
						} else if (met_pistol_parts and p_x < pistol.pistol_last_symbol[p_y]) {
							frame += SetColor(game::BLACK) + rectangles[2] + ResetColor();
						} //Иначе добавляем часть карты, так как мы закрасили всё, что нужно, а дальше идут выравнивательные пробелы
						else {
							if (prev_colour == -1) {
								frame += SetColor(frame_colors_pixels[y][x]) + map_grid[y][x];
							} else if (prev_colour != (int)frame_colors_pixels[y][x]) {
								frame += ResetColor() + SetColor(frame_colors_pixels[y][x]) + map_grid[y][x];
							} else {
								frame += map_grid[y][x];
							}
						}

						//Прибавляем p_x в любом случае, так как все, что дальше, является пистолетом
						p_x++;

						//иначе просто добавляем в кадр фрагмент карты
					} else {
						if (prev_colour == -1) {
							frame += SetColor(frame_colors_pixels[y][x]) + map_grid[y][x];
						} else if (prev_colour != (int)frame_colors_pixels[y][x]) {
							frame += ResetColor() + SetColor(frame_colors_pixels[y][x]) + map_grid[y][x];
						} else {
							frame += map_grid[y][x];
						}
					}
					//Отрисовываем стреляющий пистолет
				} else {
					//Та же логика. Но высота двух пистолей разная, ширина - та же
					if (y >= pistol.pistol_fire_start_y and x >= pistol.pistol_start_x
						and p_x < min(pistol.pistol_width, sw) and p_y < min(pistol.pistolfire_height, sh)) {
						if (pistol.pistol_fire_body[p_y][p_x] != ' ') {
							if (!met_pistol_parts) {
								frame += ResetColor();
							}

							//Рисуем эффект выстрела, если ноль
							if (pistol.pistol_fire_body[p_y][p_x] == '0') {
								frame += SetColor(game::YELLOW) + rectangles[0] + ResetColor();
								//Иначе рисуем сам пистолет
							} else {
								frame += pistol.pistol_fire_body[p_y][p_x];
							}

							met_pistol_parts = true;
							//закрашиваем внутренность пистолета
						} else if (met_pistol_parts and p_x < pistol.pistol_fire_last_symbol[p_y]) {
							frame += SetColor(game::BLACK) + rectangles[2] + ResetColor();
						} //Иначе добавляем часть карты, чтобы пистолет был прозрачным
						else {
							if (prev_colour == -1) {
								frame += SetColor(frame_colors_pixels[y][x]) + map_grid[y][x];
							} else if (prev_colour != (int)frame_colors_pixels[y][x]) {
								frame += ResetColor() + SetColor(frame_colors_pixels[y][x]) + map_grid[y][x];
							} else {
								frame += map_grid[y][x];
							}
						}

						//Прибавляем p_x в любом случае, так как все, что дальше, является пистолетом
						p_x++;

						//иначе просто добавляем в кадр фрагмент карты
					} else {
						if (prev_colour == -1) {
							frame += SetColor(frame_colors_pixels[y][x]) + map_grid[y][x];
						} else if (prev_colour != (int)frame_colors_pixels[y][x]) {
							frame += ResetColor() + SetColor(frame_colors_pixels[y][x]) + map_grid[y][x];
						} else {
							frame += map_grid[y][x];
						}
					}
				}

			}

			//Проверяем две начальные точки для стреляющего и нестреляющего пистолета
			if (y >= pistol.pistol_start_y and !pistol.shooting) {
				p_y++;
				p_x = 0;
				met_pistol_parts = false;
			} else if (y >= pistol.pistol_fire_start_y and pistol.shooting) {
				p_y++;
				p_x = 0;
				met_pistol_parts = false;
			}


			//очищаем строку
			//frame += L"\033[K";

			//добавляем мини-карту справа
			if (ws_y > 5 and ws_i < MAP_SIZE) {
				//отступ
				frame += ResetColor();
				frame += L"                 ";
				frame += minimap[ws_i];
				ws_i++;
			}

			ws_y++;

			frame += L'\n';
		}

		//отладочная информация
		string dir_s;

		//расчет угла игрока
		if (player.angle >= 0 and player.angle < 90) {
			dir_s = ("right and up");
		} else if (player.angle >= 90 and player.angle < 180) {
			dir_s = ("left and up");
		} else if (player.angle >= 180 and player.angle < 270) {
			dir_s = ("left and down");
		} else {
			dir_s = ("right and down");
		}

		//сбор отладочной информации
		string debug;

		string str_fps = to_string(FPS);

		//Выравниваем FPS всегда до трехзначного числа, чтобы консоль не мерцала
		if (FPS < 100) {
			str_fps.insert(str_fps.begin(), '0');
		}

		debug += "DEBUG INFO: ";
		debug += "angle: "; debug += to_string(player.angle);
		debug += " direction: "; debug += dir_s;
		debug += " x: "; debug += to_string(player.x);
		debug += " y: "; debug += to_string(player.y);
		debug += " FPS: "; debug += str_fps;
		debug += " Enemys: "; debug += to_string(enemys_left);
		if (enemys_left == 0) {
			debug += ". Now find exit!";
		}

		//превращаем в int и прибавляем 0.9, так как вывод показывает 0, хотя например может быть 0.9, то есть ждать ещё почти секунду
		debug += "\nPistol reload time: "; debug += to_string(max(0, (int)(pistol.current_cd + 0.9)));

		//добавляем ANSI-код удаления строки, а потом на чистую строку добавляем отладочную инфу
		frame += ResetColor();
		frame += L"\033[K";

		//добавляем отладочную инфу
		frame += wstring(debug.begin(), debug.end());

		//Обнуляем карту от врагов, так как вся логика закончилась
		copy(start_map->begin(), start_map->end(), game_map->begin());

		//выводим отрендеренный кадр
		//логика вывода с WinAPI
		DWORD written = 0;
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		//очищаем консоль от прошлых кадров
		set_cursor(0, 0);

		//Выводим текст
		WriteConsoleW(hConsole, frame.c_str(), frame.size(), &written, NULL);

		//очищаем кадр
		frame.clear();

		//задерживаем время между кадрами
		if (delta_time < TARGET_FRAME_TIME) {
			//выводится из формулы: FPS = 1/deltaTime
			double sleepTime = (TARGET_FRAME_TIME - delta_time) * 500;
			if (sleepTime > 1) {  // Не спать меньше 1ms
				this_thread::sleep_for(chrono::milliseconds((int)sleepTime));
			}
		}

		//инициализация победы: заканчиваем цикл если игрок победил
		if ((int)player.x == 18 and (int)player.y == 19 and enemys_left == 0) {
			break;
		}
	}

	//Если игрок победил, выводим ASCII-арт текст "YOU WIN!"
	if (won) {
		//считывание ASCII-рисунка победы
		vector<string> win_text;
		ifstream wtxt("wintext.txt");
		string ws_read_wtxt;

		while (getline(wtxt, ws_read_wtxt)) {
			win_text.push_back(ws_read_wtxt);
		}

		//Вывод цветного текста "YOU WIN!"
		this_thread::sleep_for(chrono::milliseconds(500));

		//Очищаем экран, чтобы туда вывести текст
		system("cls");

		for (string wsa : win_text) {
			wcout << SetColor(game::RED) + wstring(wsa.begin(), wsa.end()) + ResetColor() << endl;
		}

		//Задерживаем, чтобы консоль случайно не закрылась от случайного нажатия клавиши
		this_thread::sleep_for(chrono::milliseconds(1000));

		//Иначе выводим ASCII-арт "YOU LOSE!"
	} else {
		vector<string> lose_text;
		ifstream ltxt("losetext.txt");
		string ws_read_ltxt;

		while (getline(ltxt, ws_read_ltxt)) {
			lose_text.push_back(ws_read_ltxt);
		}

		this_thread::sleep_for(chrono::milliseconds(500));

		//Очищаем экран, чтобы туда вывести текст
		system("cls");

		for (string wsa : lose_text) {
			wcout << SetColor(game::RED) + wstring(wsa.begin(), wsa.end()) + ResetColor() << endl;
		}

		//Задерживаем, чтобы консоль случайно не закрылась от случайного нажатия клавиши
		this_thread::sleep_for(chrono::milliseconds(1000));
	}

	return 0;
}

//КОНЕЦ!