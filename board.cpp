#include "board.h"

board::board(int x, int y)
{
	this->x = x; this->y = y;

	img_block.loadFromFile("res/graphic/block.png");
	img_block_overlay.loadFromFile("res/graphic/block_overlay.png");
	img_ghost_block.loadFromFile("res/graphic/ghost_block.png");
	img_board.loadFromFile("res/graphic/board.png");
	img_clear_line.loadFromFile("res/graphic/clear_line.png");
	img_tetrismino.loadFromFile("res/graphic/tetrismino.png");

	bool smooth_img = false;
	img_block.setSmooth(smooth_img);
	img_block_overlay.setSmooth(smooth_img);
	img_ghost_block.setSmooth(smooth_img);
	img_board.setSmooth(smooth_img);
	img_clear_line.setSmooth(smooth_img);
	img_tetrismino.setSmooth(smooth_img);

	img_block.generateMipmap();
	img_block_overlay.generateMipmap();
	img_ghost_block.generateMipmap();
	img_board.generateMipmap();
	img_clear_line.generateMipmap();
	img_tetrismino.generateMipmap();

	sp_block.setTexture(img_block);
	sp_block_overlay.setTexture(img_block_overlay);
	sp_ghost_block.setTexture(img_ghost_block);
	sp_board.setTexture(img_board);
	sp_clear_line.setTexture(img_clear_line);
	sp_tetrismino.setTexture(img_tetrismino);

	red_bar = sf::VertexArray(sf::Quads);
	for (int i = 0; i < 8; ++i) red_bar.append(sf::Vertex(sf::Vector2f(0, 0), sf::Color::Red));
	red_bar[2].position = sf::Vector2f(this->x + 168 + 24, this->y + 800);
	red_bar[3].position = sf::Vector2f(this->x + 168, this->y + 800);
	red_bar[6].position = sf::Vector2f(this->x + 608 + 24, this->y + 800);
	red_bar[7].position = sf::Vector2f(this->x + 608, this->y + 800);
	red_bar[0].position.x = this->x + 168;
	red_bar[1].position.x = this->x + 168 + 24;
	red_bar[4].position.x = this->x + 608;
	red_bar[5].position.x = this->x + 608 + 24;
}

void board::init()
{
	for (int i = 0; i < 40; i++) {
		for (int k = 0; k < 10; k++) {
			this->data[i][k] = 0;
		}
	}
	this->next_piece.clear();
	this->hold_piece = ' ';
	this->current_piece = ' ';
	this->garbage = 0;
	this->b2b = false;
	this->ren = 0;
	this->in_game_timer = 0.0; 
	this->clear_line_timer = 0.6;
	this->hold_counter = 0;
	this->piece_taken_counter = 0; 
	this->clear_line_counter = 2;
	this->game_over = false;

	shuffle_bag();
	fill_queue();
	change_piece();
}

void board::set_enemy(board * _enemy)
{
	this->enemy = _enemy;
}

void board::shuffle_bag()
{
	int n = 7;
	while (n > 1)
	{
		n--;
		int k = rand() % (n + 1);
		char value = this->bag[k];
		this->bag[k] = this->bag[n];
		this->bag[n] = value;
	}
}

void board::fill_queue()
{
	int nextpiececounter = this->next_piece.size();
	for (int i = 0; i < preview - nextpiececounter; i++) {
		this->next_piece.push_back(this->bag[this->piece_taken_counter]);
		this->piece_taken_counter++;
		if (this->piece_taken_counter >= 7) {
			this->piece_taken_counter = 0;
			this->shuffle_bag();
		}
	}
}

void board::change_piece()
{
	this->current_piece = this->next_piece[0];
	this->real_piece.set_type(this->current_piece);
	this->real_piece.init();
	this->next_piece.erase(this->next_piece.begin() + 0);
	this->fill_queue();
}

void board::hold()
{
	if (hold_counter == 0) {
		hold_counter = 1;
		if (this->hold_piece == ' ') {
			this->hold_piece = this->current_piece;
			this->change_piece();
		}
		else {
			char place_holder = this->hold_piece;
			this->hold_piece = this->current_piece;
			this->current_piece = place_holder;
			this->real_piece.set_type(this->current_piece);
			this->real_piece.init();
		}
	}
}

void board::place_piece()
{
	for (size_t _y = 0; _y < piece_form[this->current_piece][this->real_piece.form].size(); _y++) {
		for (size_t _x = 0; _x < piece_form[this->current_piece][this->real_piece.form][_y].size(); _x++) {
			if (piece_form[this->current_piece][this->real_piece.form][_y][_x] > 0) {
				this->data[_y + this->real_piece.y][_x + this->real_piece.x] = piece_form[this->current_piece][this->real_piece.form][_y][_x];
			}
		}
	}
}

std::vector<int> board::full_row()
{
	std::vector<int> result;

	for (int i = 0; i < 40; ++i) {
		int mul_pdt = 1;
		for (int k = 0; k < 10; ++k) {
			mul_pdt *= this->data[i][k];
		}
		if (mul_pdt != 0) result.push_back(i);
	}

	return result;
}

std::vector<int> board::full_line_before_lock()
{
	if (!this->real_piece.is_lock()) {
		return std::vector<int>();
	}

	std::vector<int> result;

	int vir_board[40][10];
	for (int i = 0; i < 40; ++i) {
		for (int k = 0; k < 10; ++k) {
			vir_board[i][k] = this->data[i][k];
		}
	}
	
	for (size_t _y = 0; _y < piece_form[this->current_piece][this->real_piece.form].size(); _y++) {
		for (size_t _x = 0; _x < piece_form[this->current_piece][this->real_piece.form][_y].size(); _x++) {
			if (piece_form[this->current_piece][this->real_piece.form][_y][_x] > 0) {
				vir_board[_y + this->real_piece.y][_x + this->real_piece.x] = piece_form[this->current_piece][this->real_piece.form][_y][_x];
			}
		}
	}

	for (int i = 0; i < 40; ++i) {
		int mul_pdt = 1;
		for (int k = 0; k < 10; ++k) {
			mul_pdt *= vir_board[i][k];
		}
		if (mul_pdt != 0) result.push_back(i);
	}

	return result;
}

void board::clear_line(std::vector<int> full_row_vec)
{
	for (int i = 0; i < (int)full_row_vec.size(); i++) {
		for (int k = full_row_vec[i]; k > 0; k--) {
			for (int m = 0; m < 10; ++m) {
				this->data[k][m] = this->data[k - 1][m];
			}
		}
	}
}

int board::count_garbage(bool t_spin, std::vector<int> full_row_vec)
{
	int result = 0;

	if (t_spin) {
		result = full_row_vec.size() * 2;
		if (this->b2b)
			result++;
		this->b2b = true;
	}
	else {
		switch (full_row_vec.size())
		{
		case 0:
			break;
		case 4:
			result = 4;
			if (this->b2b)
				result++;
			this->b2b = true;
			break;
		default:
			this->b2b = false;
			result = full_row_vec.size() - 1;
			break;
		}
	}

	// find if perfect clear
	int not_full_or_full_row_count = 0;
	for (int i = 20; i < 40; i++) {
		int row_sum = 0;
		int row_product = 1;
		for (int k = 0; k < 10; k++) {
			row_sum += this->data[i][k];
			row_product *= this->data[i][k];
		}
		if ((row_sum == 0) || (row_product > 0)) {
			not_full_or_full_row_count++;
		}
	}
	if (not_full_or_full_row_count == 20) {
		result = 10;
		this->b2b = true;
	}

	// ren
	if (full_row_vec.size() > 0) {
		this->ren++;
	}
	else {
		this->ren = 0;
	}
	if (this->ren < 10) {
		result += (this->ren - (this->ren % 2)) / 2;
	}
	else if (this->ren == 10) {
		result += 4;
	}
	else {
		result += 5;
	}

	return result;
}

void board::sent_garbage(int line)
{
	this->enemy->garbage += line;
}

void board::place_garbage()
{
	if (this->garbage > 39) {
		this->garbage = 22;
	}

	std::vector<int> g_dist;

	if (this->garbage < 1) {
		return;
	}

	if (this->garbage < 4) {
		g_dist.push_back(this->garbage);
	}
	else {
		int g_remain = 0;
		while (g_remain < this->garbage)
		{
			int to_add = std::min(this->garbage - g_remain, this->g_per_dist[rand() % 8]);
			g_remain += to_add;
			g_dist.push_back(to_add);
		}
	}

	// shift the row up
	for (int i = 0; i < 40 - this->garbage; i++) {
		for (int k = 0; k < 10; ++k) {
			this->data[i][k] = this->data[i + this->garbage][k];
		}
	}

	std::vector<std::array<int, 10>> g_line_to_add;
	for (int i = 0; i < (int)g_dist.size(); i++) {
		int hole_pos = rand() % 8;
		for (int k = 0; k < g_dist[i]; k++) {
			std::array<int, 10> to_add = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };
			to_add[hole_pos] = 0;
			g_line_to_add.push_back(to_add);
		}
	}

	for (int i = 0; i < this->garbage; i++) {
		for (int k = 0; k < 10; ++k) {
			this->data[39 - i][k] = g_line_to_add[i][k];
		}
	}

	this->garbage = 0;
}

bool board::is_game_over()
{
	return this->game_over;
}

bool board::is_clearing_line()
{
	return this->clear_line_timer < line_clear;
}

bool board::just_done_clear_line()
{
	return this->clear_line_counter == 1;
}

void board::update_clear_line(double dt)
{
	this->clear_line_timer += dt;
	if (this->clear_line_timer >= line_clear) {
		this->clear_line_timer = line_clear + 0.1;
		this->clear_line_counter++;
		if (this->clear_line_counter >= 2) this->clear_line_counter = 2;
	}
	else {
		this->clear_line_counter = 0;
	}
}

void board::update(double dt)
{
	if (!this->is_game_over()) {
		this->update_clear_line(dt);
		if (this->just_done_clear_line()) {
			bool t_spin = this->real_piece.is_lock_tspin(this->data);
			this->place_piece();
			std::vector<int> full_row_list = this->full_row();
			int line_sent = this->count_garbage(t_spin, full_row_list);
			if (full_row_list.size() < 1) {
				this->place_garbage();
			}
			else {
				if (this->garbage > line_sent) {
					this->garbage -= line_sent;
					this->place_garbage();
				}
				else {
					this->sent_garbage(line_sent - this->garbage);
					this->garbage = 0;
				}
			}
			this->clear_line(full_row_list);
			this->change_piece();
		}
		if (!this->is_clearing_line()) {
			this->real_piece.update(dt, this->data);
			this->in_game_timer += dt;
			if (this->in_game_timer >= 0.5) {
				this->in_game_timer = 0.0;
				if (!input::down.isPressed()) {
					this->real_piece.try_down(this->data);
				}
			}
			if (input::c.justPressed())
				this->hold();
			if (this->real_piece.is_lock()) {
				if (this->real_piece.y <= 18 && (this->real_piece.x == 3 || this->real_piece.x == 4)) this->game_over = true;
				this->hold_counter = 0;
				if (this->full_line_before_lock().size() < 1) {
					this->place_piece();
					this->change_piece();
					this->place_garbage();
					this->ren = 0;
				}
				else {
					this->clear_line_timer = 0.0;
					this->clear_line_counter = 0;
				}
			}
		}
	}
}

void board::render(sf::RenderTexture * _canvas)
{
	std::vector<int> full_row_list = full_line_before_lock();

	// draw board
	sp_board.setPosition(this->x - 8, this->y - 8);
	_canvas->draw(sp_board);

	// draw red bar
	int rb_y = this->y + 20 * 40 - this->garbage * 40;
	red_bar[0].position.y = rb_y;
	red_bar[1].position.y = rb_y;
	red_bar[4].position.y = rb_y;
	red_bar[5].position.y = rb_y;
	_canvas->draw(this->red_bar);

	// draw block
	for (int i = 0; i < 20; ++i) {
		if (is_in_vec(full_row_list, i + 20)) {
			continue;
		}
		for (int k = 0; k < 10; ++k) {
			if (this->data[i + 20][k] > 0) {
				if (!this->is_game_over())
					sp_block.setColor(this->color[this->data[i + 20][k] - 1]);
				else {
					sp_block.setColor(sf::Color(64, 64, 64, 255));
				}
				sp_block.setPosition(this->x + 200 + k * 40, this->y + i * 40);
				_canvas->draw(sp_block);
			}
		}
	}
	for (int i = 0; i < 20; ++i) {
		if (is_in_vec(full_row_list, i + 20)) {
			continue;
		}
		for (int k = 0; k < 10; ++k) {
			if (this->data[i + 20][k] > 0) {
				sp_block_overlay.setPosition(this->x + 200 + k * 40, this->y + i * 40);
				//_canvas->draw(sp_block_overlay);
			}
		}
	}

	// draw ghost piece
	int _y = this->real_piece.y;
	while (!this->real_piece.is_colliding(this->data, this->real_piece.x, _y + 1))
	{
		_y++;
	}
	for (int i = 0; i < (int)piece_form[this->real_piece.get_type()][this->real_piece.form].size(); ++i) {
		for (int k = 0; k < (int)piece_form[this->real_piece.get_type()][this->real_piece.form][i].size(); ++k) {
			int a_pixel = piece_form[this->real_piece.get_type()][this->real_piece.form][i][k];
			if (a_pixel > 0 && _y + i - 20 > -1 && !is_in_vec(full_row_list, _y + i)) {
				sp_ghost_block.setColor(this->color[a_pixel - 1]);
				sp_ghost_block.setPosition(this->x + 200 + this->real_piece.x * 40 + k * 40, this->y - 800 + _y * 40 + i * 40);
				_canvas->draw(sp_ghost_block);
			}
		}
	}

	// draw piece
	for (int i = 0; i < (int)piece_form[this->real_piece.get_type()][this->real_piece.form].size(); ++i) {
		for (int k = 0; k < (int)piece_form[this->real_piece.get_type()][this->real_piece.form][i].size(); ++k) {
			int a_pixel = piece_form[this->real_piece.get_type()][this->real_piece.form][i][k];
			if (a_pixel > 0 && this->real_piece.y + i - 20 > -1 && !is_in_vec(full_row_list, this->real_piece.y + i)) {
				sp_block.setColor(this->color[a_pixel - 1]);
				sp_block.setPosition(this->x + 200 + this->real_piece.x * 40 + k * 40, this->y - 800 + this->real_piece.y * 40 + i * 40);
				_canvas->draw(sp_block);
			}
		}
	}

	// draw piece
	for (int i = 0; i < (int)piece_form[this->real_piece.get_type()][this->real_piece.form].size(); ++i) {
		for (int k = 0; k < (int)piece_form[this->real_piece.get_type()][this->real_piece.form][i].size(); ++k) {
			int a_pixel = piece_form[this->real_piece.get_type()][this->real_piece.form][i][k];
			if (a_pixel > 0 && this->real_piece.y + i - 20 > -1 && !is_in_vec(full_row_list, this->real_piece.y + i)) {
				sp_block_overlay.setColor(sf::Color::Black);
				sp_block_overlay.setPosition(this->x + 200 + this->real_piece.x * 40 + k * 40, this->y - 800 + this->real_piece.y * 40 + i * 40);
				//_canvas->draw(sp_block_overlay);
			}
		}
	}

	// draw clear line
	if (is_clearing_line()) {
		int c_index = std::min((int)(this->clear_line_timer / 0.016), 29);
		sp_clear_line.setTextureRect(sf::IntRect(0, c_index * 40, 400, 40));
		for (size_t i = 0; i < full_row_list.size(); ++i) {
			sp_clear_line.setPosition(this->x + 200, this->y + (full_row_list[i] - 20) * 40);
			_canvas->draw(sp_clear_line);
		}
	}

	// draw next queue
	for (int i = 0; i < std::min(5, this->preview); ++i) {
		sp_tetrismino.setTextureRect(sf::IntRect(piece_index[this->next_piece[i]] * 160, 0, 160, 160));
		sp_tetrismino.setPosition(this->x + 640, this->y + i * 160);
		_canvas->draw(sp_tetrismino);
	}

	// draw hold piece
	if (this->hold_piece != ' ') {
		sp_tetrismino.setTextureRect(sf::IntRect(piece_index[this->hold_piece] * 160, 0, 160, 160));
		sp_tetrismino.setPosition(this->x, this->y);
		_canvas->draw(sp_tetrismino);
	}
}
