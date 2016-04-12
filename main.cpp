#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cmath>

using namespace sf;
using namespace std;

vector<String> TileMap = {
	"00000000000000000",
	"0               P",
	"0               P",
	"0               P",
	"0               P",
	"0               P",
	"0               P",
	"0               P",
	"0               P",
	"0               P",
	"0               P",
	"0               P",
	"0G              P",
	"0           d   P",
	"PPPPPPPPPPPPPPPPP",
	"PPPPPPPPPPPPPPPPP",
	"PPPPPPPPPPPPPPPPP",
};

string curMapName = "map1.txt";
int H = 17;//початкові розміри карти
int W = 17;
const double deathTime = 3.;
const double airTime = .22;


bool tileIsSolid(int y, int x) { //функція для перевірки зіткнень 
	if (y < 0 || y >= H || x < 0 || x >= W) return false;//перевірка на належність карти
	char c = TileMap[y][x];
	return c == 'P' || c == 'k' || c == '0' || c == 'r' || c == 't' || c == 'c';
}

Clock clockGame;

double getCurrentSeconds() {
	return clockGame.getElapsedTime().asMicroseconds() / 1000000.;
}

void resetTime(double& time) {
	time = -1000000000.;
}

class Main { // superclass
public:
	double dx, dy;
	FloatRect rect;
	Sprite sprite;
	double currentFrame;
	bool life;	
	bool doGravity = false;
	bool onGround = false;
	bool turnOnCollision = false;
	double timeOfDeath;
	double timeOfJump;

	//функції для визначення положення об'єкта
	int leftCollisionBoxTile() { return int(rect.left) / 16; }
	int rightCollisionBoxTile() { return (ceil(rect.left + rect.width)-1) / 16; }
	int topCollisionBoxTile() { return int(rect.top) / 16; }
	int bottomCollisionBoxTile() { return (ceil(rect.top + rect.height)-1) / 16; }

	void Collision(double time) { //рух об'єктів
		onGround = false;
		rect.left += dx * time;
		if (dx>0) { // moving to the right
			for (int i = topCollisionBoxTile(); i <= bottomCollisionBoxTile(); i++) {
				for (int j = leftCollisionBoxTile(); j <= rightCollisionBoxTile(); j++) {
					if (tileIsSolid(i, j)) { //перевірка на взаємодію з картою 
						rect.left = j * 16 - rect.width;
						if (turnOnCollision)
							dx *= -1;
						else
							dx = 0;
					}
				}
			}
		}
		if (dx < 0)	{ // moving to the left
			for (int i = topCollisionBoxTile(); i <= bottomCollisionBoxTile(); i++) {
				for (int j = leftCollisionBoxTile(); j <= rightCollisionBoxTile(); j++) {
					if (tileIsSolid(i, j)) {
						rect.left = (j + 1) * 16;
						if (turnOnCollision)
							dx *= -1;
						else
							dx = 0;
					}
				}
			}
		}
		if (doGravity) { //обробка стрибків
			dy = dy + 0.0005*time;
		}
		rect.top += dy*time;

		if (doGravity && dy >= 0) { // moving down
			for (int i = topCollisionBoxTile(); i <= bottomCollisionBoxTile(); i++) {
				for (int j = leftCollisionBoxTile(); j <= rightCollisionBoxTile(); j++) {
					if (tileIsSolid(i, j)) {
						rect.top = i * 16 - rect.height;
						dy = 0;
						onGround = true;
					}
				}
			}
		}
		if (doGravity && dy < 0) { // moving up
			for (int i = topCollisionBoxTile(); i <= bottomCollisionBoxTile(); i++) {
				for (int j = leftCollisionBoxTile(); j <= rightCollisionBoxTile(); j++) {
					if (tileIsSolid(i, j)) {
						rect.top = (i + 1) * 16;
						dy = 0;
						resetTime(timeOfJump);
					}
				}
			}
		}
		//вбивати об'єкти, які виходять за межі карти
		if (!(rect.left >= 0 && rect.left + rect.width < W * 16.
			&& rect.top >= 0 && rect.top + rect.height < H * 16.)) {
			kill();
			return;
		}
	}
	void kill() {
		if (life) {
			life = false;
			timeOfDeath = getCurrentSeconds();
		}
	}
	virtual void update(double time) = 0 {
		Collision(time);
	}
	virtual ~Main(){}	
};

class Player : public Main { //клас головного героя
public:
	void Collision(int num)	{
		for (int i = rect.top / 16; i<(rect.top + rect.height) / 16; i++)
			for (int j = rect.left / 16; j<(rect.left + rect.width) / 16; j++)	{
				if (tileIsSolid(i, j))	{
					if (dy>0 && num == 1) {
						rect.top = i * 16 - rect.height;  dy = 0;   onGround = true;
					}
					if (dy<0 && num == 1) {
						rect.top = i * 16 + 16;   dy = 0;
						resetTime(timeOfJump);
					}
					if (dx>0 && num == 0) {
						rect.left = j * 16 - rect.width;
					}
					if (dx < 0 && num == 0)	{
						rect.left = j * 16 + 16;
					}
				}
			}
	}

	Player(Texture &image) {
		//завантажуємо та встановлюємо картинку персонажа 
		sprite.setTextureRect(IntRect(112 + 31 * 0, 144, 16, 16));
		sprite.setTexture(image);
		rect = FloatRect(100, 180, 12, 16);
		dx = dy = 0.1;
		currentFrame = 0;
		life = true;
		resetTime(timeOfDeath);
		resetTime(timeOfJump);
		doGravity = true;
	}

	void update(double time) {
		if (!onGround && getCurrentSeconds() - timeOfJump < airTime) dy = -0.18;
		Main::update(time);
		//зміна картинок з рухом персонажу
		currentFrame += time * 0.005;
		if (currentFrame > 3) currentFrame -= 3;

		if (dx>0) sprite.setTextureRect(IntRect(112 + 31 * int(currentFrame), 144, 16, 16));
		if (dx<0) sprite.setTextureRect(IntRect(112 + 31 * int(currentFrame) + 16, 144, -16, 16));
		// на випадок смерті персонажу
		if (!life) {
			sprite.setColor(Color(0, 255, 0, 255 - (int)255. * (getCurrentSeconds() - timeOfDeath) / deathTime));
		} else {
			sprite.setColor(Color(255, 255, 255, 255));
		}

		sprite.setPosition(rect.left, rect.top);
		dx = 0;
	}	
};

class Enemy : public Main {

public:
	Enemy(Texture &image, int x, int y) {
		sprite.setTexture(image);
		rect = FloatRect(x, y, 16, 16);
		dx = 0.05;
		dy = 0;
		currentFrame = 0;
		life = true;
		resetTime(timeOfDeath);
		resetTime(timeOfJump);
		turnOnCollision = true;
		doGravity = false;
	}	

	void update(double time) {
		Collision(time);
		currentFrame += time * 0.005;
		if (currentFrame > 2) currentFrame -= 2;

		sprite.setTextureRect(IntRect(18 * int(currentFrame), 0, 16, 16));
		if (!life) {
			sprite.setTextureRect(IntRect(58, 0, 16, 16));
			dx = 0;
			sprite.setColor(Color(255, 255, 255, 255 - (int)255. * (getCurrentSeconds() - timeOfDeath) / deathTime));
		}
		sprite.setPosition(rect.left, rect.top);
	}
};

class Block : public Main { //клас для блоків
public:
	Block(Texture &image, int x, int y) {
		sprite.setTexture(image);
		sprite.setTextureRect(IntRect(128, 112, 16, 16));
		rect = FloatRect(x, y, 16, 17);
		dy	= 0.1;	
		life = true;
		resetTime(timeOfDeath);
		resetTime(timeOfJump);
	}
	void update(double time)	{		
		sprite.setPosition(rect.left, rect.top);		
	}
};

std::vector<std::unique_ptr<Main>> enemy; // створеня масиву ворогів
std::vector<std::unique_ptr<Main>> coin; // масив блоків
Texture tileSet; // головна текстура

void loadMapFromFile(const string& s, Player& player) {//завантаження карти 
	ifstream f{s};// зчитування карти
	if (!f.good()) return; // перевірка на успішність зчитування карти
	TileMap.clear(); //очищення карти
	TileMap.emplace_back();
	enemy.clear();
	coin.clear();
	H = 0;
	W = 2000000000;
	char c = '\0';
	int curW = 0;
	// запис карти в масив
	while (!f.eof() && c != char(10)) {
		f.get(c);
		if ((f.eof() || c == char(10)) && curW > 0) {
			TileMap.emplace_back();
			++H;
			W = min(W, curW);
			curW = 0;
			c = char(0);
			continue;
		}
		if (c == char(13)) continue;
		TileMap[H] += c;
		if (c == 'c') coin.push_back(std::make_unique<Block>(tileSet, curW * 16, H * 16));
		if (c == 'e') enemy.push_back(std::make_unique<Enemy>(tileSet, curW * 16, H * 16));
		if (c == 'O') {
			player.rect.top = H * 16;
			player.rect.left = curW * 16;
			player.dx = 0;
			player.dy = 0;
			player.currentFrame = 0;
			player.life = true;
			player.onGround = false;
		}
		++curW;
	}
}

void reloadMap(Player& player) { //перевантаження карти
	loadMapFromFile(curMapName,player);
}


int main() {
	RenderWindow window(VideoMode(800, 600), "Mario"); //створення вікна 
	View view; //створення виду, з масштабом
	view.reset(FloatRect(0, 0, 400, 270)); //масштабування
	tileSet.loadFromFile("Mario_Tileset.png"); //загрузка зображення
	
	Player Mario(tileSet);

	reloadMap(Mario); 

	Sprite tile(tileSet);

	SoundBuffer buffer;  
	buffer.loadFromFile("sound/Jump.ogg"); //загрузка звуків
	Sound sound(buffer);

	Music music;
	music.openFromFile("sound/Mario_Theme.ogg"); //загрузка звуків
	music.play();
	music.setLoop(true);

	Clock clock;
	clockGame.restart();

	while (window.isOpen())	{ //головний цикл гри, який перевіряє чи гра працює 

		double time = (double)clock.getElapsedTime().asMicroseconds();
		clock.restart();

		time = time / 700;  // здесь регулируем скорость игры
		if (time > 20) time = 20;

		Event event;
		while (window.pollEvent(event))	{
			if (event.type == Event::Closed)
				window.close();
		}

		//перевірка клавіатури
		if ((Keyboard::isKeyPressed(Keyboard::Left)) || (Keyboard::isKeyPressed(Keyboard::A)))    Mario.dx = -0.1;

		if ((Keyboard::isKeyPressed(Keyboard::Right)) || (Keyboard::isKeyPressed(Keyboard::D)))   Mario.dx = 0.1;

		if ((Keyboard::isKeyPressed(Keyboard::Up)) || (Keyboard::isKeyPressed(Keyboard::W))) {
			if (Mario.onGround) {
				Mario.timeOfJump = getCurrentSeconds();
				Mario.onGround = false;
				sound.play();
			}
		} else resetTime(Mario.timeOfJump);

		if (Keyboard::isKeyPressed(Keyboard::O)
			|| !Mario.life && getCurrentSeconds() - Mario.timeOfDeath > deathTime
		) {
			curMapName = "map1.txt";
			reloadMap(Mario);
		}
		if (Keyboard::isKeyPressed(Keyboard::Q)
			|| !Mario.life && getCurrentSeconds() - Mario.timeOfDeath > deathTime
			) {
			curMapName = "map.txt";
			reloadMap(Mario);
		}
		
		for (auto&& x : enemy) { // взаимодействие врагов и игрока
			if (Mario.rect.intersects(x->rect)) {
				if (x->life)	{
					if (Mario.dy > 0 ||
						Mario.rect.top + Mario.rect.height < x->rect.top + x->rect.height/2)
					{
						x->kill();
						Mario.dy = -0.1;
					}
					else { Mario.kill(); }
				}
			}
		}

		for (auto&& x : coin) {
			if (Mario.rect.intersects(x->rect)) {
				if (Mario.dy < x->dy)	{
					x->life = false;
					x->dy = -3;
					x->sprite.setTextureRect(IntRect(111, 112, 16, 16));

					Mario.dy = 0;
				}
			}
		}
		for (auto it = enemy.begin(); it != enemy.end();) {//очистка карти від вбитих ворогів
			if (!(*it)->life)
				if (getCurrentSeconds() - (*it)->timeOfDeath > deathTime)
					it = enemy.erase(it);
			if (it != enemy.end()) ++it;
		}	
		//оновлення всіх об'єктів
		Mario.update(time); 
		for (auto&& x : enemy) {
			x->update(time);
		}
		for (auto&& x : coin) {
			x->update(time);
		}
		//рух камери	
		float centerX = Mario.rect.left + Mario.rect.width / 2;
		float centerY = Mario.rect.top + Mario.rect.height / 2;
		centerX = min(centerX, W * 16.f - 400/2);
		centerY = min(centerY, H * 16.f - 270/2);
		centerX = max(centerX, 0.f      + 400/2);
		centerY = max(centerY, 0.f      + 270/2);
		view.setCenter(centerX, centerY);
		window.setView(view);
		window.clear(Color(93, 148, 251));
		//прорисовка карты
		for (int i = 0; i < H; i++) {
			for (int j = 0; j < W; j++) {
				switch (TileMap[i][j]) {
				case 'z': tile.setTextureRect(IntRect(96, 6, 202 - 96, 105)); break;
				case 'i': tile.setTextureRect(IntRect(111, 112, 16, 16)); break;
				case 's': tile.setTextureRect(IntRect(50, 58, 42, 21)); break;
				case 'P': tile.setTextureRect(IntRect(143 - 16 * 3, 112, 16+1, 16)); break;
				case 'k': tile.setTextureRect(IntRect(143, 112, 16, 16)); break;
				case 't': tile.setTextureRect(IntRect(0, 47, 32, 95 - 47)); break;
				case 'g': tile.setTextureRect(IntRect(0, 16 * 9 - 5, 3 * 16, 16 * 2 + 5)); break;
				case 'G': tile.setTextureRect(IntRect(145, 222, 222 - 145, 255 - 222)); break;
				case 'd': tile.setTextureRect(IntRect(0, 106, 74, 127 - 106)); break;
				case 'w': tile.setTextureRect(IntRect(99, 224, 140 - 99, 255 - 224)); break;
				case 'r': tile.setTextureRect(IntRect(143 - 32, 112, 16, 16)); break;
				default:
					continue;
				}
				tile.setPosition(j * 16, i * 16);
				window.draw(tile);
			}
		}	
		
		//прорисовка персонажей
		window.draw(Mario.sprite);
		for (auto&& x : coin) {
			window.draw(x->sprite);
		}
		for (auto&& x : enemy) {
			window.draw(x->sprite);
		}
		/*sf::RectangleShape rect;
		rect.setPosition(Mario.leftCollisionBoxTile() * 16 - offsetX,
			Mario.topCollisionBoxTile() * 16 - offsetY);
		rect.setSize(sf::Vector2f(
			Mario.rightCollisionBoxTile() * 16 - Mario.leftCollisionBoxTile() * 16 + 16,
			Mario.bottomCollisionBoxTile() * 16 - Mario.topCollisionBoxTile() * 16 + 16));
		rect.setFillColor(sf::Color(255, 255, 255, 127));
		rect.setScale(1,1);
		window.draw(rect);*/
		//вывод окна
		window.display();
	}
	return 0;
}