#include "render.h"
#include "rawTexture.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>


void render_rectangle(SDL_Renderer* ren, SDL_Rect* rect, SDL_Color color = { 0, 0, 0, 0 }){
	Uint8 red, green, blue, alpha;
	SDL_GetRenderDrawColor(ren, &red, &green, &blue, &alpha);
	SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
	if(SDL_RenderDrawRect(ren, rect) != 0){
		printf("Error drawing rectangle\n");
	}
	SDL_SetRenderDrawColor(ren, red, green, blue, alpha);
}

class Timer{
public:
	Uint32 interval;
	Uint32 type;
	int timer_id;
	bool running;
	Timer(Uint32 interval){
		this->interval = interval;
		this->running = false;
		this->timer_id = 0;

		this->type = SDL_RegisterEvents(1);
		if(this->type == ((Uint32)-1)){
			printf("shit something went waayyy wrong\n");
		}
	}
	virtual ~Timer(){
		stop();
	}
	void start(){
		timer_id = SDL_AddTimer(interval, Timer::timer_callback, this);
		if(timer_id == 0){ return; }
		running = true;
	}
	void stop(){
		if(running == false){ return; }
		if(SDL_RemoveTimer(timer_id) == SDL_FALSE){
			return;
		}
		timer_id = 0;
		running = false;
	}
	static Uint32 timer_callback(Uint32 interval, void* t){
		if(t == NULL){ return 0; }
		Timer* self = (Timer*)t;
		SDL_Event e;
		SDL_zero(e);
		e.type = self->type;
		e.user.type = self->type;
		e.user.code = 0;
		e.user.data1 = t;
		e.user.data2 = 0;
		SDL_PushEvent(&e);
		return interval;
	}
};

render::render(){
	printf("Constructor render\n");
}
render::~render(){
	printf("Destroying render\n");
}

bool render::isKeyboardEvent(SDL_Event& e){
	return (e.type == SDL_KEYDOWN ||
		e.type == SDL_KEYUP ||
		e.type == SDL_TEXTEDITING ||
		e.type == SDL_TEXTINPUT);
}
bool render::isMouseEvent(SDL_Event& e){
	return (e.type == SDL_MOUSEMOTION ||
		e.type == SDL_MOUSEBUTTONDOWN ||
		e.type == SDL_MOUSEBUTTONUP);
}

void render::run(rawTexture* imageList, int size){
	// variables
	Timer timer(1000 / 30);
	SDL_Renderer* ren;
	SDL_Window* win;
	int win_x = 200;
	int win_y = 200;
	int win_w = 400;
	int win_h = 400;

	// Create the window
	Uint32 window_flags = SDL_WINDOW_SHOWN;
	win = SDL_CreateWindow(
		"binPackerTest",
		win_x, win_y, win_w, win_h,
		window_flags
		);
	if(win == NULL){
		printf("Failed to create window\n");
		return;
	}

	// create the renderer
	Uint32 ren_flags = SDL_RENDERER_ACCELERATED;
	ren_flags = SDL_RENDERER_PRESENTVSYNC;
	ren = SDL_CreateRenderer(win, -1, ren_flags);
	if(ren == NULL){
		SDL_DestroyWindow(win);
		printf("Failed to create renderer\n");
		return;
	}

	// main loop
	SDL_Event e;
	bool exit_flag = false;
	bool draw_flag = false;
	timer.start();
	for(;;){
		while(SDL_PollEvent(&e) && exit_flag == false){
			if(e.type == SDL_QUIT){
				exit_flag = true;
			} else if(isKeyboardEvent(e)){
				const Uint8* keyboard = SDL_GetKeyboardState(NULL);
				if(keyboard[SDL_SCANCODE_ESCAPE]){
					SDL_Event ev;
					SDL_zero(ev);
					ev.type = SDL_QUIT;
					ev.quit.type = SDL_QUIT;
					ev.quit.timestamp = SDL_GetTicks();
					SDL_PushEvent(&ev);
				}
			} else if(isMouseEvent(e)){
				// do nothing				
			} else if(e.type >= SDL_USEREVENT){
				if(e.user.type == timer.type){
					draw_flag = true;
				}
				// do nothing
			}
		}

		if(exit_flag){ break; }
		if(draw_flag){
			// draw the shit here.
			SDL_RenderClear(ren);

			SDL_Rect rect;
			SDL_Color color = { 90, 90, 120, 140 };
			for(int i = 0; i < size; ++i){
				rect.x = imageList[i].extent.x();
				rect.y = imageList[i].extent.y();
				rect.w = imageList[i].extent.w();
				rect.h = imageList[i].extent.h();
				render_rectangle(ren, &rect, color);
			}

			SDL_RenderPresent(ren);
		}
	}

	// destruction
	timer.stop();
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
}