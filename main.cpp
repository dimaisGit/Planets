#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cmath>
using namespace sf;

const double PI = 3.14159265358979323846;
const double G = 6.67e-11;
const int massPerInch = 1000000;

enum {
	HD_W = 1280,
	HD_H = 720,
	FULL_HD_W = 1920,
	FULL_HD_H = 1080
};

struct Planet
{
	Vector2f position;
	Vector2f speed;
	int radius;
	double mass;
	Color color;
};


template <typename T>
T sumVector(const std::vector<T>&);
template <> Vector2<double> sumVector(const std::vector<Vector2<double>>&);
template <typename T>
T getInput(const char* msg, bool range = false, int minValue = 0, int maxValue =10000000, std::string errorMsg = "Please enter a valid value\n");
template <typename T>
void showVector(const std::vector<T>&);
template <> void showVector(const std::vector<Planet>&);
template <> void showVector(const std::vector<Vector2<double>>&);
double circleArea(double);
double getHypotenuseFromVector(const Vector2<double>&);
void drawPlanets(RenderWindow&, const std::vector<Planet>&);
void calculateAccelerations(std::vector<Vector2<double>>&, const std::vector<Planet>&, const std::vector<double>&);
void getInputs(Vector2i &, int&, int&);
void setRandPlanets(std::vector<Planet>&, Vector2i, int);
void setConstants(std::vector<double>&, const std::vector<Planet>&);
void getDistanceAsVector(Vector2<double>&, const Vector2f&, const Vector2f&);
void applyAccelerations(const std::vector<Vector2<double>>&, std::vector<Planet>&, double);
void updatePlanets(std::vector<Planet>&, double);



int main()
{
	srand(time(0));
	Vector2i windowSize;
	int radius;
	int countOfObj;
	getInputs(windowSize, radius, countOfObj);
	std::vector<Planet> planets(countOfObj);//вектор планет
	setRandPlanets(planets, windowSize, radius);
	std::vector<double> constants(countOfObj);//чтобы убрать лишние просчеты, мы сразу определяем для каждой планеты G * m ^ 2
	setConstants(constants, planets);
	std::vector<Vector2<double>> accelerations(countOfObj);
	RenderWindow window(VideoMode(windowSize.x, windowSize.y), "Planets");
	Clock timer;
	double timeFromNow;
	while (window.isOpen())
	{
		timeFromNow = timer.getElapsedTime().asMicroseconds() / 1000000.0;
		timer.restart();
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::Closed)
				window.close();
		}
		if (Mouse::isButtonPressed(Mouse::Left))
			std::cout << Mouse::getPosition(window).x << " " << Mouse::getPosition(window).y << std::endl;
		calculateAccelerations(accelerations, planets, constants);
		applyAccelerations(accelerations, planets, timeFromNow);
		updatePlanets(planets, timeFromNow);
		window.clear();
		drawPlanets(window, planets);
		window.display();
	}

	return 0;
}

void updatePlanets(std::vector<Planet>& planets, double elapsedTime)
{
	for (int i = 0; i < planets.size(); i++)
	{
		planets[i].position.x += planets[i].speed.x * elapsedTime;
		planets[i].position.y += planets[i].speed.y * elapsedTime;
	}
}

void applyAccelerations(const std::vector<Vector2<double>>& accelerations, std::vector<Planet>& planets, double elapsedTime)
{
	for (int i = 0; i < planets.size(); i++)
	{
		planets[i].speed.x += accelerations[i].x * elapsedTime;
		planets[i].speed.y += accelerations[i].y * elapsedTime;
	}
}

void calculateAccelerations(std::vector<Vector2<double>>& accelerations, const std::vector<Planet>& planets, const std::vector<double>& constants)
{
	Vector2<double> distance;
	double R, A, xA, yA;
	std::vector<Vector2<double>> intermediateAccelerations(planets.size());
	for (int i = 0; i < planets.size(); i++)
	{
		intermediateAccelerations[i] = Vector2<double>(0, 0);
		for (int k = 0; k < planets.size(); k++)
		{
			if (i == k)
				continue;
			getDistanceAsVector(distance, planets[i].position, planets[k].position);
			R = getHypotenuseFromVector(distance);
			if (R < planets[i].radius + planets[k].radius)
				R = planets[i].radius + planets[k].radius;
			A = constants[k] / pow(R, 2);
			xA = A * distance.x / R;
			yA = A * distance.y / R;
			xA = planets[i].position.x > planets[k].position.x ? -xA : xA;
			yA = planets[i].position.y > planets[k].position.y ? -yA : yA;
			intermediateAccelerations[k].x = xA;
			intermediateAccelerations[k].y = yA;
		}
		accelerations[i] = sumVector(intermediateAccelerations);
	}
}

template <typename T>
T sumVector(const std::vector<T>& vec)
{
	T sum = vec[0];
	for (int i = 1; i < vec.size(); i++)
		sum += vec[i];
	return sum;
}

template <> Vector2<double> sumVector(const std::vector<Vector2<double>>& vec)
{
	Vector2<double> sum = vec[0];
	for (int i = 1; i < vec.size(); i++)
	{
		sum.x += vec[i].x;
		sum.y += vec[i].y;

	}
	return sum;
}


inline double getHypotenuseFromVector(const Vector2<double>& vector)
{
	return sqrt(pow(vector.x, 2) + pow(vector.y, 2));
}

inline void getDistanceAsVector(Vector2<double>& distance, const Vector2f& position1, const Vector2f& position2)
{
	distance.x = std::max(position1.x, position2.x) - std::min(position1.x, position2.x);
	distance.y = std::max(position1.y, position2.y) - std::min(position1.y, position2.y);
}

void setConstants(std::vector<double>& constants, const std::vector<Planet>& planets)
{
	for (int i = 0; i < constants.size(); i++)
		constants[i] = pow(planets[i].mass, 2) * G;
}

void getInputs(Vector2i& windowSize, int& radius, int& countOfObj)
{
	short windowChoise = getInput<short>(
		"Choose type of window:\n"
		"1.HD (1280x720)\n"
		"2.FullHd (1920x1080)\n",
		true,
		1,
		2
	);
	windowSize = windowChoise == 1 ? Vector2i(HD_W, HD_H) : Vector2i(FULL_HD_W, FULL_HD_H);
	radius = getInput<int>(
		"Enter a max radius value:\n",
		true,
		1
	);
	countOfObj = getInput<int>(
		"Enter a count of obj:\n",
		true,
		1
	);
}

template <typename T>
T getInput(const char* msg, bool range, int minValue, int maxValue, std::string errorMsg)
{
	std::cout << msg;
	T input;
	while (!(std::cin >> input) || range && std::cin && (input < minValue || input > maxValue))
	{
		std::cout << errorMsg;
		std::cin.clear();
		while (std::cin.get() != '\n');
	}
	return input;
}

template <typename T>
void showVector(const std::vector<T>& arr)
{
	for (int i = 0; i < arr.size(); i++)
		std::cout << arr[i] << std::endl;
}

template <> void showVector(const std::vector<Vector2<double>>& arr)
{
	for (int i = 0; i < arr.size(); i++)
		std::cout << "x: " << arr[i].x << " y: " << arr[i].y << std::endl;
}

template <> void showVector(const std::vector<Planet>& arr)
{
	for (int i = 0; i < arr.size(); i++)
	{
		std::cout << "x: " <<  arr[i].position.x << " " << " y: " << arr[i].position.y << std::endl;
		std::cout << "mass: " << arr[i].mass << std::endl;
		std::cout << "radius: " << arr[i].radius << std::endl;
	}
}

void setRandPlanets(std::vector<Planet>& planets, Vector2i windowSize, int radius)
{
	for (int i = 0; i < planets.size(); i++)
	{
		planets[i].position = Vector2f(rand() % windowSize.x, rand() % windowSize.y);
		planets[i].radius = rand() % radius + 1;
		planets[i].mass = massPerInch * circleArea(planets[i].radius);
		planets[i].color = Color::White;
		planets[i].speed = Vector2f(0, 0);
	}
}

inline double circleArea(double radius)
{
	return PI * pow(radius, 2);
}

void drawPlanets(RenderWindow& window, const std::vector<Planet>& planets)
{
	CircleShape circle;
	for (int i = 0; i < planets.size(); i++)
	{
		circle.setFillColor(planets[i].color);
		circle.setRadius(planets[i].radius);
		circle.setPosition(planets[i].position);
		window.draw(circle);
	}
}
