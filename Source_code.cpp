#pragma comment(lib, "Winscard.lib")

#include <stdio.h>
#include <stdlib.h>
#include <winscard.h>
#include <string>
#include <iostream>
//#include <conio.h>

#include "PerfromCommand.h"

using namespace std;

int main()
{
	EstContext();
	FindReaders();
	ConnectCard();
	string command;
	while (command != "Exit")
	{
		cout << ">";
		getline(cin,command);
		if (command != "Exit") PerformCommand(command);
		cout << endl;
	}

	CloseAndFree();

	//system("pause");
	return 0;
}