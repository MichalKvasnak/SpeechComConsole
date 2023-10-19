#include "SpeechConsole.h"

int ErrorHandle(int errorValue, char* errorMessage)
{
	fprintf(stderr, "\n\n### ERROR: %s ###\n\n", errorMessage);

	return errorValue;
}
HANDLE SerialCommSetup(TCHAR* comName, DWORD baudRate, BYTE byteSize, BYTE parity, BYTE stopBits)
{
	/* Struktura pro seriovou komunikaci */
	DCB dcb;

	/* Priznak uspesneho nastaveni */
	BOOL fSuccess;

	/* Nazev portu, na kterem je pripojena vyvojova deska */
	TCHAR* pcCommPort = comName;

	/* Struktura pro nastaveni casovych limitu */
	COMMTIMEOUTS timeouts = { 0 };

	/* Struktura pro manipulaci se seriovou komunikaci */
	HANDLE hCom = { 0 };

	/* Vytvoreni nastaveni seriove komunikace */
	hCom = CreateFile(pcCommPort,						// Nazev portu, kde je pripojena vyvojova deska
		GENERIC_READ | GENERIC_WRITE,	// Mod zapisu i cteni
		0,								// Exkluzivni pristup
		NULL,							// Zakladni zabezpeceni
		OPEN_EXISTING,					// Otevre jiz existujici nastaveni
		0,								// Asynchronni komunikace
		NULL);							// (hodnota musi byt nulova)

	/* Testovani platneho vytvoreni nastaveni seriove komunikace*/
	if (hCom == INVALID_HANDLE_VALUE)
	{
		printf("Vytvoreni nastaveni seriove komunikace selhalo! Error: %d.\n", GetLastError());
		return hCom;
	}

	/* Inicializace DCB struktury */
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	/* Ziskani jiz existujicich parametru */
	fSuccess = GetCommState(hCom, &dcb);

	if (!fSuccess)
	{
		printf("Ziskani existujicich parametru selhalo! Error: %d.\n", GetLastError());
		return hCom;
	}

	/* Nastaveni rychlosti prenosu*/
	dcb.BaudRate = baudRate;
	/* Nastaveni poctu datovych bitu */
	dcb.ByteSize = byteSize;
	/* Generovani parity */
	dcb.Parity = parity;
	/* Pocet stop bitu*/
	dcb.StopBits = stopBits;

	/* Overeni nastaveni parametru */
	fSuccess = SetCommState(hCom, &dcb);

	if (!fSuccess)
	{
		printf("Nastaveni novych parametru selhalo! Error: %d.\n", GetLastError());
		return hCom;
	}

	/* Casovy limit mezi prijimanymi Byty */
	timeouts.ReadIntervalTimeout = 100;
	/* Celkovy casovy limit prijimani dat*/
	timeouts.ReadTotalTimeoutConstant = 500;
	/* Nasobne rozsireni casoveho limitu pro prijimani dat*/
	timeouts.ReadTotalTimeoutMultiplier = 1;
	/* Celkovy casovy limit pro odesilani dat*/
	timeouts.WriteTotalTimeoutConstant = 500;
	/* Nasobne rozsireni casoveho limitu pro odesilani dat*/
	timeouts.WriteTotalTimeoutMultiplier = 1;

	/* Overeni nastaveni casovych limitu */
	if (SetCommTimeouts(hCom, &timeouts) == 0)
	{
		fprintf(stderr, "Nastaveni casovych limitu selhalo!\n");
		CloseHandle(hCom);
		return hCom;
	}

	/* Overeni spravnosti nastaveni seriove komunikace */
	fSuccess = GetCommState(hCom, &dcb);

	if (!fSuccess)
	{
		printf("Nastaveni seriove komunikace selhalo! Error: %d.\n", GetLastError());
		return hCom;
	}

	_tprintf(TEXT("\n### Seriovy port %s byl uspesne prenastaven! ###\n\n"), pcCommPort);
	return hCom;
}

bool EndsWith(const char* fString, const char* fEnding)
{
	size_t fStringLen = strlen(fString);
	size_t fEndingLen = strlen(fEnding);

	return (fStringLen >= fEndingLen) && (0 == strcmp(fString + (fStringLen - fEndingLen), fEnding));
}

bool MakeWav(char* generatedSpeech, size_t speechSize, char* fileName)
{
	FILE* waveFile;
	size_t waveSize;

	char finalFileName[FILE_NAME_BUFFER_SIZE * 2] = { 0 };

	strcat(finalFileName, FILE_PATH);
	strcat(finalFileName, fileName);

	if (!generatedSpeech || !fileName || !(waveFile = fopen(finalFileName, "w")))
		return false;

	/*****************************
	**		FMT SUB-CHUNK		**
	*****************************/
	const char subchunk1ID[] = { 'f', 'm', 't', ' ' };
	const unsigned int subChunk1Size = 16;
	const unsigned short audioFormat = 1;
	const unsigned short numChannels = 1;
	const unsigned int sampleRate = BAUD_RATE / 8; // BAUD_RATE / bits per sample

	const unsigned short bitsPerByte = 8;
	const unsigned short bitsPerSample = 8;
	const unsigned int byteRate = (sampleRate * numChannels * bitsPerSample) / bitsPerByte;

	const unsigned short blockAlign = (numChannels * bitsPerSample) / bitsPerByte;

	/*****************************
	**		DATA SUB-CHUNK		**
	*****************************/
	const char subchunk2ID[] = { 'd', 'a', 't', 'a' };
	const int subchunk2Size = (speechSize * numChannels * bitsPerSample) / bitsPerByte;

	/*************************
	**		RIFF CHUNK		**
	*************************/
	const char chunkID[] = { 'R', 'I', 'F', 'F' };
	const int chunkSize = 36 + subchunk2Size;
	const char format[] = { 'W', 'A', 'V', 'E' };

	// Zapis 'RIFF' chunku:
	fwrite(&chunkID, 1, sizeof(chunkID), waveFile);
	fwrite(&chunkSize, sizeof(chunkSize), 1, waveFile);
	fwrite(&format, 1, sizeof(format), waveFile);

	// Zapis 'FMT' sub-chunku:
	fwrite(&subchunk1ID, 1, sizeof(subchunk1ID), waveFile);
	fwrite(&subChunk1Size, sizeof(subChunk1Size), 1, waveFile);
	fwrite(&audioFormat, sizeof(audioFormat), 1, waveFile);
	fwrite(&numChannels, sizeof(numChannels), 1, waveFile);
	fwrite(&sampleRate, sizeof(sampleRate), 1, waveFile);
	fwrite(&byteRate, sizeof(byteRate), 1, waveFile);
	fwrite(&blockAlign, sizeof(blockAlign), 1, waveFile);
	fwrite(&bitsPerSample, sizeof(bitsPerSample), 1, waveFile);

	// Zapis 'DATA' sub-chunku
	fwrite(&subchunk2ID, 1, sizeof(subchunk2ID), waveFile);
	fwrite(&subchunk2Size, sizeof(subchunk2Size), 1, waveFile);

	waveSize = fwrite(generatedSpeech, sizeof(char), speechSize * numChannels, waveFile);
	fclose(waveFile);
	return true;
}