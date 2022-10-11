#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <curses.h>

using namespace std; // Sorry

/*
 * !!!! BIG FAT DISCLAIMER !!!!
 * Applies when using the the xy() string overload in Grid.
 * The value argument ("v") MUST be define as a variable (or constant) of type "string", otherwise it will be treated as a boolean.
 * Correct:
 * 		string pixel = "!";
 * 		grid.xy(0, 0, pixel);
 * 		// Pixel at (0, 0) will be shown as the character "!"
 * Incorrect:
 * 		// NO definition of string "pixel"
 * 		grid.xy(0, 0, "!");
 * 		// Pixel at (0, 0) will be shown as an "on" pixel (usually "█")
 * If not followed specified pixel will be shown as a "on" pixel
 */

typedef vector<string> BaseGridRow;
typedef vector<BaseGridRow> BaseGrid;
typedef array<int, 2> Coords;
typedef vector<Coords> Block;
typedef array<string, 2> BWRender;

class BWGrid {
public:
	// Constructor
	BWGrid(int w, int h) {
		grid = initGrid(w, h, "0");
	}
	BWGrid(const BWGrid &g) {};

	// Getters
	BaseGrid getGrid() {
		return grid;
	}

	virtual string render(BWRender shade) {
		string rendered = "";
		for (BaseGridRow row : grid) {
			for (string px : row) {
				rendered += px == "1" ? shade[0] : shade[1];
			}
			rendered += '\n';
		}
		return rendered;
	}

	// Setters
	virtual void xy(int x, int y, bool v) {
		grid[y][x] = v ? "1" : "0";
	}
	virtual bool xyVal(int x, int y) {
		return grid[y][x] == "1" ? true : false;
	}
	virtual void xyBlock(int x, int y, int w, int h, bool v) {
		Block block = genBlock(x, y, w, h);
		for (Coords p : block) {
			xy(p[0], p[1], v);
		}
	}

	int get_height() {
		return grid.size();
	}
	int get_width() {
		return grid.at(0).size();
	}

	BWRender createBWRender(string on, string off) {
		BWRender new_shade = { on, off };
		return new_shade;
	}

	BWRender shaded = { "█", "░" };
	BWRender clear = { "█", " " };
	BWRender points = { "█", "•" };

protected:
    BaseGrid grid;
	Block genBlock(int x, int y, int w, int h) {
		Block block;
		for (int i = 0; i < w; i++) {
			for (int l = 0; l < h; l++) {
				Coords coords = { i + x, l + y };
				block.push_back(coords);
			}
		}
		return block;
	}
	BaseGrid initGrid(int w, int h, string v) {
		BaseGridRow row = initRowArray(w, v);
		BaseGrid res = initGridFromRows(h, row);
		return res;
	}

private:

	BaseGrid initGridFromRows(int h, BaseGridRow row) {
		BaseGrid res;
		for (int i = 0; i < h; i++) res.push_back(row);
		return res;
	}
	BaseGridRow initRowArray(int w, string v) {
		BaseGridRow row;
		for (int i = 0; i < w; i++) row.push_back(v);
		return row;
	}

};

class Grid : public BWGrid {
public:
	Grid(int w, int h) : BWGrid(w, h) {
		BWShade = clear;
		grid = initGrid(w, h, BWShade[1]);
	}
	Grid(const Grid &g) : BWGrid(g) {
		BWShade = g.BWShade;
		grid = g.grid;
	}

	// Getters
	virtual string render() {
		string rendered = "";
		for (BaseGridRow row : grid) {
			for (string px : row) {
				rendered += px;
			}
			rendered += '\n';
		}
		return rendered;
	}
	BWRender getBWRender() {
		return BWShade;
	}
	string getXY(int x, int y) {
		return grid[y][x];
	}

	// Setters
	virtual void xy(int x, int y, bool v) {
		grid[y][x] = v ? BWShade[0] : BWShade[1];
	}
	virtual void xy(int x, int y, string v) {
		grid[y][x] = v;
	}
	string xyStrVal(int x, int y) {
		return grid[y][x];
	}
	virtual void xyBlock(int x, int y, int w, int h, bool v) {
		Block block = genBlock(x, y, w, h);
		for (Coords p : block) {
			xy(p[0], p[1], v);
		}
	}
	virtual void xyBlock(int x, int y, int w, int h, string v) {
		Block block = genBlock(x, y, w, h);
		for (Coords p : block) {
			xy(p[0], p[1], v);
		}
	}
	virtual void xyBlock(Block block, bool v) {
		for (Coords p : block) {
			string strVal = v ? BWShade[0] : BWShade[1];
			xy(p[0], p[1], strVal);
		}
	}
	virtual void xyBlock(Block block, string v) {
		for (Coords p : block) {
			xy(p[0], p[1], v);
		}
	}

	void setBWRender(BWRender shade) {
		// Temporary Solution
		for (int y = 0; y < get_height(); y++) {
			for (int x = 0; x < get_width(); x++) {
				if (grid[y][x] == BWShade[0])
					grid[y][x] = shade[0];
				else if (grid[y][x] == BWShade[1])
					grid[y][x] = shade[1];
			}
		}
		BWShade = shade;
	}

private:
	BWRender BWShade;

};

class CGrid : public Grid {
public:
    CGrid(int w, int h, WINDOW *new_win) : Grid(w, h) {
		win = new_win;
	}
    CGrid(const CGrid &cgrid) : Grid(cgrid) {
		coords = cgrid.coords;
		colors = cgrid.colors;
		win = cgrid.win;
	}
    virtual string render(/*vector<Coords> bold*/) {
		string rendered = "";
		vector<Coords> bold = {};
		wmove(win, 0, 0);
		for (int y = 0; y < static_cast<int>(grid.size()); y++) {
			for (int x = 0; x < static_cast<int>(grid[y].size()); x++) {
				int i = 0;
				bool has_color = false;
				bool is_bold = false;
				for (Coords c : coords) {
					if (c[0] == x && c[1] == y) {
						cerr << x << "-" << y << "/" << colors[i] << endl;
						has_color = true;
						init_pair(1, static_cast<short>(colors[i]), COLOR_BLACK);
						break;
					}
					i++;
				}
				for (Coords c : bold) {
					if (c[0] == x && c[1] == y) {
						is_bold = true;
						break;
					}
				}
				if (has_color) attron(COLOR_PAIR(1));
				if (is_bold) attron(A_BOLD);
				printw(grid[y][x].c_str());
				if (has_color) attroff(COLOR_PAIR(1));
				if (is_bold) attroff(A_BOLD);
			}
			printw("\n");
		}
		return "";
	}
	void add_color(Coords c, int color) {
		coords.push_back(c);
		colors.push_back(color);
	}
	void reset_colors() {
		coords.clear();
		colors.clear();
	}
	void del_color(Coords c) {
		int i;
		bool found = false;
		for (i = 0; i < static_cast<int>(coords.size()); i++) {
			if (coords[i][0] == c[0] && coords[i][1] == c[1]) {
				found = true;
				break;
			}
		}
		if (found) {
			cerr << coords[i][0] << "x" << coords[i][1] << endl;
			coords.erase(coords.begin() + i);
			colors.erase(colors.begin() + i);
		} else {
			cerr << "Couldn't find color" << endl;
		}
	}
private:
	vector<Coords> coords;
	vector<int> colors;
	WINDOW *win;
};
