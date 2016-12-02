/*
Converts a BMP image to an HTML table of colored cells.
Each cell is 1 pixel in width and height.
*/

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <string>

using namespace std;

int main(int argc, char* argv[])
{
	//Display usage if incorrect number of parameters
	if (argc != 3) {
		cout << "Converts a BMP image to an HTML table of colored cells." << endl << endl;
		cout << "BMPtoHTML.exe [input_bmp_filename] [output_html_filename]" << endl;
		return 0;
	}
	string bmpFileName = argv[1];
	string htmlFileName = argv[2];
	
	//Open input BMP file
	fstream bmpFile;
	bmpFile.open(bmpFileName, fstream::in | fstream::binary);
	if (!bmpFile.is_open()) {
		cout << "Error opening input BMP file " << bmpFileName << endl;
		return -1;
	}
	
	//Read BMP file header
	BITMAPFILEHEADER bFileHeader;
	bmpFile.read((char*)&bFileHeader, sizeof(bFileHeader));
	if ((bFileHeader.bfType != 0x4D42) &&//BM
		(bFileHeader.bfType != 0x4142) &&//BA
		(bFileHeader.bfType != 0x4943) &&//CI
		(bFileHeader.bfType != 0x5043) &&//CP
		(bFileHeader.bfType != 0x4349) &&//IC
		(bFileHeader.bfType != 0x5450)) {//PT
		cout << "Can't read this BMP file." << endl;
		cout << "bfType: " << bFileHeader.bfType << endl;
		bmpFile.close();
		return -1;
	}

	//Read size of info header to determine which struct to use
	unsigned int headerSize;
	bmpFile.read((char*)&headerSize, 4);
	bmpFile.seekg((int)bmpFile.tellg() - 4);

	BITMAPINFOHEADER bHeader;
	memset(&bHeader, 0, sizeof(bHeader));
	if (headerSize == 12) {
		bmpFile.read((char*)&bHeader, 12);
	}
	else if (headerSize == 40) {
		bmpFile.read((char*)&bHeader, 40);
		memcpy(&bHeader, &bHeader, 12);
	}
	else if (headerSize == 108) {
		BITMAPV4HEADER bHeaderExt;
		bmpFile.read((char*)&bHeaderExt, 108);
		memcpy(&bHeader, &bHeaderExt, 40);
	}
	else if (headerSize == 124) {
		BITMAPV5HEADER bHeaderExt;
		bmpFile.read((char*)&bHeaderExt, 124);
		memcpy(&bHeader, &bHeaderExt, 40);
	}
	else {
		cout << "Header unsupported. Length " << headerSize << endl;
		bmpFile.close();
		return -1;
	}
	
	//Check for unsupported features
	if (bHeader.biPlanes != 1) {
		cout << "Unsupported number of planes ("<< bHeader.biPlanes << ")" << endl;
		bmpFile.close();
		return -1;
	}
	if (bHeader.biBitCount != 24) {
		cout << "Unsupported number of bits per pixel (" << bHeader.biBitCount << ")" << endl;
		bmpFile.close();
		return -1;
	}
	if (bHeader.biCompression != BI_RGB) {//BI_RGB means uncompressed
		cout << "Compressed image formats not supported." << endl;
		bmpFile.close();
		return -1;
	}
	
	//Rows are rounded to 4 bytes
	//Calculate how many bytes to jump at the end of a row
	unsigned int requiredRowBytes = bHeader.biWidth * 3;
	int paddedBytes = (4 - (requiredRowBytes % 4)) % 4;
	unsigned int actualRowBytes = requiredRowBytes + paddedBytes;

	//Read the pixel data and close the BMP file
	unsigned int bytesToRead = actualRowBytes * bHeader.biHeight;
	char* pixelData = new char[bytesToRead];
	bmpFile.seekg(bFileHeader.bfOffBits);
	bmpFile.read(pixelData, bytesToRead);
	bmpFile.close();
	
	//Open output HTML file
	fstream htmlFile;
	htmlFile.open(htmlFileName, fstream::out | fstream::trunc);
	if (!htmlFile.is_open()) {
		cout << "Error opening output HTML file " << htmlFileName << endl;
		return -1;
	}

	//Write the HTML file and parse the colors simultaneously
	char htmlTableStart[] = "<html>\n<head>\n<style type=\"text/css\">\ntd {\n    height:1px;\n    width:1px;\n}\n</style>\n</head>\n<body>\n<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\">\n";
	htmlFile.write(htmlTableStart, strlen(htmlTableStart));
	for (int row = 0; row < bHeader.biHeight; ++row) {
		char htmlRowStart[] = "<tr>\n";
		htmlFile.write(htmlRowStart, strlen(htmlRowStart));
		unsigned int rowOffset = (bHeader.biHeight - row - 1) * actualRowBytes;
		for (int col = 0; col < bHeader.biWidth; ++col) {
			char htmlCellStart[] = "<td bgcolor=\"#";
			htmlFile.write(htmlCellStart, strlen(htmlCellStart));
			unsigned int colOffset = col * 3;
			//Read 3 bytes for BGR and write their RGB hex string
			for (int z = 2; z >= 0; --z) {
				int color = 0;//must be int for _itoa_s()
				((char*)&color)[0] = pixelData[rowOffset + colOffset + z];
				char hexcode[3];
				memset(hexcode, 0, sizeof(hexcode));
				_itoa_s(color, hexcode, sizeof(hexcode), 16);
				//if hexcode is 1 char long, need to write a 0 too
				if (hexcode[1] == 0) {
					hexcode[1] = hexcode[0];
					hexcode[0] = '0';
				}
				htmlFile.write(hexcode, 2);
			}
			char htmlCellEnd[] = "\"></td>\n";
			htmlFile.write(htmlCellEnd, strlen(htmlCellEnd));
		}
		char htmlRowEnd[] = "</tr>\n";
		htmlFile.write(htmlRowEnd, strlen(htmlRowEnd));
	}
	delete[] pixelData;
	char htmlTableEnd[] = "</table>\n</body>\n</html>\n";
	htmlFile.write(htmlTableEnd, strlen(htmlTableEnd));
	htmlFile.close();
	
    return 0;
}
