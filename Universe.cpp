#include "Universe.h"
#include <time.h>
#include "CSIcommands.h"
#include <deque>

using namespace std;
extern deque<char> keyevents;
static const int dx[] = {0,0, 1,1,1, -1,-1,-1};
static const int dy[] = {1,-1,1,0,-1, 1,0, -1};


bool Universe::is_valid(int x, int y)
{
	return x >= 0 && y >= 0 && x < m_size && y < m_size;
}

Universe::Universe(int size, float gps) : m_size(size), m_gps(gps), m_currentGeneration(0)
{
	std::cout.sync_with_stdio(false);

	m_grid.resize(m_size);
	for(auto& v : m_grid)
		v.resize(m_size);
}

//the grid has to be equal in size
void Universe::initialize(vector<vector<int>> start_grid)
{
	for(int i = 0; i < m_size; i++)
	{
		for(int j = 0; j < m_size; j++)
			m_grid[i][j] = start_grid.at(i).at(j);
	}
}
void Universe::reset()
{
	for(int i = 0; i < m_size; i++)
		for(int j = 0; j < m_size; j++)
			m_grid[i][j] = 0;
}

int Universe::count_neighbors(int x, int y)
{
	int count = 0;
	for(int d = 0; d < 8; d++)
	{
		if(this->is_valid(dx[d] + x, dy[d]+ y))
		{
			count += m_grid[dx[d] + x][dy[d]+ y];
		}
	}
	return count;
}

// Any live cell with fewer than two live neighbors dies, as if caused by underpopulation
// Any live cell with two or three live neighbors lives on to the next generation
// Any live cell with more than three live neighbors dies, as if by overpopulation
//  Any dead cell with exactly three live neighbors becomes a live cell, as if by reproductio
void Universe::next_generation()
{
	auto new_grid = m_grid;
	for(int i = 0; i < m_size; i++)
		for(int j = 0;j < m_size; j++)
		{
			int neighbors = this->count_neighbors(i, j);
			if(neighbors < 2)
				new_grid[i][j] = false;
			else if(neighbors > 3)
				new_grid[i][j] = false;
			else if(neighbors == 3)
				new_grid[i][j] = true;
		}
	m_grid = move(new_grid);
	m_currentGeneration++;
}
void Universe::display()
{

	std::cout << CSI_HIDE_CURSOR;
	std::cout << CSI_SET_CURSOR(1,1);

	for(int i = 0; i < m_size; i++)
	{
		for(int j = 0; j < m_size; j++)
		{
			int b = 0;
			if(i == m_highlighter_y && j == m_highlighter_x ) b = 255;
			if(m_grid[i][j])
			{
				cout << CSI_SET_BG(0, 255,b) << "  " ;
			}
			else
			{
				cout << CSI_SET_BG(50, 50,50 + min(b, 205)) << "  " ;
			}
		}
		cout << CSI_RESET_FORMAT <<'\n';
	}
	cout << "current Generation: " << m_currentGeneration <<'\n';

	cout << "press wasd to move\n";
	cout << "press spacebar to set/unset a cell\n";
	cout << "press p to pause/unpause\n";
	cout << "press z to hide selection\n";
	cout << flush;
	// std::cout.sync_with_stdio(true);

}
void Universe::run(int gens)
{
	string temp;
	int current_gen = 0;
	unsigned int lastClock, currentClock;
	unsigned int gen_time = (CLOCKS_PER_SEC / m_gps);
	cout << CSI_SET_CURSOR(1,1);
	this->display();
	lastClock = clock() + gen_time;
	while(true)
	{
		for(char event : keyevents)
		{
			switch(event)
			{
				case 'a':
				case 'A':
					if(m_highlighter_x > 0) m_highlighter_x--;
				break;
				case 'd':
				case 'D':
					if(m_highlighter_x < 40 - 1) m_highlighter_x++;
				break;
				case 'w':
				case 'W':
					if(m_highlighter_y > 0) m_highlighter_y--;

				break;
				case 's':
				case 'S':
					if(m_highlighter_y < 40 - 1) m_highlighter_y++;
				break;
				case ' ':
					this->set_cell(m_highlighter_x, m_highlighter_y, !this->cell_at(m_highlighter_x, m_highlighter_y));
				break;
				case 'z':
				case 'Z':
					m_highlighter_y = -1;
					m_highlighter_x = -1;
				break;
				case 'p':
				case 'P':
					m_paused = !m_paused;
				break;
			}
		}
		keyevents.clear();
		this->display();
		if(lastClock < (currentClock = clock())  && !m_paused)
		{
			this->next_generation();
			this->display();
			lastClock = currentClock + gen_time;
			current_gen++;
			if(current_gen == gens)
				break;
		}

	}	
}
void Universe::set_cell(int x, int y, bool value)
{
	if(is_valid(x, y))
		m_grid[y][x] = value;
}
bool Universe::cell_at(int x, int y)
{
	if(is_valid(x, y))
		return m_grid[y][x];
	return 0;
}

void Universe::set_highlight(int x, int y)
{
	m_highlighter_x = x;
	m_highlighter_y = y;
}
