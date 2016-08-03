// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


using namespace boost::filesystem;

int main(int argc, char *argv[]) {
	path p(argc>1 ? argv[1] : ".");

	if (is_directory(p)) {
		std::cout << p << " is a directory containing:\n";

		for (auto& entry : boost::make_iterator_range(directory_iterator(p), {})) {			
			std::string filename = entry.path().filename().generic_string();
			if (filename.substr(filename.length()-4) == ".jpg")
			{
				std::cout << filename.substr(0, filename.length() - 4) << ".dat\n";

			}
			//entry.replace_filename()
		}
	}
}
