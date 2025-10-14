#pragma once

#include "graphics.h"
#include <vector>
#include <GLFW/glfw3.h>

float lerp(float start, float end, float t)
{
    return start + t * (end - start);
}

class Animator
{
public:
    Animator() : animating(false), duration(0.4) {}

    void startAnimation(const std::vector<Point> &start_poses, const std::vector<Point> &end_poses)
    {
        start_positions = start_poses;
        end_positions = end_poses;
        startTime = glfwGetTime();
        animating = true;
    }

    void update(std::vector<Point> &current_positions)
    {
        if (!animating)
        {
            return;
        }

        double currentTime = glfwGetTime();
        double elapsedTime = currentTime - startTime;
        float progress = static_cast<float>(elapsedTime / duration);

        if (progress >= 1.0f)
        {
            progress = 1.0f;
            animating = false;
        }

        for (size_t i = 0; i < current_positions.size(); ++i)
        {
            current_positions[i].x = lerp(start_positions[i].x, end_positions[i].x, progress);
            current_positions[i].y = lerp(start_positions[i].y, end_positions[i].y, progress);
        }

        if (!animating)
        {
            current_positions = end_positions;
        }
    }

    bool isAnimating() const { return animating; }

private:
    bool animating;
    double startTime;
    double duration;
    std::vector<Point> start_positions;
    std::vector<Point> end_positions;
};