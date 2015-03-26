#ifndef PERFORMCOMMAND_H
#define PERFORMCOMMAND_H

#include "SmartCard.h"

int PerformCommand(string command)
{
	if (command.find("Help")!= -1)
	{
		Help();
		return 0;
	}
	if (command.find("LoadDefault") != -1)
	{
		LoadDefault();
		return 0;
	}
	if (command.find("AuthSector") != -1)//
	{
		int repeat = 1; //stoul(command.substr(14,2),nullptr,10);
		BYTE SectorNumber = stoul(command.substr(11,2),nullptr,10);
		if (command.length() > 13) repeat = stoul(command.substr(14,2),nullptr,10);
		for (int i = 1; i<= repeat; i++)
		{
			int ret = AuthSector(SectorNumber);
			if (ret == 0)
				printf("Success\n");
			if (ret == 1)
				printf("Failure\n");
			if (ret == -1)
				printf("Something is wrong. Maybe card has been disconnected\n");
		}
		return 0;
	}
	if (command.find("ShowAll") != -1)
	{
		int repeat = 1; //stoul(command.substr(14,2),nullptr,10);
		if (command.length() > 7) repeat = stoul(command.substr(8,2),nullptr,10);
		for (int i = 1; i<= repeat; i++)
		{
			ShowAll();
		}
		return 0;
	}
	if (command.find("ShowBlock") != -1)
	{
		int repeat = 1; //stoul(command.substr(14,2),nullptr,10);
		BYTE SectorNumber = stoul(command.substr(10,2),nullptr,10);
		BYTE BlockNumber =  stoul(command.substr(13,2),nullptr,10);
		if (command.length() > 15) repeat = stoul(command.substr(16,2),nullptr,10);
		for (int i = 1; i<= repeat; i++)
		{
			ReadBlock(SectorNumber,BlockNumber);
		}
		return 0;
	}
	if (command.find("ShowSector") != -1)
	{
		int repeat = 1; //stoul(command.substr(14,2),nullptr,10);
		BYTE SectorNumber = stoul(command.substr(11,2),nullptr,10);
		if (command.length() > 13) repeat = stoul(command.substr(14,2),nullptr,10);
		for (int i = 1; i<= repeat; i++)
		{
			ReadSector(SectorNumber);
		}
		return 0;
	}
	if (command.find("LoadAuth") != -1)
	{
		BYTE Location, Key[6];
		Location = stoul(command.substr(9,2),nullptr,10);
		for (int i = 0; i< 6; i++)
		{
			Key[i] = stoul(command.substr(12+i*2,2),nullptr,16);
			printf("%0.2X ",Key[i]);
		}
		LoadAuth(Location,Key);
		return 0;
	}
	if (command.find("LoadAllAuth") != -1)
	{
		BYTE Key[6];
		for (int i = 0; i< 6; i++)
		{
			Key[i] = stoul(command.substr(12+i*2,2),nullptr,16);
			printf("%0.2X ",Key[i]);
		}
		for (int i = 0; i<16; i++)
		{
			LoadAuth(i,Key);
		}
		return 0;
	}
	if (command.find("WriteBlock") != -1)
	{
		int repeat = 1;
		BYTE Data[16];
		BYTE SectorNumber = stoul(command.substr(11,2),nullptr,10);
		BYTE BlockNumber =  stoul(command.substr(14,2),nullptr,10);
		for (int i=0; i<16; i++)
		{
			Data[i] = stoul(command.substr(17+i*2,2),nullptr,16);
		}
		if (command.length() > 49) repeat = stoul(command.substr(50,2),nullptr,10);
		for (int i = 1; i<= repeat; i++)
		{
			WriteBlock(SectorNumber,BlockNumber,Data);
		} //WriteBlock 00 00 12345678901234567890123456789012 00
		return 0;
	}
	if (command.find("ShowKeys") != -1)
	{
		ShowKeys();
		return 0;
	}
	if (command.find("SaveKeys") != -1)
	{
		SaveKeys();
		return 0;
	}
	if (command.find("LoadKeys") != -1)
	{
		LoadingKey();
		return 0;
	}
	if (command.find("ChangeKey") != -1)
	{
		string arg = command.substr(10,1);
		BYTE SectorNumber, Key[6];
		SectorNumber = stoul(command.substr(12,2),nullptr,10);
		for (int i = 0; i< 6; i++)
		{
			Key[i] = stoul(command.substr(15+i*2,2),nullptr,16);
			printf("%0.2X ",Key[i]);
		} //ChangeKey x 00 123456789012 aa
		int repeat = 1; //stoul(command.substr(14,2),nullptr,10);
		if (command.length() > 27) repeat = stoul(command.substr(28,2),nullptr,10);
		for (int i = 1; i<= repeat; i++)
		{
			ChangeKey(SectorNumber,Key,arg);
		}
		return 0;
	}
	if (command.find("Dump") != -1)
	{
		Dump();
		return 0;
	}
	if (command.find("Brain") != -1)
	{
		printf("Same thing we do every night Pinky -\ntry to take over the crypto-world!\n");
		return 0;
	}
	printf("Unknown command\n");
	return 0;
};

#endif