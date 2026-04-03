#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <random>
#include <thread>
#include <fcntl.h>
#include <io.h>

using namespace std;

//CONFIG
const double PI = 3.141592653589793;
const double FOV = PI / 3;
const int WIDTH = 1920;
const int HEIGHT = 1080;
const int CELLSIZE = 1;
const int MAP_SIZE = 6;
const double EPS = 0.00001;
const int MAX_DIST = 6;

//scale - масштаб
const int scale = 40;
//sw и sh - масштабированные ширина и высота
const int sw = WIDTH / scale, sh = HEIGHT / scale;

//Coordinates move from: 0, 0 - top left corner; 5, 5 - downmost right corner.
const string map[MAP_SIZE] = {
	"111111",
	"100001",
	"100101",
	"100101",
	"100001",
	"111111"
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

	if (player.angle == 0 or player.angle == 90 or player.angle == 180 or player.angle == 270 or player.angle == 360) {
		player.angle -= EPS;
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
	int closest_vx = int(player.x + vxn);

	//doing that because int truncates numbers. For example, if ray hit 2, a wall is within [1, 2] but located in map as 1, then we won't be able to detect. by taking off EPS we get 1.999... which is 1 as int
	if (not looks_right) {
		closest_vx -= EPS;
	}
	
	int closest_vy = int(player.y + vyn);

	//the same things but as floats
	Vector2 closest_v{ player.x + vxn, player.y + vyn };

	//the closest_vy and vx checks are because sometimes the ray might travel past walls
	//here we check
	if (closest_vy < MAP_SIZE and closest_vx < MAP_SIZE and closest_vy >= 0 and closest_vx >= 0 and map[closest_vy][closest_vx] - '0') {
		vert_intersection = true;
		vertical_distance = dist(closest_v, player.pos);
	} else {
		Vector2 current_point = closest_v;

		while ((int)current_point.x < MAP_SIZE and (int)current_point.y < MAP_SIZE and current_point.x >= 0 and current_point.y >= 0 and dist(current_point, player.pos) <= MAX_DIST) {
			if (map[(int)current_point.y][(int)current_point.x] - '0') {
				vert_intersection = true;
				vertical_distance = dist(current_point, player.pos);
				break;
			}

			current_point.x += vxs;
			current_point.y += vys;
		}
	}


	//horizontal
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

	int closest_hx = int(player.x + hxn);
	int closest_hy = int(player.y + hyn);

	Vector2 closest_h{ player.x + hxn, player.y + hyn };

	if (closest_hy < MAP_SIZE and closest_hx < MAP_SIZE and closest_hy >= 0 and closest_hx >= 0 and map[closest_hy][closest_hx] - '0') {
		hor_intersection = true;
		horizontal_distance = dist(player.pos, closest_h);
	} else {
		Vector2 current_point = closest_h;

		while ((int)current_point.x < MAP_SIZE and (int)current_point.y < MAP_SIZE and current_point.x >= 0 and current_point.y >= 0 and dist(current_point, player.pos) <= MAX_DIST) {
			if (map[(int)current_point.y][(int)current_point.x] - '0') {
				hor_intersection = true;
				horizontal_distance = dist(current_point, player.pos);
				break;
			}

			current_point.x += hxs;
			current_point.y += hys;
		}


	}

	double return_dist = min(horizontal_distance, vertical_distance);
	return return_dist;
}


vector<double> cast_rays(Player player) {
	const int fov_deg = to_deg(FOV);

	const double step = fov_deg / sw;
	const int num_rays = sw;

	const double start = player.angle - fov_deg/2;
	player.angle = start;

	vector<double> ans;

	for (int i = 0; i < num_rays; i++) {
		ans.push_back(dda(player));
		player.angle += step;
	}

	return ans;
}


int main() {
	//настройка вывода в консоли
	_setmode(_fileno(stdout), _O_U16TEXT);

	//автоповорот камеры
	int start_angle = -5;
	int step = 5;

	while (true) {
		start_angle += 5;

		if (start_angle >= 360) {
			start_angle = 0;
		}

		Player player{ 2, 2, start_angle};

		//информация получена в гайде от: Vectozavr
		//расстояния до стен в пределах FOV
		vector<double> distances = cast_rays(player);

		//расстояние до экрана
		const int dx = 1;
		//размер стены
		const int b = HEIGHT;
		const int mid = HEIGHT / 2;

		//отображаемая в консоли сетка
		//"-" -пусто, "#" - стена
		wstring map_grid[sh];

		for (int i = 0; i < sh; i++) {
			map_grid[i] = wstring();

			for (int j = 0; j < sw; j++) {
				map_grid[i].push_back('-');
			}
		}

		for (int i = 0; i < sw; i++) {
			double d = distances[i];

			//верхняя и нижняя часть стены; остальное - пол/небо
			double line_height = b / (d + EPS);

			int upper_wall = mid - (line_height / 2);
			int lower_wall = mid + (line_height / 2);

			int s_upwall = upper_wall / scale;
			int s_lowall = lower_wall / scale;

			for (int y = s_upwall; y < s_lowall; y++) {
				if (d <= 1) {
					map_grid[y][i] = L'█';
				} else if (d > 1 and d <= 2) {
					map_grid[y][i] = L'▓';
				} else if (d > 2 and d <= 3) {
					map_grid[y][i] = L'▒';
				} else {
					map_grid[y][i] = L'░';
				}
			}
		}

		//"очищаем" консоль
		system("cls");

		for (wstring s : map_grid) {
			wcout << s << '\n';
		}

		this_thread::sleep_for(chrono::milliseconds(200));
	}
}