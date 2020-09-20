#include "ValueInspector.h"

int main(int argc, char* argv[])
{	
	char str[] = "somewhere in memory";
	// cout << &str << endl;
	// cin.get();

	ValueInspector vins(str, sizeof(str));
	vins.setColorTheme("classic");
	vins.setCmdFormat(1200, 800, 0, 0);
	vins.setDispDataFormat(16, 4, 15, 6, "hex");
	if(argc == 1) vins.refreshScreen();

	vins.showConsole(argc, argv);

	return 0;
}