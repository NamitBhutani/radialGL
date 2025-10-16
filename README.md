# Interactive Free Tree Visualization

This project allows users to generate, load, and manipulate tree structures in real-time. The core visualization uses a radial layout to arrange nodes in a visually appealing circular pattern, with smooth animations providing feedback for every change.

## Features

### Tree Generation and Management

- **Random Tree Generation:** Specify the number of nodes in the GUI and click "Random" to generate a valid tree structure using a Prüfer sequence (`R1Tree::generateRandomTree`).
- **Load from File:** Load a tree from a text file (`.txt`) using the "Load" button. The format is an integer `N` for the node count, followed by `u v` edge pairs on each line.
- **Live Tree Editing:** A GUI text box displays the current tree's edge list. Manually add, remove, or modify edges and click "Update" to apply changes and see an animated transition to the new layout.

### Interactive Visualization and Controls

- **Radial Layout Algorithm:** The primary layout places the tree's true center at the origin. Nodes are then placed on concentric circles based on their depth, with their angle determined by their subtree's width, ensuring a planar drawing.
- **Interactive Rerooting:** **Left-click** any node to instantly designate it as the new root. The entire layout is recalculated and smoothly animated from the new perspective.
- **Smooth Panning:** **Right-click and drag** to pan the camera across the scene, making it easy to navigate large trees.
- **Dynamic Spacing:** Use the **mouse scroll wheel** to increase or decrease the spacing between the concentric layers of the tree, triggering a smooth animated transition to the new scale.
- **Node Tooltip:** Hovering over a node displays its ID, depth, and subtree width. The hovered node and its entire subtree are also highlighted for clarity.

### Animations

- **Smooth Transitions:** Nearly every action that changes the layout (rerooting, updating, spacing) is animated using a time-based linear interpolation (`lerp`) for a fluid user experience.
- **Center-Finding Animation:** Visualize the center-finding algorithm as it iteratively prunes leaf nodes. Pruned nodes fade out, showing the convergence to the central one or two nodes.

### Technical and GUI Features

- **GUI Panel:** The control panel is built with **ImGui**, providing a powerful and easy-to-use interface.
- **Layout Blueprint:** A "Show Blueprint" checkbox toggles the visibility of the geometric framework (concentric circles and angular wedges) used by the layout algorithm.
- **Legacy Graphics Primitives:** Lines and circles are rendered from scratch using **Bresenham's and Midpoint Circle algorithms** via legacy immediate mode OpenGL (`graphics.h`).

---

## How It Works: The Radial Layout Algorithm

The implemented layout is a radial drawing algorithm that arranges the tree on a series of concentric circles. To ensure a balanced and symmetric visualization, it begins by finding the "center" of the tree and designates it as the root.

The algorithm then determines the final position for each node in two phases:

1.  **Bottom-Up Traversal:** It first traverses the tree to calculate the "width" of each subtree, defined by its number of leaves. This width determines the size of the angular wedge each branch will occupy.
2.  **Top-Down Traversal:** A second, top-down traversal places each node on a circle corresponding to its depth from the root, positioned angularly within its assigned wedge.

This two-phase process guarantees a planar drawing with no edge crossings.

---

## Performance

The computational complexity of the core algorithms is highly efficient:

- **Layout Calculation:** Based on two tree traversals, calculating a new layout is linear in the number of nodes, or `O(V)`, where `V` is the number of vertices.
- **Rendering:** Each frame iterates through all nodes and edges to draw them, resulting in an `O(V+E)` operation, which is `O(V)` for a tree. Performance is excellent for trees up to several hundred nodes.
- **Animation:** The animation update loop iterates over all nodes, making it an `O(V)` operation per frame.

---

## Known Issues & Challenges

- **Legacy OpenGL Pipeline Limitations:** The drawing functions in `graphics.h` use immediate mode OpenGL (e.g., `glBegin`/`glEnd`), which is deprecated. This prevented the implementation of modern features like shaders for more advanced visual effects (e.g., node glow), as they are incompatible with the legacy pipeline.

---

## Technologies & Libraries Used

- **Language:** C++
- **Graphics API:** OpenGL
- **Windowing & Input:** GLFW
- **OpenGL Function Loading:** GLAD
- **GUI:** ImGui

## References

- **Algorithm Source:** The radial drawing method is based on the paper **"Drawing Free Trees" by P. D. Eades**.
