#include "ui.h"
#include "cpu.h"
#include "../../ForgeLib/include/asserts.h"

static bool uiInitialized = false;


void ui_init()
{
    if(uiInitialized) return;

    FORGE_LOG_DEBUG("STARTING UI");
   // - - - Optional to implement DEBUG menu

   uiInitialized = true;
}

void uiHandleEvents()
{
    // - - - Need to add restart and stop later
    TODO
}