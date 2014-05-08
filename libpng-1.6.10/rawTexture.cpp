#include "rawTexture.h"

AABB::AABB(){init(0, 0, 0, 0);}
AABB::~AABB(){}
void AABB::init(int x, int y, int w, int h){set(x, y, w, h);}
int AABB::x(){ return min.x; }
int AABB::y(){ return min.y; }
int AABB::w(){ return max.x - min.x;}
int AABB::h(){ return max.y - min.y;}
int AABB::area(){ return w()*h(); }
int AABB::perimiter(){ return 2 * (w() + h()); }
void AABB::set(int x, int y, int w, int h){
	min.x = x;
	min.y = y;
	max.x = min.x + w;
	max.y = min.y + h;
}


rawTexture::rawTexture(){
	extent.set(0, 0, 0, 0);
	size.set(0, 0, 0, 0);
	image_data = NULL;
}

rawTexture::~rawTexture(){	
	if(image_data != NULL){
		delete[] image_data;
	}
}
