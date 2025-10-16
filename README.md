# Interactive Free Tree Visualization

This project allows users to generate, load, and manipulate tree structures in real-time. The core visualization uses a radial layout to arrange nodes in a visually appealing circular pattern, with smooth animations providing feedback for every change.

The radial drawing method is based on the paper **"Drawing Free Trees" by P. D. Eades**.

![Main](docs/assets/main.png)

## Features

_For a more detailed explanation of the algorithms and implementation, take a look at `docs/index.html`._

### Tree Generation and Management

-   **Live Tree Editing:** A GUI text box displays the current tree's edge list. Manually add, remove, or modify edges and click "Update" to apply changes and see an animated transition to the new layout.
-   **Random Tree Generation:** Specify the number of nodes in the GUI and click "Random" to generate a valid tree structure using a random Prüfer sequence (`R1Tree::generateRandomTree`).
-   **Load from File:** Load a tree from a text file (`.txt`) using the "Load" button. The format is an integer `N` for the node count, followed by `u v` edge pairs on each line.

### Interactive Visualization and Controls

-   **Radial Layout Algorithm:** The primary layout places the tree's true center at the origin. Nodes are then placed on concentric circles based on their depth, with their angle determined by their subtree's width, ensuring a planar drawing.
-   **Interactive Rerooting:** **Left-click** any node to instantly designate it as the new root. The entire layout is recalculated and smoothly animated from the new perspective.
-   **Smooth Panning:** **Right-click and drag** to pan the camera across the scene, making it easy to navigate large trees.
-   **Dynamic Spacing:** Use the **mouse scroll wheel** to increase or decrease the spacing between the concentric layers of the tree, triggering a smooth animated transition to the new scale.
-   **Node Tooltip:** **Hover** over a node to display its ID, depth, and subtree width. The hovered node and its entire subtree are also highlighted for clarity.

### Animations

-   **Center-Finding Animation:** Visualize the center-finding algorithm as it iteratively prunes leaf nodes. Pruned nodes fade out, showing the convergence to the central one or two nodes.
-   **Smooth Transitions:** Nearly every action that changes the layout (rerooting, updating, spacing) is animated using a time-based linear interpolation (`lerp`) for a fluid user experience.

### Technical and GUI Features

-   **GUI Panel:** The control panel is built with **ImGui**, providing a powerful and easy-to-use interface.
-   **Layout Blueprint:** A "Show Blueprint" checkbox toggles the visibility of the geometric framework (concentric circles and angular wedges) used by the layout algorithm.
-   **Legacy Graphics Primitives:** Lines and circles are rendered from scratch using **Bresenham's and Midpoint Circle algorithms** via immediate mode OpenGL (`graphics.h`).

## Technologies & Libraries Used

-   **Language:** C++
-   **Graphics API:** OpenGL
-   **Windowing & Input:** GLFW
-   **OpenGL Function Loading:** GLAD
-   **GUI:** ImGui
-   **Build System:** CMake

## Setup

ImGui and GLAD are included as Git submodules.

```bash
git clone --recursive https://github.com/NamitBhutani/radialGL
```

CMake is used to manage the build process.

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
./FreeTreeDrawing
```
