
#include "stdio.h"
#include "emu.h"

int main() {

    Emulator emu;
    // Init
    emu.initializeWindow();
    emu.initializeUI();
    // Running
    emu.renderLoop();
    // Close
    emu.destroyUI();

    return 0;
}
