#include <iostream>

#include "Engine/Application.h"
#include <Engine/UI.h>
#include "Engine/GameState.h"
#include <Engine/StackTrace.h>

int main()
{
    try {
        std::cout << "------\n";
        StackTrace backtrace;
        Application* app = new Application((char*)"Dean", 1920, 1080);
        app->Run();
        std::cout << "------\n";
    }
    catch(std::exception& e)
    {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        throw std::exception();
    }
    catch (...) 
    {
        std::terminate();//NOTE(darius) will save stacktrace, but i dont like it
    }

    return 0;
}

