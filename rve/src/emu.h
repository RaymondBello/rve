#ifndef EMU_H
#define EMU_H

// UI
#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengl.h>
#endif
#ifdef __EMSCRIPTEN__
#include "emscripten_mainloop_stub.h"
#endif

// Std Library
#include <string>
// Dependencies
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
// RISC core
#include "rv32ima.h"


struct EmuSettings
{
    std::string name = "RVE - RISCV Emulator";
    std::string font = "assets/fonts/SFPro.ttf";
    float font_size = 19.0f;

    // Window settings
    bool show_demo_window = false;      // Imgui Demo
    bool show_plot_demo_window = false; // Implot Demo
    bool show_terminal_window = true;
};

class Emulator
{
    bool running;
    EmuSettings settings;

    SDL_Window *window;
    SDL_WindowFlags window_flags;
    SDL_GLContext window_context;
    SDL_Event window_event;

    const char *glsl_version;

    ImVec4 window_bg_color;

public:
    Emulator(/* args */);
    ~Emulator();
    int initializeWindow();
    int initializeUI();
    int destroyUI();
    // Rendering
    void beginRender();
    void endRender();
    void renderLoop();
    // UI
    void drawUI();
    void handleEvents();
    // Windows
    void createMenubar();
    void createTerminal();
};

#endif // EMU_H