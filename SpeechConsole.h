#ifndef __SPEECHCONSOLE_H__
#define __SPEECHCONSOLE_H__

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>

/*************************************
**		RETEZCE PRO TESTOVANI		**
*************************************/

/* Pro otestovani je nutno dane retezce zkopirovat
**  a nasledne jej zadat do uzivatelske konzole.		*/

// A A' B C C^ D D^ E E' F G H CH I I' J K L M N N^ O O' P Q R R^ S S^ T T^ U U' V W X Y Y' Z Z^
// Tento r^ete^zec slouz^i' pro vyzkous^eni' funkc^nosti odesi'la'ni' vstupni'ho r^ete^zce a pr^iji'ma'ni generovane'ho hlasove'ho vy'stupu.
// Toto je hlas vas^eho poc^i'tac^e.w

/*****************************
**		DEFINICE FUNKCI		**
*****************************/
#define WAV_FILE 0
/* Rychlost prenosu (Musi byt stejna jako rychlost prenosu MCU)*/
#define BAUD_RATE 57600

/* Maximalni delka vstupniho retezce */
#define INPUT_BUFFER_SIZE 516

/* Maximalni delka prijimaneho retezce
** Pozn. delka je pocitana dle pismena [A'] (odesila nejvice Bytu - 1397)
**		tedy [(Maximalni delka retezce)*(Nejdelsi mozna bytova rada jednoho pismena)]
*/
#define RECEIVED_BUFFER_SIZE INPUT_BUFFER_SIZE*1400	

/* Maximalni delka nazvu souboru*/
#define FILE_NAME_BUFFER_SIZE 50		

/* Cesta do slozky, kde maji byt soubory ulozeny */
#define FILE_PATH "../A_ComConsole/Generovane_hlasove_vystupy/"

/*****************************
**		DEFINICE FUNKCI		**
*****************************/

/* Obsluha chyb
**	- 'errorValue' - navratova hodnota
**	- 'errorMessage' - zprava, ktera ma byt vypsana
*/
int ErrorHandle(int errorValue, char* errorMessage);

/* Inicializace seriove komunikace
**	- 'comName' - nazev portu, na kterem je pripojena vyvojova deska
**	- 'baudRate' - rychlost prenosu
**	- 'byteSize' - pocet datovych bitu
**	- 'parity' - generovana parita
**	- 'stopBits' - pocet stop bitu
**
** Pozn. funkce byla psana za pomoci dokumentace (viz https://learn.microsoft.com/en-us/windows/win32/api/winbase/ )
*/
HANDLE SerialCommSetup(TCHAR* comName, DWORD baudRate, BYTE byteSize, BYTE parity, BYTE stopBits);

/* Testovani, zda retezec konci urcitym podretezcem
**	- 'fString' - testovany retezec
**	- 'fEnding' - podretezec, kterym konci testovany retezec
*/
bool EndsWith(const char* fString, const char* fEnding);

/* Funkce generujici soubor '.wav'
**	- 'generatedSpeech' - generovany hlasovy vystup
**	- 'speechSize' - delka hlasoveho vystupu
**	- 'fileName' - nazev, pod kterym ma byt soubor ulozen
**
** Pozn. funkce byla psana pomoci dokumentace souboru '.wav' (viz https://www.videoproc.com/resource/wav-file.htm )
*/
bool MakeWav(char* generatedSpeech, size_t speechSize, char* fileName);

#endif

