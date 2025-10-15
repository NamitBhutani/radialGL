#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/glut.h>
#include "tree.h"
#include "animate.h"
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

R1Tree *tree = nullptr;
Animator *animator = nullptr;
int hoveredNodeID = -1;
bool isPanning = false;
double lastMouseX = 0.0, lastMouseY = 0.0;
float cameraX = 0.0f, cameraY = 0.0f;

GLuint spotlightShaderProgram;
GLint mousePosLocation;
enum class AppState
{
    IDLE,
    ANIMATING_LAYOUT,
    ANIMATING_FIND_CENTER
};
AppState currentState = AppState::IDLE;
bool showFramework = false;

int findCenter_step = 0;
double findCenter_last_step_time = 0.0;
const double FIND_CENTER_STEP_DURATION = 0.8;

bool show_tree_window = true;

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(cameraX, cameraY, 0.0f);
    if (tree)
    {
        DrawState state = (currentState == AppState::ANIMATING_FIND_CENTER) ? DrawState::ANIMATING_FIND_CENTER : DrawState::NORMAL;
        tree->draw(hoveredNodeID, showFramework, state, findCenter_step);
    }
}

void renderText(float x, float y, const std::string &text)
{
    glColor3f(1.0f, 1.0f, 1.0f);
    float lineHeight = 16.0f;
    std::istringstream iss(text);
    std::string line;
    float yOffset = 0.0f;
    float textWidth = 0.0f;
    while (std::getline(iss, line))
    {
        glRasterPos2f(x, y - yOffset);
        for (char const &c : line)
        {
            glutBitmapCharacter(GLUT_BITMAP_9_BY_15, c);
        }
        yOffset += lineHeight;
        textWidth = std::max(textWidth, line.size() * 9.0f);
    }
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (isPanning)
    {
        double dx = xpos - lastMouseX;
        double dy = ypos - lastMouseY;
        cameraX += dx;
        cameraY -= dy;
        lastMouseX = xpos;
        lastMouseY = ypos;
    }

    if (animator->isAnimating() || currentState == AppState::ANIMATING_FIND_CENTER)
    {
        hoveredNodeID = -1;
        return;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float worldX = static_cast<float>(xpos) - cameraX;
    float worldY = static_cast<float>(height - ypos) - cameraY;

    hoveredNodeID = -1;
    if (tree)
    {
        const auto &positions = tree->getCurrentPositions();
        const float radius = 8.0f;
        for (size_t i = 0; i < positions.size(); ++i)
        {
            float dx_node = worldX - positions[i].x;
            float dy_node = worldY - positions[i].y;
            if (sqrt(dx_node * dx_node + dy_node * dy_node) < radius)
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
    if (io.WantCaptureMouse)
        return;

    if (tree && animator && !animator->isAnimating() && currentState == AppState::IDLE)
    {
        float currentDelta = tree->getDelta();
        float newDelta = currentDelta + yoffset * 2.0f;
        if (newDelta < 5.0f)
            newDelta = 5.0f;

        currentState = AppState::ANIMATING_LAYOUT;
        const auto &start_pos = tree->getCurrentPositions();
        tree->setDelta(newDelta);
        const auto &end_pos = tree->getTargetPositions();
        animator->startAnimation(start_pos, end_pos);
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    const auto &io = ImGui::GetIO();
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
            const auto &centralNodes = tree->getCenterNodes();
            bool isCentral = (std::find(centralNodes.begin(), centralNodes.end(), hoveredNodeID) != centralNodes.end());

            if (!isCentral)
            {
                currentState = AppState::ANIMATING_LAYOUT;
                std::cout << "Re-rooting tree on node " << hoveredNodeID << "." << std::endl;
                const auto &start_pos = tree->getCurrentPositions();
                tree->calculateLayoutFromRoot(hoveredNodeID);
                const auto &end_pos = tree->getTargetPositions();
                animator->startAnimation(start_pos, end_pos);
            }
        }
    }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_E)
    {
        show_tree_window = !show_tree_window;
        return;
    }

    const auto &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
        return;

    if (key == GLFW_KEY_R && animator && !animator->isAnimating() && currentState == AppState::IDLE)
    {
        currentState = AppState::ANIMATING_LAYOUT;
        std::cout << "Resetting tree to true center." << std::endl;
        const auto &start_pos = tree->getCurrentPositions();
        tree->calculateTrueCenterLayout();
        const auto &end_pos = tree->getTargetPositions();
        animator->startAnimation(start_pos, end_pos);
    }
    else if (key == GLFW_KEY_B && currentState == AppState::IDLE)
    {
        showFramework = !showFramework;
        std::cout << "Blueprint view " << (showFramework ? "ON" : "OFF") << "." << std::endl;
    }
    else if (key == GLFW_KEY_F && animator && !animator->isAnimating())
    {
        if (currentState == AppState::ANIMATING_FIND_CENTER)
        {
            currentState = AppState::IDLE;
            std::cout << "findCenter animation stopped." << std::endl;
        }
        else
        {
            std::cout << "Starting findCenter animation..." << std::endl;
            currentState = AppState::ANIMATING_FIND_CENTER;
            tree->prepareFindCenterAnimation();
            findCenter_step = 0;
            findCenter_last_step_time = glfwGetTime();

            animator->startAnimation(tree->current_positions, tree->getTargetPositions());
        }
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    const int num_nodes = 20;
    tree = R1Tree::generateRandomTree(num_nodes);
    // tree = R1Tree::loadTreeFromFile("tree.txt");

    // tree = new R1Tree(num_nodes);
    // tree->addEdge(0, 1);
    // tree->addEdge(1, 2);
    // tree->addEdge(1, 3);
    // tree->addEdge(2, 4);
    // tree->addEdge(2, 5);
    // tree->addEdge(3, 6);
    // tree->addEdge(3, 7);
    // tree->addEdge(4, 8);
    // tree->addEdge(4, 9);
    // tree->addEdge(5, 10);
    // tree->addEdge(5, 11);
    // tree->addEdge(6, 12);
    // tree->addEdge(6, 13);
    // tree->addEdge(7, 14);
    // tree->addEdge(7, 15);
    // tree->addEdge(8, 16);
    // tree->addEdge(9, 17);
    // tree->addEdge(10, 18);
    // tree->addEdge(11, 19);

    animator = new Animator();

    std::cout << "Controls:\n"
              << " - Left-Click a node to re-root.\n"
              << " - Right-Click and Drag to Pan.\n"
              << " - Press 'R' to reset.\n"
              << " - Scroll to change spacing.\n"
              << " - Press 'E' to edit the tree.\n"
              << " - Press 'F' to animate finding the center.\n"
              << " - Press 'B' to toggle the blueprint view." << std::endl;

    tree->calculateTrueCenterLayout();
    tree->current_positions = tree->getTargetPositions();

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
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    framebuffer_size_callback(window, 800, 600);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();

    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()) * 1.25f;
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    char edgeListBuffer[1024 * 16];
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

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (show_tree_window)
        {
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

            ImGui::Begin("Edit Tree", &show_tree_window, ImGuiWindowFlags_NoCollapse);

            static int ui_num_nodes = tree->getNumVertices();
            ImGui::Text("Nodes:");
            ImGui::SameLine();
            ImGui::InputInt("##Nodes", &ui_num_nodes);

            ImGui::Text("Edges:");
            ImGui::InputTextMultiline("##Edges", edgeListBuffer, IM_ARRAYSIZE(edgeListBuffer), ImVec2(-1.0f, ImGui::GetTextLineHeight() * 8));

            if (ImGui::Button("Update"))
            {
                currentState = AppState::IDLE;
                animator = new Animator();
                delete tree;
                tree = new R1Tree(ui_num_nodes);
                std::istringstream iss(edgeListBuffer);
                std::string line;
                while (std::getline(iss, line))
                {
                    std::istringstream lineStream(line);
                    int u, v;
                    if (lineStream >> u >> v)
                    {
                        tree->addEdge(u, v);
                    }
                }
                tree->calculateTrueCenterLayout();
                tree->current_positions = tree->getTargetPositions();
                hoveredNodeID = -1;
                cameraX = 0.0f;
                cameraY = 0.0f;
            }

            ImGui::SameLine();

            if (ImGui::Button("Random"))
            {
                currentState = AppState::IDLE;
                animator = new Animator();
                delete tree;
                tree = R1Tree::generateRandomTree(ui_num_nodes);
                tree->calculateTrueCenterLayout();
                tree->current_positions = tree->getTargetPositions();
                hoveredNodeID = -1;
                cameraX = 0.0f;
                cameraY = 0.0f;
                updateEdgeListBuffer();
            }

            ImGui::End();
        }

        if (currentState == AppState::ANIMATING_LAYOUT)
        {
            animator->update(tree->current_positions);
            if (!animator->isAnimating())
            {
                currentState = AppState::IDLE;
            }
        }
        else if (currentState == AppState::ANIMATING_FIND_CENTER)
        {
            animator->update(tree->current_positions);
            double currentTime = glfwGetTime();
            if (currentTime - findCenter_last_step_time > FIND_CENTER_STEP_DURATION)
            {
                findCenter_step++;
                findCenter_last_step_time = currentTime;
                const auto &generations = tree->getPruningGenerations();
                if (findCenter_step > generations.size())
                {
                    if (findCenter_step > generations.size() + 1)
                    {
                        currentState = AppState::IDLE;
                        std::cout << "findCenter animation finished." << std::endl;
                    }
                }
            }
        }

        display();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (tree && hoveredNodeID != -1 && currentState == AppState::IDLE)
        {
            const auto &positions = tree->getCurrentPositions();
            const auto &depths = tree->getDepths();
            const auto &widths = tree->getWidths();
            std::stringstream ss;
            ss << "Node ID: " << hoveredNodeID << "\n"
               << "Depth: " << depths[hoveredNodeID] << "\n"
               << "Subtree Width: " << widths[hoveredNodeID];

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete tree;
    delete animator;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}