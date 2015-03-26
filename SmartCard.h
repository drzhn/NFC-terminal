#ifndef SMARTCARD_H
#define SMARTCARD_H

#include <winscard.h>
#include <stdio.h>
#include <string>

using namespace std;

SCARDCONTEXT	hContext;
SCARDHANDLE     hCardHandle;
LONG            lReturn, lReturn2;
LPTSTR          pmszReaders = NULL;
DWORD           cch = SCARD_AUTOALLOCATE;
DWORD           dwAP;
LPTSTR          pReader,ListOfReaders[10];

LPCBYTE			SendBuffer;
DWORD			SendLength;
LPBYTE			RecvBuffer;
DWORD			RecvLength;

BYTE keyList [16][6];

//В каждой из команд <commandname>[3] -- номер блока начиная с 0x00
BYTE LoadKey[] =	{0xFF,0x82,0x00,0x00,0x06,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; // ключ для загрузки в память ридера
BYTE Auth[] =		{0xFF,0x86,0x00,0x00,0x05,0x01,0x00,0x03,0x60,0x00};//Аутентификация
BYTE Read[] =		{0xFF,0xB0,0x00,0x00,0x10};// чтение блока
BYTE Write[] =		{0xFF,0xD6,0x00,0x04,0x10,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}; //21

//в начале работы следует вызвать по очереди функции EstContext, FindReaders, ConnectCard
//в конце обязательно CloseAndFree

int EstContext();
int FindReaders();
int ConnectCard();
int CloseAndFree();
int Help();
int LoadDefault();
int AuthSector(BYTE BlockNumber); 
int LoadAuth(BYTE Location, BYTE Key[6]);
int ShowAll(); 
int ReadBlock(BYTE SectorNumber,BYTE BlockNumber);
int ReadSector(BYTE SectorNumber);
int WriteBlock(BYTE SectorNumber, BYTE BlockNumber, BYTE Data[16]);
int ChangeKey(BYTE SectorNumber,BYTE Key[6], string arg);//
int ShowKeys();
int SaveKeys();
int LoadingKey();
int Dump();//

int EstContext()
{
	for (int i = 0; i< 32; i++) // загружаем в кей-лист дефолтные ключи
	{
		for (int j = 0; j<6; j++)
		{
			keyList[i][j] = 0xFF;
		}
	}
	printf("Establish context of smart card...");
	lReturn = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
	if (lReturn != SCARD_S_SUCCESS)
	{
		printf(" error\n");
		getchar();
		return 1;
	} else
	{
		printf(" OK\n\n");
		return 0;
	}
};

int FindReaders()
{
	lReturn = SCardListReaders(hContext,NULL,(LPTSTR)&pmszReaders,&cch );
	while (lReturn == SCARD_E_NO_READERS_AVAILABLE)
	{
		printf("Please, insert smart card reader and press Enter to continue\n");
		getchar();
		lReturn = SCardListReaders(hContext,NULL,(LPTSTR)&pmszReaders,&cch );
	}

	//in case SCARD_S_SUCCESS:
	//выводим список ридеров
	int SIndex = 0;
	pReader = pmszReaders;
	ListOfReaders[SIndex] = pReader;
	printf("Connecting readers:\n");
	while ( '\0' != *pReader )
	{
		// Display the value.
		printf("#%d: %S\n",SIndex+1, ListOfReaders[SIndex] );
		// Advance to the next value.
		pReader = pReader + wcslen((wchar_t *)pReader) + 1;
		SIndex += 1;
		ListOfReaders[SIndex] = pReader;
	}
	printf("\nUse reader #%d...\n",1); // Здесь должен будет осуществляться выбор ридера, оставим нулевой по умолчанию
	return 0;
};

int ConnectCard()
{
	lReturn = SCardConnect( hContext, 
		//(LPCTSTR)"Rainbow Technologies SCR3531 0",
		ListOfReaders[0],
		SCARD_SHARE_SHARED,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		&hCardHandle,
		&dwAP );
	while ( SCARD_S_SUCCESS != lReturn )
	{
		printf("Please, insert smart card and press Enter to continue\n");
		getchar();
		lReturn = SCardConnect( hContext, 
			ListOfReaders[0],
			SCARD_SHARE_SHARED,
			SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
			&hCardHandle,
			&dwAP );
	}

	printf("Card successfully connected with ");
	switch ( dwAP )
	{
	case SCARD_PROTOCOL_T0:
		printf("protocol T0\n\n"); 
		break;

	case SCARD_PROTOCOL_T1:
		printf("protocol T1\n\n"); 
		break;

	case SCARD_PROTOCOL_UNDEFINED:
	default:
		printf(" unnegotiated or unknown protocol\n\n"); 
		break;
	}
	return 0;
};

int CloseAndFree()
{
	//осуществляем выход, отсоединяем карту и освобождаем память контекста
	lReturn = SCardDisconnect(hCardHandle, SCARD_LEAVE_CARD);
	if ( SCARD_S_SUCCESS != lReturn )
	{
		printf("Failed SCardDisconnect\n");
		return 1;
	}
	lReturn2 = SCardFreeMemory( hContext,pmszReaders );
	if ( SCARD_S_SUCCESS != lReturn2 )
	{
		printf("Failed SCardFreeMemory\n");
		return 1;
	}
	return 0;
};

int Help()
{
	printf("LoadDefault -- Loading default keys (FF..FF) to the keylist\n\n");
	printf("AuthSector DD -- This command uses the keys stored in the keylist\n");
	printf("                             to do authentication\n\n");
	printf("LoadAuth DD XX..XX(6 bytes) -- Loading authentication keys\n"); 
	printf("                             to the keylist. The authentication\n"); 
	printf("                             keys are used to authenticate\n");
	printf("                             the particular sector of the memory card\n\n");
	printf("ShowAll -- Show all data on the card\n\n");
	printf("ShowBlock DD(sector number) DD(number of block in the sector)\n");
	printf("                             -- Show block stored in the card\n\n");
	printf("ShowSector DD(sector number) -- Show sector stored in the card\n\n");
	printf("WriteBlock DD(sector number) DD(number of block in the sector) XX..XX(16 bytes)\n");
	printf("                             -- Write a block in the card. The data\n");
	printf("                             block/trailer block must be authenticated\n"); 
	printf("                             first before executing this command.\n\n");
	printf("ChangeKey T DD(sector number) XX..XX(6 bytes)\n");
	printf("                             -- This command is used for changing key\n");
	printf("                             for sector and loading key to the keylist\n");
	printf("                             T - type. T may be a, b or x\n\n");
	printf("ShowKeys -- Show all keys stored in the keylist\n\n");
	printf("SaveKeys -- Save all keys from the keylist to KEYS.txt\n\n");
	printf("LoadKeys -- Load all keys from KEYS.txt the keylist\n\n");
	printf("(DD is dec number, XX is hex value)\n");
	return 0;
};

int LoadDefault()
{
	for (int i = 0; i< 32; i++) // загружаем в кей-лист дефолтные ключи
	{
		for (int j = 0; j<6; j++)
		{
			keyList[i][j] = 0xFF;
		}
	}
	LoadKey[3] = 0x00;
	printf("Success\n");
	return 0;
};

int SaveKeys()
{
	FILE *f = fopen("KEYS.txt","w+");
	if (f != NULL)
	{
		for (int i = 0; i<16; i++) 
		{
			for (int j = 0; j<6; j++)
			{
				//keyList[i][j] = 0xFF;
				fprintf(f,"%0.2X ",keyList[i][j]);
			}
			fprintf(f,"\n");
		}
		fclose(f);
		printf("Success\n");
	} else printf("Failure\n");
	return 0;
};

int LoadingKey()
{
	FILE *f = fopen("KEYS.txt","r");
	if (f != NULL)
	{
		for (int i = 0; i< 16; i++) 
		{
			for (int j = 0; j<6; j++)
			{

				fscanf(f,"%0.2X ",&keyList[i][j]);
			}
			fscanf(f,"\n");
		}
		fclose(f);
		printf("Success\n");
	} else printf("Failure\n");
	return 0;
};

int AuthSector(BYTE SectorNumber) //Авторизируем сектор ключом 
{
	LoadKey[3] = 0x00;
	for (int j = 5; j<=10; j++)
	{
		LoadKey[j] = keyList[SectorNumber][j-5];
	}
	SendBuffer = &LoadKey[0];
	SendLength = sizeof(LoadKey);
	RecvBuffer = new BYTE[2];
	RecvLength = 2;
	lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,SendLength,NULL,RecvBuffer,&RecvLength);
	if (lReturn == SCARD_S_SUCCESS)
	{
		//printf("%X %X\n",RecvBuffer[0],RecvBuffer[1]);
		if ((RecvBuffer[0] != 0x90)&&(RecvBuffer[1] != 0x00))
		{
			delete RecvBuffer;
			return 1;
		}
	} else return -1;

	delete RecvBuffer;

	//BYTE Auth[] = {0xFF,0x86,0x00,0x00,0x05,0x01,0x00,0x03,0x60,0x00};//10
	Auth[7] = SectorNumber * 4;

	SendBuffer = &Auth[0];
	SendLength = sizeof(Auth);
	RecvBuffer = new BYTE[2];
	RecvLength = 2;

	lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,SendLength,NULL,RecvBuffer,&RecvLength);
	if (lReturn == SCARD_S_SUCCESS)
	{
		//printf("%X %X\n",RecvBuffer[0],RecvBuffer[1]);
		if ((RecvBuffer[0] == 0x90)&&(RecvBuffer[1] == 0x00))
		{
			delete RecvBuffer;
			return 0;
		}
		else
		{
			delete RecvBuffer;
			return 1;
		}

	} else 
	{
		delete RecvBuffer;
		return -1;
	}

	delete RecvBuffer;
	return 0;
};

int LoadAuth(BYTE Location, BYTE Key[6]) // загружаем ключ Key[6] в KeyList
{
	for (int i = 5; i<=10; i++)
	{
		keyList[Location][i-5] = Key[i-5];
	}
	printf("Success\n");
	return 0;
}


int ShowAll()
{
	RecvBuffer = new BYTE[18];
	RecvLength = 18;
	for (int i = 15; i>=0; i--)
	{
		printf("---------------------SECTOR %2d--------------------\n", i);
		int ret = AuthSector(i);
		if (ret != 0)
		{
			printf("Error: can't authenticate this sector\n");
			//return 1;
		}
		else
		{
			printf("   00-01-02-03-04-05-06-07-08-09-10-11-12-13-14-15\n", i);
			//считываем
			for (int j = 3; j>=0; j--)
			{
				RecvBuffer = new BYTE[18];
				RecvLength = 18;
				//BYTE Read[] = {0xFF,0xB0,0x00,0x00,0x10};//5
				Read[3] = 4*i+j;
				SendBuffer = &Read[0];
				SendLength = sizeof(Read);
				lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,SendLength,NULL,RecvBuffer,&RecvLength);
				printf("%0.2d:",j);
				for (int k = 0; k < 16; k++)
				{
					printf("%0.2X ",RecvBuffer[k]);
				}
				printf("\n");
				delete RecvBuffer;
			}
		}

	}
	return 0;
};

int ChangeKey(BYTE SectorNumber, BYTE Key[6], string arg)
{
	AuthSector(SectorNumber);
	for (int i = 0; i< 6; i++)
	{
		keyList[SectorNumber][i] = Key[i];
	}
	Read[3] = SectorNumber * 4 + 3;
	SendBuffer =  &Read[0];
	RecvBuffer = new BYTE[18];
	RecvLength = 18;
	lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,5,NULL,RecvBuffer,&RecvLength);
	if (RecvBuffer[16] !=0x90)
	{
		printf("Can't read trailer block\n");
		delete RecvBuffer;
		return 0;
	}
	printf("\n");
	for (int i = 0; i <= 16; i++)
	{
		printf("%0.2X ",RecvBuffer[i]);
	}
	printf("\n");

	for (int i = 0; i <= 20; i++)
	{
		printf("%0.2X ",Write[i]);
	}
	printf("\n");

	Write[3] = SectorNumber * 4 + 3;
	for (int i = 5; i <= 20; i++)
	{
		Write[i] = RecvBuffer[i-5];
	}

	for (int i = 15; i <= 20; i++)
	{
		if ((arg == "A")||(arg == "a"))
		{
			Write[i-10] = Key[i-15]; // keyA
		}
		if ((arg == "B")||(arg == "b"))
		{
			Write[i] = Key[i-15]; // keyB
		}
		if ((arg == "X")||(arg == "x"))
		{
			Write[i-10] = Key[i-15]; // keyA
			Write[i] = Key[i-15]; // keyB
		}
	}

	for (int i = 0; i <= 20; i++)
	{
		printf("%0.2X ",Write[i]);
	}
	printf("\n");
	delete RecvBuffer;
	SendBuffer = &Write[0];
	RecvBuffer = new BYTE[20];
	RecvLength = sizeof(RecvBuffer);
	lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,21,NULL,RecvBuffer,&RecvLength);
	if (RecvBuffer[0]==0x90)
	{
		printf("Success\n");
	}
	else
	{
		printf("Something wrong. Maybe card has been disconnected\nor sector is not authenticated\n");
	}
	delete RecvBuffer;
	return 0;
};

int ShowKeys ()
{
	for (int i = 0; i<=15; i++)
	{
		printf("%0.2d: ",i);
		for (int j=0; j<6; j++)
		{
			printf("%0.2X ",keyList[i][j]);
		}
		printf("\n");
	}
	return 0;
}



int ReadSector(BYTE SectorNumber)
{
	//BYTE Read[] = {0xFF,0xB0,0x00,0x00,0x10};//5
	SendBuffer =  &Read[0];
	RecvBuffer = new BYTE[18];
	RecvLength = 18;
	for (int j = 0; j<=3; j ++)
	{
		Read[3] = SectorNumber*4 + j;
		lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,5,NULL,RecvBuffer,&RecvLength);
		if (RecvBuffer[16]==0x90)
		{
			for (int i = 0; i< 16; i++)
			{
				printf("%0.2X ",RecvBuffer[i]);
			}
			printf("\n");
		}
		else
		{
			printf("Something wrong. Maybe card has been disconnected\nor sector is not authenticated\n");
			delete RecvBuffer;
			return 0;
		}
	}

	printf("\n");
	delete RecvBuffer;
	return 0;
}

int ReadBlock(BYTE SectorNumber,BYTE BlockNumber)
{
	//BYTE Read[] = {0xFF,0xB0,0x00,0x00,0x10};//5
	Read[3] = SectorNumber * 4 + BlockNumber;
	SendBuffer =  &Read[0];
	RecvBuffer = new BYTE[18];
	RecvLength = 18;

	lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,5,NULL,RecvBuffer,&RecvLength);
	if (RecvBuffer[16]==0x90)
	{
		for (int i = 0; i< 16; i++)
		{
			printf("%0.2X ",RecvBuffer[i]);
		}
		printf("\n");
	}
	else
	{
		printf("Something wrong. Maybe card has been disconnected\nor sector is not authenticated\n");
	}

	printf("\n");
	delete RecvBuffer;
	return 0;
};

int Dump()
{
	FILE *f = fopen("DUMP","wb");
	for (int SectorNumber = 0; SectorNumber <= 15; SectorNumber++)
	{
		AuthSector(SectorNumber);
		//printf("Success\n");
		for (int BlockNumber = 0; BlockNumber <= 3; BlockNumber++)
		{
			RecvBuffer = new BYTE[20];
			RecvLength = 20;
			SendBuffer =  &Read[0];
			Read[3] = SectorNumber * 4 + BlockNumber;
			lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,5,NULL,RecvBuffer,&RecvLength);
			if (RecvBuffer[16]==0x90)
			{
				fwrite(RecvBuffer,16,1,f);
				/*for (int i = 0; i<=15; i++)
				{
					printf("%0.2X ", RecvBuffer[i]);
				}*/
				//printf("\n");
			}
			delete RecvBuffer;
		}
	}
	fclose(f);
	return 0;
}

int WriteBlock(BYTE SectorNumber, BYTE BlockNumber, BYTE Data[16])
{
	if (BlockNumber == 3) 
	{
		printf("Are you sure you want to write to the trailer block? Y/N\n");
		char answer = '\n';
		scanf("%c",&answer);
		if ((answer == 'N')||(answer == 'n')) return 0;
		if ((answer != 'Y')&&(answer != 'y')) return 0;
	}

	SendBuffer = &Write[0];

	RecvBuffer = new BYTE[20];
	RecvLength = sizeof(RecvBuffer);
	Write[3] = SectorNumber * 4 + BlockNumber;
	for (int i = 5; i <= 20; i++)
	{
		Write[i] = Data[i-5];
	}

	lReturn = SCardTransmit(hCardHandle,SCARD_PCI_T1,SendBuffer,21,NULL,RecvBuffer,&RecvLength);

	if (RecvBuffer[0]==0x90)
	{
		printf("Success\n");
	}
	else
	{
		printf("Something wrong. Maybe card has been disconnected\nor sector is not authenticated\n");
	}
	delete RecvBuffer;
	return 0;
}


#endif /* SMARTCARD_H */