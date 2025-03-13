
#include "stdio.h"
#include "app.h"



int main(int argc, char *argv[])
{
    // Main program
    App app;
    // Init
    app.initializeEmu(argc, argv);
    app.initializeWindow();
    app.initializeUI();
    // Running
    app.renderLoop();
    // Close
    app.destroyUI();

    return 0;
}
