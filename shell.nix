{
  pkgs ? import <nixpkgs> { },
}:

pkgs.mkShell {
  name = "opengl-dev";

  buildInputs = with pkgs; [
    pkg-config # Dependency management for libraries
    glm # GLM library
    libGL
    libGLU
    glfw
    gcc
    cmake
    ninja
    freeglut
    xorg.libX11
    xorg.libXrandr
    wayland
  ];
}
