#pragma once
#include <vector>
#include "rawTexture.h"

class binPacker{
private:
	bool isWidthSmaller(AABB& extent);
	bool isHeightSmaller(AABB& extent);
	bool canContain(AABB& part, AABB& extent);
	AABB* findBest(AABB& extent);
	bool splitPartitionOnExtent(AABB& part, AABB& extent);
public:
	int width, height;
	std::vector<AABB> partitions;

	binPacker();
	virtual ~binPacker();

	bool start(int width, int height);
	bool clear();
	bool place(rawTexture& t);
};

