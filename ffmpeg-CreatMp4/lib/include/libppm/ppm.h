//Process a binary PPM file
#include <vector>
//#include "string"
class ppm
{
	void init();
	//info about the PPM file (height, width)
	unsigned int nr_lines;
	unsigned int nr_columns;
public:
	//arrays for storing the R,G,B values
	std::vector<uint8_t > r;
	std::vector<uint8_t > g;
	std::vector<uint8_t > b;
	
	unsigned int height;
	unsigned int width;
	unsigned int max_col_val;
	//total number of elements (pixels)
	unsigned int size;
	ppm();
	//create a PPM object and fill it with data stored in fname
	ppm(const std::string &fname);
	//create an "empty" PPM image with a given width and height;the R,G,B are filled with zeros
	ppm(const unsigned int _width, const unsigned int _height);
	//read the PPM image from fname
	void read(const std::string &fname);
	//write the PPM image to fname
	void write(const std::string &fname);
};
