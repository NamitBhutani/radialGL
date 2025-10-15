#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include "tree.h"
#include "treeLayout.h"
#include "treeRenderer.h"
#include "animate.h"
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <chrono>

Tree *tree = nullptr;
TreeLayout *layout = nullptr;
TreeRenderer *renderer = nullptr;
std::vector<Point> current_positions;
Animator *animator = nullptr;

int hoveredNodeID = -1;
bool isPanning = false;
double lastMouseX = 0.0, lastMouseY = 0.0;
float cameraX = 0.0f, cameraY = 0.0f;

// manage what the app is doing right now
enum class AppState
{
    IDLE,
    ANIMATING_LAYOUT,
    ANIMATING_FIND_CENTER
};
AppState currentState = AppState::IDLE;
bool showBlueprint = false;

// variables for the find center animation timing
int findCenter_step = 0;
double findCenter_last_step_time = 0.0;
const double FIND_CENTER_STEP_DURATION = 0.8;

bool show_tree_window = true;

// main drawing function
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // moves the whole scene based on camera panning
    glTranslatef(cameraX, cameraY, 0.0f);
    if (tree && layout && renderer)
    {
        DrawState state = (currentState == AppState::ANIMATING_FIND_CENTER) ? DrawState::ANIMATING_FIND_CENTER : DrawState::NORMAL;
        // draw the tree
        auto start = std::chrono::high_resolution_clock::now();
        renderer->draw(current_positions, hoveredNodeID, showBlueprint, state, findCenter_step);
        auto end = std::chrono::high_resolution_clock::now();
        double millis = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() / 1e6;
        // std::cout << "Draw call took " << millis << " ms" << std::endl;
    }
}

void renderText(float x, float y, const std::string &text)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    std::istringstream iss(text);
    std::string line;
    const float lineHeight = 16.0f;
    float yOffset = 0.0f;
    // split text into lines and render each line varying y position
    while (std::getline(iss, line))
    {
        glRasterPos2f(x, y - yOffset);
        for (char const &c : line)
        {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
        }
        yOffset += lineHeight;
    }
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (isPanning)
    {
        double dx = xpos - lastMouseX;
        double dy = ypos - lastMouseY;
        cameraX += dx;
        cameraY -= dy; // y is inverted in screen coords, so we subtract
        lastMouseX = xpos;
        lastMouseY = ypos;
    }

    // if animation is playing, block with hovering
    if (animator->isAnimating() || currentState == AppState::ANIMATING_FIND_CENTER)
    {
        hoveredNodeID = -1;
        return;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    // convert screen coordinates to our world coordinates, taking camera pan into account
    float worldX = static_cast<float>(xpos) - cameraX;
    float worldY = static_cast<float>(height - ypos) - cameraY;

    hoveredNodeID = -1;
    // check if the mouse is close enough to any node to be considered a hover
    if (tree)
    {
        const float radius = 8.0f;
        for (size_t i = 0; i < current_positions.size(); ++i)
        {
            float dx_node = worldX - current_positions[i].x;
            float dy_node = worldY - current_positions[i].y;
            if (dx_node * dx_node + dy_node * dy_node < radius * radius)
            {
                hoveredNodeID = i;
                break;
            }
        }
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, 0, height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    const auto &io = ImGui::GetIO();
    // if the ui wants the mouse, we ignore the scroll
    if (io.WantCaptureMouse)
        return;

    if (tree && layout && animator && !animator->isAnimating() && currentState == AppState::IDLE)
    {
        float currentDelta = layout->getDelta();
        float newDelta = currentDelta + yoffset * 2.0f;
        if (newDelta < 5.0f)
            newDelta = 5.0f; // dont let it get too small

        currentState = AppState::ANIMATING_LAYOUT;
        const auto &start_pos = current_positions;
        layout->setDelta(newDelta);
        const auto &end_pos = layout->getTargetPositions();
        animator->startAnimation(start_pos, end_pos);
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    const auto &io = ImGui::GetIO();
    // if the ui wants the mouse, we ignore the click
    if (io.WantCaptureMouse)
        return;

    if (currentState != AppState::IDLE && !(currentState == AppState::ANIMATING_FIND_CENTER && action == GLFW_PRESS))
        return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            isPanning = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE)
        {
            isPanning = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && animator && !animator->isAnimating())
    {
        if (hoveredNodeID != -1)
        {
            const auto &centralNodes = layout->getCenterNodes();
            bool isCentral = (std::find(centralNodes.begin(), centralNodes.end(), hoveredNodeID) != centralNodes.end());

            // we can only re-root on non central nodes for now
            if (!isCentral)
            {
                currentState = AppState::ANIMATING_LAYOUT;
                std::cout << "re-rooting tree on node " << hoveredNodeID << "." << std::endl;
                const auto &start_pos = current_positions;
                layout->calculateLayoutFromRoot(hoveredNodeID);
                const auto &end_pos = layout->getTargetPositions();
                animator->startAnimation(start_pos, end_pos);
            }
        }
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    // toggle the ui menu
    if (key == GLFW_KEY_M)
    {
        show_tree_window = !show_tree_window;
        return;
    }

    const auto &io = ImGui::GetIO();
    // if typing in the ui, dont trigger any other keyboard shortcuts
    if (io.WantCaptureKeyboard)
        return;
}

int main(int argc, char **argv)
{
    auto full_start = std::chrono::high_resolution_clock::now();

    glutInit(&argc, argv);

    // create a random tree
    const int num_nodes = 20;
    tree = new Tree(Tree::generateRandom(num_nodes));
    layout = new TreeLayout(*tree);
    renderer = new TreeRenderer(*tree, *layout);
    animator = new Animator();

    // initial layout calculation
    layout->calculateTrueCenterLayout();
    // set up the initial animation
    const auto &end_pos = layout->getTargetPositions();
    // start all nodes from the center of the screen for explode effect
    current_positions.assign(tree->getNumVertices(), {400, 300});
    animator->startAnimation(current_positions, end_pos);
    currentState = AppState::ANIMATING_LAYOUT;

    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    GLFWwindow *window = glfwCreateWindow(800, 600, "R1 Free Tree - Interactive", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "failed to initialize glad" << std::endl;
        return -1;
    }

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    framebuffer_size_callback(window, 800, 600);

    // set a nice dark background color
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // enable anti aliasing
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // setting up imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();

    // set ui and font scale
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()) * 1.25f;
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // buffer to hold the tree's edge list for the ui text box
    char edgeListBuffer[1024 * 16];
    char filenameBuffer[256] = "tree.txt";

    // lambda to keep the edge list text box updated
    auto updateEdgeListBuffer = [&]()
    {
        std::ostringstream oss;
        for (const auto &edge : tree->getEdges())
        {
            oss << edge.first << " " << edge.second << "\n";
        }
        std::string edges_str = oss.str();
        std::copy(edges_str.begin(), edges_str.end(), edgeListBuffer);
        edgeListBuffer[edges_str.size()] = '\0';
    };
    updateEdgeListBuffer();

    // lambda to update the tree, layout, renderer and animator when the tree changes
    auto updateTree = [&](const Tree &new_tree)
    {
        Tree *new_tree_ptr = new Tree(new_tree);
        TreeLayout *new_layout_ptr = new TreeLayout(*new_tree_ptr);
        TreeRenderer *new_renderer_ptr = new TreeRenderer(*new_tree_ptr, *new_layout_ptr);

        new_layout_ptr->calculateTrueCenterLayout();
        const auto &end_pos = new_layout_ptr->getTargetPositions();

        delete tree;
        delete layout;
        delete renderer;

        // swap in the new ones
        tree = new_tree_ptr;
        layout = new_layout_ptr;
        renderer = new_renderer_ptr;

        // create a new animator and start the transition
        delete animator;
        animator = new Animator();
        current_positions.assign(tree->getNumVertices(), {400, 300});
        animator->startAnimation(current_positions, end_pos);
        currentState = AppState::ANIMATING_LAYOUT;
        hoveredNodeID = -1;
        cameraX = 0.0f;
        cameraY = 0.0f;
    };

    bool firstFrame = true;

    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // sleep if minmized
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // start a new imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_tree_window)
        {
            // define our control panel window
            ImGui::Begin("Controls & Editor", &show_tree_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Edit Tree");
            static int ui_num_nodes = tree->getNumVertices();
            ImGui::Text("Nodes:");
            ImGui::SameLine();
            ImGui::InputInt("##Nodes", &ui_num_nodes);
            ImGui::Text("Edges:");
            ImGui::InputTextMultiline("##Edges", edgeListBuffer, IM_ARRAYSIZE(edgeListBuffer), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 8));

            // the update button takes the text from the box and builds a new tree
            if (ImGui::Button("Update"))
            {
                Tree *new_tree_ptr = new Tree(ui_num_nodes);
                std::istringstream iss(edgeListBuffer);
                std::string line;
                while (std::getline(iss, line))
                {
                    std::istringstream lineStream(line);
                    int u, v;
                    if (lineStream >> u >> v)
                    {
                        new_tree_ptr->addEdge(u, v);
                    }
                }
                updateTree(*new_tree_ptr);
            }
            ImGui::SameLine();
            if (ImGui::Button("Random"))
            {
                updateTree(Tree::generateRandom(ui_num_nodes));
                updateEdgeListBuffer();
            }

            ImGui::Separator();
            ImGui::Text("Load From File");
            ImGui::InputText("##Filename", filenameBuffer, IM_ARRAYSIZE(filenameBuffer));
            ImGui::SameLine();
            if (ImGui::Button("Load"))
            {
                Tree new_tree = Tree::loadFromFile(filenameBuffer);
                if (new_tree.getNumVertices() > 0)
                {
                    updateTree(new_tree);
                    ui_num_nodes = tree->getNumVertices();
                    updateEdgeListBuffer();
                }
            }

            ImGui::Separator();
            ImGui::Text("Visualization");
            ImGui::Checkbox("Show Blueprint", &showBlueprint);
            if (ImGui::Button("Reset Tree"))
            {
                if (animator && !animator->isAnimating() && currentState == AppState::IDLE)
                {
                    // reset the tree to the true center layout
                    currentState = AppState::ANIMATING_LAYOUT;
                    std::cout << "resetting tree to true center." << std::endl;
                    const auto &start_pos = current_positions;
                    layout->calculateTrueCenterLayout();
                    const auto &end_pos = layout->getTargetPositions();
                    animator->startAnimation(start_pos, end_pos);
                }
            }
            ImGui::SameLine();
            const char *anim_button_text = (currentState == AppState::ANIMATING_FIND_CENTER) ? "Stop Animation" : "Animate Center Finding";
            if (ImGui::Button(anim_button_text))
            {
                if (animator && !animator->isAnimating())
                {
                    if (currentState == AppState::ANIMATING_FIND_CENTER)
                    {
                        currentState = AppState::IDLE;
                        std::cout << "findcenter animation stopped." << std::endl;
                    }
                    else if (currentState == AppState::IDLE)
                    {
                        std::cout << "starting findcenter animation..." << std::endl;
                        currentState = AppState::ANIMATING_FIND_CENTER;
                        layout->prepareFindCenterAnimation();
                        // reset animation step tracking
                        findCenter_step = 0;
                        findCenter_last_step_time = glfwGetTime();
                        // start the animation
                        animator->startAnimation(current_positions, layout->getTargetPositions());
                    }
                }
            }
            ImGui::Separator();
            ImGui::Text("Mouse Controls");
            ImGui::TextDisabled("Scroll to change spacing");
            ImGui::TextDisabled("Left-click a node to re-root");
            ImGui::TextDisabled("Right-click and drag to pan");
            ImGui::TextDisabled("Hover over node for info");
            ImGui::Separator();
            ImGui::TextDisabled("Press M to toggle this window");
            ImGui::End();
        }

        // if we are in an animating state, we update the animator
        if (currentState == AppState::ANIMATING_LAYOUT)
        {
            animator->update(current_positions);
            // after animation is done, go back to idle
            if (!animator->isAnimating())
            {
                currentState = AppState::IDLE;
            }
        }
        else if (currentState == AppState::ANIMATING_FIND_CENTER)
        {
            animator->update(current_positions);
            double currentTime = glfwGetTime();
            // step the animation forward every FIND_CENTER_STEP_DURATION seconds
            if (currentTime - findCenter_last_step_time > FIND_CENTER_STEP_DURATION)
            {
                findCenter_step++;
                findCenter_last_step_time = currentTime;
                const auto &generations = layout->getPruningGenerations();
                if (findCenter_step > generations.size() + 1)
                {
                    currentState = AppState::IDLE;
                    std::cout << "findcenter animation finished." << std::endl;
                }
            }
        }

        // display tree
        display();

        // render the imgui ui on top
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // render the info tooltip on hover
        if (tree && layout && hoveredNodeID != -1 && currentState == AppState::IDLE)
        {
            const auto &positions = current_positions;
            const auto &depths = layout->getDepths();
            const auto &widths = layout->getWidths();

            std::stringstream ss;
            ss << "Node ID: " << hoveredNodeID << "\n"
               << "Depth: " << depths[hoveredNodeID] << "\n"
               << "Width: " << widths[hoveredNodeID];

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            // opengl matrix calculation to draw text in the right spot
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            glOrtho(0, width, 0, height, -1, 1);
            renderText(positions[hoveredNodeID].x + cameraX + 15,
                       positions[hoveredNodeID].y + cameraY + 15,
                       ss.str());
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }

        // swap the back buffer to the front to show what we've drawn
        glfwSwapBuffers(window);
        // check for any new events like mouse clicks or key presses
        glfwPollEvents();

        // print time to first frame
        if (firstFrame)
        {
            firstFrame = false;
            auto full_end = std::chrono::high_resolution_clock::now();
            double full_millis = std::chrono::duration_cast<std::chrono::nanoseconds>(full_end - full_start).count() / 1e6;
            std::cout << "Total time to first frame: " << full_millis << " ms" << std::endl;
        }
    }

    // clean up
    delete tree;
    delete layout;
    delete renderer;
    delete animator;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}