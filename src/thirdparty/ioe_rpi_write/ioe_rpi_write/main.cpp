#include <iostream>
#include <fstream>

using std::ofstream;

int main(int argc, char **argv)
{
	if (argc != 2 && argc != 3)
	{
		std::cout << "ERROR: Usage - ioe_rpi_write <IOE_RPI_NAME> [Absolute Path]" << std::endl;
	}	

	std::string szOutputPath = "../../../src/IOE/IOERenderer/IOERPI_Defines.h";

	if (argc == 3)
	{
		szOutputPath = argv[2];
	}

	std::string szOutputText = \
		"//////////////////////////////////////////////////////////////////////////\n"
		"//                  GENERATED FILE, DO NOT MODIFY                         \n"
		"//////////////////////////////////////////////////////////////////////////\n"
		"\n"
		"#if !defined(IOE_RPI)\n"
		"	#define IOE_RPI ";
	szOutputText += argv[1];
	szOutputText += "\n"
		"#endif // !defined(IOE_RPI)";

	ofstream tOutputStream(szOutputPath, std::ios::out);
	if (tOutputStream.is_open())
	{
		tOutputStream.write(szOutputText.c_str(), szOutputText.size());
		tOutputStream.close();
	}

	return 0;
}