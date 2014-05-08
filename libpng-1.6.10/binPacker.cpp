#include "binPacker.h"


binPacker::binPacker(){
	width = 0;
	height = 0;
	partitions.clear();
}

binPacker::~binPacker(){
	printf("destroying bin packer\n");
	partitions.clear();
	width = 0;
	height = 0;
}

bool binPacker::isWidthSmaller(AABB& extent){
	return (extent.w() < extent.h());
}
bool binPacker::isHeightSmaller(AABB& extent){
	return (extent.h() < extent.w());
}

bool binPacker::canContain(AABB& part, AABB& extent){
	return (
		extent.w() <= part.w() &&
		extent.h() <= part.h()
		);	
}
AABB* binPacker::findBest(AABB& extent){
	AABB* chosen = NULL;

	for(unsigned i = 0; i < partitions.size(); ++i){
		if(canContain(partitions[i], extent)){
			if(chosen == NULL){
				chosen = &partitions[i];
			} else if(partitions[i].area() < chosen->area()){
				chosen = &partitions[i];		
			}
		}
	}
	return chosen;
}
bool binPacker::splitPartitionOnExtent(AABB& node, AABB& extent){
	AABB part;
	part.min = node.min;
	part.max = node.max;

	if(isWidthSmaller(extent)){
		part.min.y += extent.h();
		part.max.x = node.min.x + extent.w();		
		node.min.x += extent.w();
	} else {		
		part.min.x += extent.w();
		part.max.y = node.min.y + extent.h();				
		node.min.y += extent.h();
	}

	//printf("part = %d,%d,%d,%d  old = %d,%d,%d,%d\n",
	//	part.min.x,
	//	part.min.y,
	//	part.max.x,
	//	part.max.y,
	//	node.min.x,
	//	node.min.y,
	//	node.max.x,
	//	node.max.y		
	//	);
	if(part.area() > 0){
		partitions.push_back(part);
	}
	return !(node.area() < 0);
}
bool binPacker::start(int width, int height){
	this->width = width;
	this->height = height;
	AABB size;
	size.set(0, 0, width, height);
	this->partitions.push_back(size);
	return true;
}
bool binPacker::clear(){
	this->partitions.clear();
	this->width = 0;
	this->height = 0;
}

bool binPacker::place(rawTexture& t){
	AABB* best = findBest(t.size);
	if(best == NULL){ return false; }

	// set the texture extents
	t.extent.min.x = best->min.x;
	t.extent.min.y = best->min.y;
	t.extent.max.x = best->min.x + t.size.w();
	t.extent.max.y = best->min.y + t.size.h();

	if(splitPartitionOnExtent(*best, t.size) == false){
		// remove the partition from the list.
		std::vector<AABB>::iterator it;
		for(it = partitions.begin(); it != partitions.end(); it++){
			if(&(*it) == best){
				partitions.erase(it);
				break;
			}
		}
	}

	return true;
}





