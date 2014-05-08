#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_audio.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "png.h"
#include "AtlasGen.h"
#include "rawTexture.h"
#include "binPacker.h"
#include "render.h"


bool init();
void shutdown();
bool setDimensions(rawTexture& texture);
bool loadImageData(rawTexture& texture);
int texture_comp(const void* lhs, const void* rhs);
void sortTextures(rawTexture* textures, int size);
void dump_atlas(rawTexture* textures, int size, std::string fout);
void save_png(png_bytep* image_data, int width, int height, FILE* fout);
bool load_image_data(rawTexture* textures, int size);
png_byte* load_atlas_image(rawTexture* textures, int size, int width, int height);
void dump_png(png_byte* image, int width, int height, std::string fout);

bool init(){
	srand((unsigned)time(NULL));
	do{
		if(SDL_Init(SDL_INIT_EVERYTHING)){
			printf("SDL_Init Error\n");
			break;
		}

		if((IMG_Init(IMG_INIT_PNG)& IMG_INIT_PNG) != IMG_INIT_PNG){
			printf("IMG_Init PNG error\n");
			break;
		}

		if(TTF_Init() != 0){
			printf("TTF_Init error\n");
			break;
		}

		int flags = MIX_INIT_MP3;
		if((Mix_Init(flags) & flags) != flags){
			printf("Mix_Init error\n");
			break;
		}

		if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1){
			printf("Mix_OpenAudio error\n");
			break;
		}

		return true;
	} while(0);
	return false;
}
void shutdown(){
	Mix_CloseAudio();
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char** argv){
	if(!init()){ return 1; }
	AtlasGen atlas;
	std::map<std::string, std::string>* params;

	params = atlas.parse_arguments(argc, argv);
	if(params == NULL){
		atlas.print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	// read input file.
	std::string line;
	std::ifstream infile((*params)["inputfile"]);
	std::vector<std::string> imageNameList;
	if(!infile.is_open()){
		delete params;
		printf("could not open inputfile %s\n ", (*params)["inputfile"].c_str());
		exit(EXIT_FAILURE);
	}
	int count = 0;
	while(getline(infile, line)){
		if(line == "\n" || line ==""){ continue; }
		++count;
		imageNameList.push_back(line);
	}
	infile.close();	

	// create arary of textures.
	binPacker packer;
	rawTexture* imageList = new rawTexture[imageNameList.size()];
	int size = imageNameList.size();
	int width = atoi((*params)["width"].c_str());
	int height = atoi((*params)["height"].c_str());
	bool good_flag = true;
	
	// assign names to textures;
	// process extents dimensions for all the textures	
	for(int i = 0; i < size; ++i){
		imageList[i].filename = imageNameList[i];		
		if(setDimensions(imageList[i]) == false){
			printf("failed to set the dimensions for image %s\n", imageNameList[i].c_str());
		}
	}

	// assign extents	based on binPacker stuff
	sortTextures(imageList, size);
	packer.start(width, height);
	for(int i = 0; i < size; ++i){
		if(packer.place(imageList[i]) == false){
			printf("packer.place failed %s\n", imageList[i].filename.c_str());
			good_flag = false;
			break;
		} 
	}

	if(good_flag){
		std::string atlas_output = (*params)["outputDirectory"] + "/" + (*params)["atlasfilename"];
		std::string atlas_png_output = (*params)["outputDirectory"] + "/" +(*params)["pngfilename"];		

		dump_atlas(imageList, size,atlas_output);

		if(load_image_data(imageList, size)){
			png_byte* atlas_image = load_atlas_image(imageList, size, width, height);
			if(atlas_image){
				dump_png(atlas_image, width, height,atlas_png_output);
				delete[] atlas_image;
			} else{
				printf("atlas image failed to be constructed\n");
			}
		}				
	} else{
		printf("Atlas was not dumped.\n");
	}

	printf("%s completed.", argv[0]);
	//render r;
	//r.run(imageList,size);
	//r.draw((*params)["pngfilename"]);

	delete[] imageList;
	delete params;
	shutdown();

	char c;
	scanf("%c",&c);
	return 0;
}


bool setDimensions(rawTexture& texture){
	// read image data from png file.
	FILE* fin = fopen(texture.filename.c_str(), "rb");
	if(!fin){ return false; }

	do{
		const int png_header_size = 8;
		png_byte header[png_header_size];
		png_structp png_ptr = NULL;
		png_infop png_info = NULL;

		if(fread(header, sizeof(png_byte), png_header_size, fin) == 0){
			printf("fread header\n");
			break;
		};
		if(png_sig_cmp(header, 0, png_header_size)){
			printf("png_sig_cmp error not a png file %s\n",texture.filename.c_str());
			break;
		}
		
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(!png_ptr){
			printf("error png_create_read_struct\n");
			png_destroy_read_struct(&png_ptr, &png_info,NULL);
			break;
		}
		png_info = png_create_info_struct(png_ptr);
		if(!png_info){
			printf("error png_create_info_struct\n");
			png_destroy_read_struct(&png_ptr, &png_info, NULL);
			break;
		}

		// ERROR POINT
		if(setjmp(png_jmpbuf(png_ptr))){
			printf("Error, longjmp back to here\n");
			fclose(fin);
			return false;
		}

		png_init_io(png_ptr, fin);
		png_set_sig_bytes(png_ptr, png_header_size);
		png_read_info(png_ptr, png_info);
		
		texture.size.min.x = 0;
		texture.size.min.y = 0;
		texture.size.max.x = png_get_image_width(png_ptr, png_info);
		texture.size.max.y = png_get_image_height(png_ptr, png_info);

		//cleanup
		png_destroy_read_struct(&png_ptr, &png_info, NULL);
		fclose(fin);
		return true;
	} while(0);


	fclose(fin);
	return false;
}
bool loadImageData(rawTexture& texture){
	// load in image data.
	FILE* fin = fopen(texture.filename.c_str(), "rb");
	if(!fin){
		printf("Failed to open file [%s]\n", texture.filename.c_str());
		return false;
	}

	const int png_header_size = 8;
	const int pixel_size = 4;
	png_byte header[png_header_size];
	png_structp png_ptr = NULL;
	png_infop png_info = NULL;

	do{
		if(fread(header, sizeof(png_byte), 8, fin) == 0){
			printf("Failed to read header\n");
			break;
		}

		if(png_sig_cmp(header, 0, png_header_size) != 0){
			printf("Not a png file %s\n", texture.filename.c_str());
			break;
		}

		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
		if(!png_ptr){
			printf("Failed to crate read struct\n");
			break;
		}

		png_info = png_create_info_struct(png_ptr);
		if(!png_info){
			printf("Failed to create info struct\n");
			break;
		}

		if(setjmp(png_jmpbuf(png_ptr))){
			printf("Error, longjmped back here (before the read)\n");
			png_destroy_read_struct(&png_ptr, &png_info, NULL);
			fclose(fin);
			return false;
		}

		png_init_io(png_ptr, fin);
		png_set_sig_bytes(png_ptr, png_header_size);
		png_read_info(png_ptr, png_info);

		int width = png_get_image_width(png_ptr, png_info);
		int height = png_get_image_height(png_ptr, png_info);
		int bitDepth = png_get_bit_depth(png_ptr, png_info);
		int colorType = png_get_color_type(png_ptr, png_info);

		// standardize
		switch(colorType){
		case(PNG_COLOR_TYPE_RGB) :
			png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
			colorType = PNG_COLOR_TYPE_RGB_ALPHA;
			break;
		case(PNG_COLOR_TYPE_PALETTE) :
			png_set_palette_to_rgb(png_ptr);
			break;
		case(PNG_COLOR_TYPE_GRAY):
			if(bitDepth < 8){
				png_set_expand_gray_1_2_4_to_8(png_ptr);
				bitDepth = 8;
			}
			break;
		}

		if(png_get_valid(png_ptr, png_info, PNG_INFO_tRNS)){
			png_set_tRNS_to_alpha(png_ptr);
		}
		if(bitDepth == 16){
			png_set_strip_16(png_ptr);
		}
		png_read_update_info(png_ptr, png_info);

		// read in the image;
		const unsigned stride = width*pixel_size;
		png_bytep* rowPtrs = new png_bytep[height];
		png_byte* imageData = new png_byte[height*stride];

		if(setjmp(png_jmpbuf(png_ptr))){
			printf("Error, longjmped back here (before the read)\n");
			png_destroy_read_struct(&png_ptr, &png_info, NULL);
			delete[] rowPtrs;
			delete[] imageData;
			fclose(fin);
			return false;
		}

		for(int i = 0; i < height; ++i){
			//rowPtrs[i] = (png_bytep)(imageData + (height - i - 1)*stride);
			rowPtrs[i] = (png_bytep)(imageData + i*stride);
		}
		png_read_image(png_ptr, rowPtrs);


		texture.image_data = imageData;
		delete rowPtrs;
		png_destroy_read_struct(&png_ptr, &png_info, NULL);
		fclose(fin);
		return true;
	} while(0);


	png_destroy_read_struct(&png_ptr, &png_info, NULL);
	fclose(fin);
	return false;
}

int texture_comp(const void* lhs, const void* rhs){
	rawTexture* t1 = (rawTexture*)lhs;
	rawTexture* t2 = (rawTexture*)rhs;
	if(t1->size.area() < t2->size.area()){ return 1; }
	if(t1->size.area() == t2->size.area()){ return 0; }
	return -1;
}
void sortTextures(rawTexture* textures, int size){
	std::qsort(textures,size,sizeof(rawTexture), texture_comp);
}


void dump_atlas(rawTexture* textures, int size, std::string fout){
	FILE* f = fopen(fout.c_str(), "w");
	if(!f){ return; }
	printf("Dumping atlas to %s\n", fout.c_str());

	for(int i = 0; i < size; ++i){
		fprintf(stdout, "%s area = %5.2f %5.2f %5.2f %5.2f %5.2f\n",
			textures[i].filename.c_str(),
			(float)textures[i].extent.area(),
			(float)textures[i].extent.x(),
			(float)textures[i].extent.y(),
			(float)textures[i].extent.w(),
			(float)textures[i].extent.h()
			);
		fprintf(f, "%s %5.2f %5.2f %5.2f %5.2f\n",
			textures[i].filename.c_str(),			
			(float)textures[i].extent.x(),
			(float)textures[i].extent.y(),
			(float)textures[i].extent.w(),
			(float)textures[i].extent.h()
			);
	}
	fclose(f);
}


bool load_image_data(rawTexture* textures, int size){
	printf("loading all image data\n");
	for(int i = 0; i < size; ++i){
		if(loadImageData(textures[i]) == false){
			printf("Failed to load image dta [%s]\n", textures[i].filename.c_str());
			return false;
		}
	}
	return true;
}
png_byte* load_atlas_image(rawTexture* textures, int size, int width, int height){
	printf("loading all image data into single atlas image\n");
	const int pixel_size = 4;
	int atlas_area = width*height*pixel_size;
	png_byte* atlas_image = new png_byte[atlas_area];
	memset(atlas_image, 0xFFFFFFFF, atlas_area);

	int atlas_stride = width*pixel_size;
	int atlas_height = height;

	// copy image data into atlas image
	for(int i = 0; i < size; ++i){
		int atlas_x_offset = textures[i].extent.min.x*pixel_size;
		int texture_stride = textures[i].extent.w() * pixel_size;

		int row = textures[i].extent.min.y;
		int y = 0;
		for( /*row,y*/; row < textures[i].extent.max.y; ++row, ++y){
			png_byte* row_ptr = atlas_image + (row*atlas_stride + atlas_x_offset);
			memcpy(row_ptr, textures[i].image_data + (y*texture_stride), texture_stride);
		}
	}
	return atlas_image;	
}
void dump_png(png_byte* image, int width, int height, std::string fout){
	printf("dumping png to %s\n", fout.c_str());
	FILE* f = fopen(fout.c_str(), "wb");
	if(!f){ return; }

	const int pixel_size = 4;
	int stride = width*pixel_size;
	png_structp png_ptr;
	png_infop png_info;
	png_bytep* rowPtrs = new png_bytep[height];

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_info = png_create_info_struct(png_ptr);

	if(setjmp(png_jmpbuf(png_ptr))){
		printf("Error, longjmp back into here\n");
		delete[] rowPtrs;
		png_destroy_write_struct(&png_ptr, &png_info);		
		fclose(f);
	}

	png_init_io(png_ptr,f);
	png_set_IHDR(
		png_ptr, png_info,
		width, height, 8,
		PNG_COLOR_TYPE_RGB_ALPHA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
		);
	png_write_info(png_ptr, png_info);
	for(int i = 0; i < height; ++i){
		//rowPtrs[i] = image + (height - i - 1)*stride;
		rowPtrs[i] = image + i*stride;
	}

	png_set_rows(png_ptr, png_info, rowPtrs);
	png_write_png(png_ptr, png_info, PNG_TRANSFORM_IDENTITY, NULL);
	png_write_end(png_ptr, NULL);
	

	delete[] rowPtrs;
	png_free_data(png_ptr, png_info, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&png_ptr, &png_info);
	fclose(f);	
}

