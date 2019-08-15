#pragma once

class Position {
public:
    int x = 0;
    int y = 0;

    Position() {}
    Position(int x, int y) : x(x), y(y) {}
    Position operator+(Position &p) {
        return Position(this->x + p.x, this->y + p.y);
    }
};

class Positionable {
public:
    Position position;
    int width = 0;
    int height = 0;
    Positionable() : position() {}
    Positionable(int x, int y, int width, int height) : position(x, y), width(width), height(height) {}
};
