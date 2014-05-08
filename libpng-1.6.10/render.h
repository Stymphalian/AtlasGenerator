#pragma once
#include <SDL.h>
#include "rawTexture.h"

class render{
public:
	bool good;
	render();
	virtual ~render();

	bool isKeyboardEvent(SDL_Event& e);
	bool isMouseEvent(SDL_Event& e);
	void run(rawTexture* imageList, int size);
private:
};
