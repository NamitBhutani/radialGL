#pragma once

#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>

struct Point
{
    float x, y;
};

namespace Drawing
{
    inline void drawPixel(int x, int y)
    {
        glBegin(GL_POINTS);
        glVertex2i(x, y);
        glEnd();
    }

    // basic midpoint circle algorithm
    inline void drawCircleOutline(Point center, int radius)
    {
        int x = radius;
        int y = 0;
        int d = 1 - x;

        while (x >= y)
        {
            // only calculate one octant and then mirror it eight times
            drawPixel(center.x + x, center.y + y);
            drawPixel(center.x + y, center.y + x);
            drawPixel(center.x - y, center.y + x);
            drawPixel(center.x - x, center.y + y);
            drawPixel(center.x - x, center.y - y);
            drawPixel(center.x - y, center.y - x);
            drawPixel(center.x + y, center.y - x);
            drawPixel(center.x + x, center.y - y);

            y++;
            if (d <= 0)
            {
                d += 2 * y + 1;
            }
            else
            {
                x--;
                d += 2 * (y - x) + 1;
            }
        }
    }

    // draws a solid, filled in circle
    inline void drawFilledCircle(Point center, int radius)
    {
        int x = radius;
        int y = 0;
        int d = 1 - x;

        while (x >= y)
        {
            // this is similar to the outline but we also fill the inside
            drawPixel(center.x - x, center.y - y);
            drawPixel(center.x + x, center.y - y);

            drawPixel(center.x - x, center.y + y);
            drawPixel(center.x + x, center.y + y);

            drawPixel(center.x - y, center.y - x);
            drawPixel(center.x + y, center.y - x);

            drawPixel(center.x - y, center.y + x);
            drawPixel(center.x + y, center.y + x);

            // fill in the circle for pretty nodes
            for (int tx = -x + 1; tx <= x - 1; tx++)
            {
                drawPixel(center.x + tx, center.y - y);
                drawPixel(center.x + tx, center.y + y);
            }

            for (int ty = -y + 1; ty <= y - 1; ty++)
            {
                drawPixel(center.x + ty, center.y - x);
                drawPixel(center.x + ty, center.y + x);
            }

            y++;
            if (d <= 0)
            {
                d += 2 * y + 1;
            }
            else if (d > 0)
            {
                x--;
                d -= 2 * x + 1;
            }
        }
    }

    // draws a line between two points using bresenham's algorithm
    inline void drawLine(Point p1, Point p2)
    {
        int x1 = static_cast<int>(p1.x);
        int y1 = static_cast<int>(p1.y);
        int x2 = static_cast<int>(p2.x);
        int y2 = static_cast<int>(p2.y);

        int dx = x2 - x1;
        int dy = y2 - y1;

        // to handle steep lines by swapping x and y
        bool is_steep = abs(dy) > abs(dx);

        if (is_steep)
        {
            std::swap(x1, y1);
            std::swap(x2, y2);
        }

        // makes sure we always draw from left to right
        if (x1 > x2)
        {
            std::swap(x1, x2);
            std::swap(y1, y2);
        }

        dx = x2 - x1;
        dy = abs(y2 - y1);
        int err = dx / 2;
        int ystep = (y1 < y2) ? 1 : -1;
        int y = y1;

        for (int x = x1; x <= x2; x++)
        {
            // swap back if it was a steep line
            if (is_steep)
            {
                drawPixel(y, x);
            }
            else
            {
                drawPixel(x, y);
            }
            err -= dy;
            if (err < 0)
            {
                y += ystep;
                err += dx;
            }
        }
    }
}