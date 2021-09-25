#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

int imageTGA(std::string &fileIn, std::string &fileOut, int xTile, int yTile) {

	std::vector<unsigned char> headerData, headerDataNew;
	std::vector<unsigned char> idData;
	unsigned int idLength, xOrigin, yOrigin, width, height;

	//open input file
	std::cout << "Reading " << fileIn << "...\n";
	std::ifstream file;
	file.open(fileIn, std::ios_base::binary);

	if (!file.good()) {
		std::cout << "error opening input file\n";
		return 1;
	}

	//read header
	while (file.tellg() < 18) {
		headerData.push_back(file.get());
	}

	//parse header
	idLength = headerData[0];
	xOrigin = headerData[8] + (headerData[9] << 8);
	yOrigin = headerData[10] + (headerData[11] << 8);
	width = headerData[12] + (headerData[13] << 8);
	height = headerData[14] + (headerData[15] << 8);


	//read id
	while (file.tellg() < (idLength + 18)) {
		idData.push_back(file.get());
	}


	int widthNew = width * xTile;
	int heightNew = height * yTile;

	std::cout << "New dimensions: " << widthNew << " x " << heightNew << "\n";

	if ((widthNew > 65535) || (heightNew > 65535)) {
		std::cout << "max tga dimensions are 65535 by 65535\n";
		return 1;
	}


	//update header
	headerDataNew = headerData;
	headerDataNew[13] = (widthNew) / 256;
	headerDataNew[12] = (widthNew) - (headerDataNew[13] << 8);
	headerDataNew[15] = (heightNew) / 256;
	headerDataNew[14] = (heightNew) - (headerDataNew[15] << 8);
	
	std::cout << "New size : " << headerDataNew.size() + idLength + heightNew * widthNew * 3 << " bytes\n";
	std::cout << "Tiling " << xTile << " x " << yTile << "...\n";


	//open output file
	std::fstream outFile;
	outFile.open(fileOut, std::fstream::in | std::fstream::out | std::ios_base::binary | std::fstream::trunc);

	//write new header
	for (auto c : headerDataNew) {
		outFile << c;
	}



	std::vector<unsigned char> buffer;
	buffer.resize(width * 3);
	int bufferSize = width * 3;
	buffer.resize(bufferSize);
	
	//write first row
	file.seekg(18 + idLength);
	for (int h = 0; h < height; h++) {
		file.read((char*)buffer.data(), bufferSize);
		for (int x = 0; x < xTile; x++) {
			outFile.write((char*)buffer.data(), bufferSize);
		}
	}

	//tile first row vertically
	outFile.seekg(18 + idLength);
	bufferSize = widthNew * height * 3;
	buffer.resize(bufferSize);
	outFile.read((char*)buffer.data(), bufferSize);

	for (int y = 1; y < yTile; y++) {
		outFile.write((char*)buffer.data(), bufferSize);
	}
		

	file.close();
	std::cout << "Done.\n";

	return 0;
}


int main(int argc, char* argv[])
{
	auto clock = std::chrono::system_clock();
	auto startTime = clock.now();

	if (argc != 5) {
		std::cout << "arguments: [input file] [output file] [x tiles] [y tiles]\n";
		return 1;
	}

	std::string fileIn = argv[1];
	std::string fileOut = argv[2];
	int xTile = atoi(argv[3]);
	int yTile = atoi(argv[4]);

	if (std::filesystem::path(fileIn).extension() == ".tga") {
			imageTGA(fileIn, fileOut, xTile, yTile);
	}
	else {
		std::cout << "invalid filetype\n";
		return 1;
	}

	auto endTime = clock.now();
	std::cout << "Elapsed time: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() << " ms\n";
}