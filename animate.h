#pragma once

#include <vector>
#include <GLFW/glfw3.h>

// smoothly moves a value from a to b
float lerp(float start, float end, float t)
{
    return start + t * (end - start);
}

class Animator
{
public:
    Animator() : animating(false), duration(0.4) {}

    // kicks off the animation, remembers where things start and end
    void startAnimation(const std::vector<Point> &start_poses, const std::vector<Point> &end_poses)
    {
        start_positions = start_poses;
        end_positions = end_poses;
        startTime = glfwGetTime();
        animating = true;
    }

    // this gets called every frame to move things a little bit
    void update(std::vector<Point> &current_positions)
    {
        if (!animating)
        {
            return;
        }

        double currentTime = glfwGetTime();
        double elapsedTime = currentTime - startTime;
        // calculates how far along the animation we are, as a fraction
        float progress = static_cast<float>(elapsedTime / duration);

        if (progress >= 1.0f)
        {
            progress = 1.0f;
            animating = false;
        }

        // moves each node to its new spot for this frame
        for (size_t i = 0; i < current_positions.size(); ++i)
        {
            current_positions[i].x = lerp(start_positions[i].x, end_positions[i].x, progress);
            current_positions[i].y = lerp(start_positions[i].y, end_positions[i].y, progress);
        }

        // snap everything to the final spot after animation is done to be sure
        if (!animating)
        {
            current_positions = end_positions;
        }
    }

    // check to see if currently animating
    bool isAnimating() const { return animating; }

private:
    bool animating;
    double startTime;
    double duration;
    std::vector<Point> start_positions;
    std::vector<Point> end_positions;
};