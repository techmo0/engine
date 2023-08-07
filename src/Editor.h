#pragma once

#include <UI.h>
#include <ObjectSelector.h>
#include <SystemInfo.h>

class Editor
{
public:
	Editor(Window* wind);

	void update();

    void updateInput();

    void updateCamera();

    void setPolygonMode(size_t type = GL_LINE);

    void setEditorMode(int mode);

    void printFPS();

    bool lockFPS();

    Window* getWindow();
    Renderer* getRenderer();
    
private:
    GameState state;
    SystemInfo info;
    UI ui;
    Scene currScene;
    Window* window;
    Renderer rendol;
    ObjectSelector selector;


    unsigned int frame_number = 0;
    double lastTime = 0;
    double numOfFrames = 60;

    bool debug_mode = false;
};
