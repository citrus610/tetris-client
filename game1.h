#pragma once

#include "game.h"
#include "board.h"

class game1 :
	public game
{
public:
	void load() override;
	void update(double dt) override;
	void render() override;
	void unload() override;
public:
	void start_game();
	void update_game(double dt);
	void end_game();
private:
	double gameover_timer = 0.0;
	int gameover_counter = 0;

	sf::Texture img_counter; sf::Sprite sp_counter;
public:
	board board_1 = board(80, 140), board_2 = board(1040, 140);
};

