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
    void drawPixel(int x, int y)
    {
        glBegin(GL_POINTS);
        glVertex2i(x, y);
        glEnd();
    }

    void drawCircle(Point center, int radius)
    {
        int x = radius;
        int y = 0;
        int d = 1 - x;

        while (x >= y)
        {
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
            if (d > 0)
            {
                x--;
                d -= 2 * x + 1;
            }
        }
    }

    void drawLine(Point p1, Point p2)
    {
        int x1 = static_cast<int>(p1.x);
        int y1 = static_cast<int>(p1.y);
        int x2 = static_cast<int>(p2.x);
        int y2 = static_cast<int>(p2.y);

        int dx = x2 - x1;
        int dy = y2 - y1;

        bool is_steep = abs(dy) > abs(dx);

        if (is_steep)
        {
            std::swap(x1, y1);
            std::swap(x2, y2);
        }

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
