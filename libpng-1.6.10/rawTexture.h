#pragma once

#include <string>
#include "png.h"

class Vec2d{
public:
	int x, y;
	Vec2d() : x(0), y(0){};
	Vec2d(int x, int y) : x(x), y(y){};
	Vec2d(const Vec2d& other){
		this->x = other.x;
		this->y = other.y;
	}
	virtual ~Vec2d(){};

	Vec2d operator+(const Vec2d& rhs){
		Vec2d v;
		v.x = x + rhs.x;
		v.y = y + rhs.y;
		return v;
	}
	Vec2d operator-(const Vec2d& rhs){
		Vec2d v;
		v.x = x - rhs.x;
		v.y = y - rhs.y;
		return v;
	}
	Vec2d& operator=(const Vec2d& rhs){
		this->x = rhs.x;
		this->y = rhs.y;
		return *this;
	}	
};


class AABB{
public:
	Vec2d min;
	Vec2d max;
	AABB();
	virtual ~AABB();

	void init(int a, int b, int c, int d);		
	int x();
	int y();
	int w();
	int h();
	int area();
	int perimiter();
	void set(int x, int y, int w, int h);
};



class rawTexture{
public:	
	AABB size;
	AABB extent;
	std::string filename;
	png_bytep image_data;
	
	rawTexture();
	virtual ~rawTexture();
};

