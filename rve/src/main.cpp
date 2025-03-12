
#include "stdio.h"
#include "app.h"



int main(int argc, char *argv[])
{
    // Main program
    App app;
    // Init
    app.initializeWindow();
    app.initializeUI();
    app.initializeEmu(argc, argv);
    // Running
    app.renderLoop();
    // Close
    app.destroyUI();

    return 0;
}
