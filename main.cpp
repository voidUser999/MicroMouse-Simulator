#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

using namespace sf;
using namespace std;

const int CELL_SIZE = 40;
const int ROWS = 10;
const int COLS = 10;

enum Direction
{
    NORTH = 0,
    EAST,
    SOUTH,
    WEST
};

struct Cell
{
    bool top = true, right = true, bottom = true, left = true;
    bool visited = false;
};

// True maze and discovered maze
static vector<vector<Cell>> maze;
static vector<vector<Cell>> discoveredMaze;

// DFS stack for exploration
static vector<pair<int, int>> pathStack;

// Movement deltas
int dx[] = {0, 1, 0, -1};
int dy[] = {-1, 0, 1, 0};

Direction leftOf(Direction d) { return Direction((d + 3) % 4); }
Direction rightOf(Direction d) { return Direction((d + 1) % 4); }
Direction behind(Direction d) { return Direction((d + 2) % 4); }
int simX = 1, simY = 1;
Direction simDir = SOUTH;
bool autoMode = false;
bool inBounds(int x, int y)
{
    return x >= 0 && y >= 0 && x < COLS && y < ROWS;
}

bool wallAt(int x, int y, Direction d)
{
    if (!inBounds(x, y))
        return true;
    const Cell &c = maze[y][x];
    switch (d)
    {
    case NORTH:
        return c.top;
    case EAST:
        return c.right;
    case SOUTH:
        return c.bottom;
    case WEST:
        return c.left;
    }
    return true;
}

// Sense walls and mark visited
void getData(int x, int y)
{
    discoveredMaze[y][x].top = wallAt(x, y, NORTH);
    discoveredMaze[y][x].right = wallAt(x, y, EAST);
    discoveredMaze[y][x].bottom = wallAt(x, y, SOUTH);
    discoveredMaze[y][x].left = wallAt(x, y, WEST);
    discoveredMaze[y][x].visited = true;
}

// Minimal-turn command
void turnTo(Direction &dir, Direction target)
{
    int diff = (target - dir + 4) % 4;
    if (diff == 3)
        dir = leftOf(dir);
    else if (diff == 1)
        dir = rightOf(dir);
    else if (diff == 2)
        dir = behind(dir);
}

// Draw the maze and exploration state
void drawMaze(RenderWindow &window)
{
    for (int y = 0; y < ROWS; ++y)
    {
        for (int x = 0; x < COLS; ++x)
        {
            RectangleShape cell(Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(x * CELL_SIZE, y * CELL_SIZE);
            cell.setFillColor(discoveredMaze[y][x].visited ? Color(220, 220, 255) : Color::White);
            cell.setOutlineColor(Color::Black);
            cell.setOutlineThickness(1);
            window.draw(cell);
            // walls...
            if (maze[y][x].top)
            {
                RectangleShape wall(Vector2f(CELL_SIZE, 2));
                wall.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                wall.setFillColor(Color::Black);
                window.draw(wall);
            }
            if (maze[y][x].right)
            {
                RectangleShape wall(Vector2f(2, CELL_SIZE));
                wall.setPosition((x + 1) * CELL_SIZE - 2, y * CELL_SIZE);
                wall.setFillColor(Color::Black);
                window.draw(wall);
            }
            if (maze[y][x].bottom)
            {
                RectangleShape wall(Vector2f(CELL_SIZE, 2));
                wall.setPosition(x * CELL_SIZE, (y + 1) * CELL_SIZE - 2);
                wall.setFillColor(Color::Black);
                window.draw(wall);
            }
            if (maze[y][x].left)
            {
                RectangleShape wall(Vector2f(2, CELL_SIZE));
                wall.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                wall.setFillColor(Color::Black);
                window.draw(wall);
            }
        }
    }
    // start
    CircleShape start(CELL_SIZE / 4);
    start.setFillColor(Color::Green);
    start.setPosition(1 * CELL_SIZE + CELL_SIZE / 4, 1 * CELL_SIZE + CELL_SIZE / 4);
    window.draw(start);
    // goal
    CircleShape goal(CELL_SIZE / 4);
    goal.setFillColor(Color::Blue);
    goal.setPosition((COLS - 2) * CELL_SIZE + CELL_SIZE / 4, (ROWS - 2) * CELL_SIZE + CELL_SIZE / 4);
    window.draw(goal);
    // robot
    CircleShape robot(CELL_SIZE / 4);
    robot.setFillColor(Color::Red);
    robot.setPosition(simX * CELL_SIZE + CELL_SIZE / 4, simY * CELL_SIZE + CELL_SIZE / 4);
    window.draw(robot);
}

// Maze generation DFS
void generateMazeDFS(int x, int y, vector<vector<bool>> &vis)
{
    vis[y][x] = true;
    vector<Direction> dirs = {NORTH, EAST, SOUTH, WEST};
    random_shuffle(dirs.begin(), dirs.end());
    for (Direction d : dirs)
    {
        int nx = x + dx[d], ny = y + dy[d];
        if (inBounds(nx, ny) && !vis[ny][nx])
        {
            if (d == NORTH)
            {
                maze[y][x].top = false;
                maze[ny][nx].bottom = false;
            }
            else if (d == EAST)
            {
                maze[y][x].right = false;
                maze[ny][nx].left = false;
            }
            else if (d == SOUTH)
            {
                maze[y][x].bottom = false;
                maze[ny][nx].top = false;
            }
            else if (d == WEST)
            {
                maze[y][x].left = false;
                maze[ny][nx].right = false;
            }
            generateMazeDFS(nx, ny, vis);
        }
    }
}

// Create a new random maze
void generateRandomMaze()
{
    maze.assign(ROWS, vector<Cell>(COLS));
    vector<vector<bool>> vis(ROWS, vector<bool>(COLS, false));
    generateMazeDFS(1, 1, vis);
}

// Reset simulation and DFS stack
void resetSim()
{
    simX = 1;
    simY = 1;
    simDir = SOUTH;
    autoMode = false;
    discoveredMaze.assign(ROWS, vector<Cell>(COLS));
    pathStack.clear();
    pathStack.emplace_back(simX, simY);
    discoveredMaze[simY][simX].visited = true;
}

// Step: explore then backtrack via stack
void simulateStep()
{
    if (simX == COLS - 2 && simY == ROWS - 2)
    {
        cout << "Maze Completed!" << endl;
        return;
    }
    getData(simX, simY);
    // try left, straight, right
    Direction tryDirs[3] = {leftOf(simDir), simDir, rightOf(simDir)};
    for (Direction d : tryDirs)
    {
        int nx = simX + dx[d], ny = simY + dy[d];
        if (!wallAt(simX, simY, d) && !discoveredMaze[ny][nx].visited)
        {
            pathStack.emplace_back(simX, simY);
            turnTo(simDir, d);
            simX = nx;
            simY = ny;
            discoveredMaze[simY][simX].visited = true;
            return;
        }
    }
    // backtrack if dead end
    if (pathStack.size() > 1)
    {
        auto [px, py] = pathStack.back();
        pathStack.pop_back();
        int dxm = px - simX, dym = py - simY;
        Direction backDir = SOUTH;
        for (int d = 0; d < 4; ++d)
        {
            if (dx[d] == dxm && dy[d] == dym)
            {
                backDir = Direction(d);
                break;
            }
        }
        turnTo(simDir, backDir);
        simX = px;
        simY = py;
        return;
    }
    // nothing left
}

int main()
{
    srand((unsigned)time(NULL));
    RenderWindow window(VideoMode(COLS * CELL_SIZE, ROWS * CELL_SIZE), "Micromouse Simulator");
    Clock clock;
    maze.assign(ROWS, vector<Cell>(COLS));
    discoveredMaze.assign(ROWS, vector<Cell>(COLS));
    generateRandomMaze();
    resetSim();
    while (window.isOpen())
    {
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                window.close();
            if (e.type == Event::KeyPressed)
            {
                if (e.key.code == Keyboard::Space)
                    simulateStep();
                else if (e.key.code == Keyboard::A)
                    autoMode = !autoMode;
                else if (e.key.code == Keyboard::R)
                {
                    generateRandomMaze();
                    resetSim();
                }
            }
        }
        if (autoMode && clock.getElapsedTime().asMilliseconds() > 300)
        {
            simulateStep();
            clock.restart();
        }
        window.clear(Color::White);
        drawMaze(window);
        window.display();
    }
    return 0;
}
