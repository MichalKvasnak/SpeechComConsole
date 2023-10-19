#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "SpeechConsole.h"

int main( void )
	{
	/* Promenna pro nazev portu, na kterem je pripojena vyvojova deska */
	TCHAR wroteCom[5] = { 0 };	

	/* Zadani nazvu portu, na kterem je pripojena vyvojova deska */
	printf( "Zadejte COM port: " ); 
	if( !scanf( "%ls", wroteCom ) )
		return ErrorHandle(-99, "Obdrzeni nazvu COM selhalo!", NULL);
	if( !wroteCom )
		return ErrorHandle(-98, "Nazev COM je prazdny!", NULL);
	wroteCom[4] = '\0';

/* Inicializace seriove komunikace s parametry:
*	- Port:			(ten byl zadan v predchozim kodu)
*	- Baud rate:	(viz makro BAUD_RATE)
*	- Datove bity:	8
*	- Parita:		NONE
*	- Stop bit:		1 
*/
	HANDLE mainCom = SerialCommSetup( wroteCom, BAUD_RATE, DATABITS_8, NOPARITY, ONESTOPBIT);
	if( mainCom == INVALID_HANDLE_VALUE )
		return ErrorHandle( -97, "Nastaveni seriove komunikace selhalo!", mainCom );

	/* Promenna pro detekci odpovidajici chyby */
	int error = 0;

	/* Promenna pro prochazeni stringToSend */
	size_t stringToSend_Ptr = 0;
	
	/* Pocet odeslanych Bytu */
	DWORD bytesWritten = 0;

	/* Pocet obdrzenych Bytu */
	DWORD bytesRead = 0;

	/* Detekce odpovidajiciho eventu */
	DWORD eventMask = 0;

	/* Indikace pro konec vykonavani programu*/
	char doNext[4] = { 0 };

	/* Odesilany retezec */
	char* stringToSend = (char*)calloc( INPUT_BUFFER_SIZE, sizeof( char ) );

	/* Nazev souboru */
	char* fileName = (char*)calloc( FILE_NAME_BUFFER_SIZE, sizeof( char ) );

	/* Obdrzeny generovany hlasovy vystup */
	uint8_t* receivedString = (uint8_t*)calloc( RECEIVED_BUFFER_SIZE, sizeof( uint8_t ) );

	/* Navod k zadavani retezce */
	printf( "\n##################################################################" );
	printf("\n| Pozadavky na zadany retezec:");
	printf("\n|    - Pismena s hackem jsou reprezentovany apostrofem napr. A'");
	printf("\n|    - Pismena s hackem jsou reprezentovana striskou napr. [C^]" );
	printf( "\n##################################################################\n\n" );

	/* Hlavni smycka */
	while( 1 )
		{
		if(WAV_FILE)
			{
			printf( "Zadejte nazev souboru vcetne pripony '.wav' (nesmi byt delsi nez %d): ", FILE_NAME_BUFFER_SIZE);
			if( !scanf( "%s", fileName ) )
				{
				error = ErrorHandle( -100, "Zadani souboru se nezdarilo!" );
				break;
				}
			if( !EndsWith( fileName, ".wav" ) )
				{
				error = ErrorHandle( -101, "Zadany nazev souboru nekonci priponou '.wav'" );
				break;
				}
			}
				
		while( !getchar() );

		printf( "Zadejte retezec, ktery ma byt zaslan:\n\n   " );
		fgets( stringToSend, INPUT_BUFFER_SIZE, stdin );
		
		/* Prevod '\n' na '\0' */
		stringToSend_Ptr = 0;
		while( stringToSend[stringToSend_Ptr++] != '\n' && stringToSend_Ptr < INPUT_BUFFER_SIZE );
		stringToSend[stringToSend_Ptr-1] = '\0';

		/* ODESILANI RETEZCE */

		/* Nastaveni priznaku eventu, ktery nastane, jestlize byl vstupni retezec odeslan */
		if( !SetCommMask( mainCom, EV_TXEMPTY ) ) 
			{
			error = ErrorHandle( -96, "Nastaveni priznaku odeslani vstupniho retezce selahlo!" );
			break;
			}
		/* Odeslani vstupniho retezce po seriove komunikaci */
		if( !WriteFile( mainCom, stringToSend, strlen( stringToSend ) + 1, &bytesWritten, NULL ) )
			{
			error = ErrorHandle( -95, "Odesilani vstupniho retezce selhalo!" );
			break;
			}
		/* Cekani na dokonceni prenosu */
		if( !WaitCommEvent( mainCom, &eventMask, NULL ) )
			{
			error = ErrorHandle( -94, "Odesilani nebylo dokonceno!" );
			break;
			}
		/* Nastaveni priznaku eventu, ktery nastane, jestlize byl prijat znak a ten byl presunut do zasobniku */
		if( !SetCommMask( mainCom, EV_RXCHAR ) ) 
			{
			error = ErrorHandle( -93, "Nastaveni priznaku obdrzeni retezce selhalo!" );
			break;
			}
		/* Cekani na generovany hlasovy vystup */
		if( WaitCommEvent( mainCom, &eventMask, NULL ) )
			{
			/* Zapis generovaneho hlasoveho vystupu do promenne 'receivedString' */
			if( !ReadFile( mainCom, receivedString, RECEIVED_BUFFER_SIZE, &bytesRead, NULL ) )
				{
				error = ErrorHandle( -92, "Obdrzeni dat selhalo!" );
				break;
				}
			}

		/* Vypis kolik Bytu bylo zapsano */
		printf( "\n||==============================\n" );
		printf( "||   Odeslanych Bytu:   %d\n", bytesWritten );
		/* Vypis kolik Bytu bylo prijato*/
		printf( "||   Precteno Bytu:     %d\n", bytesRead );
		printf( "||==============================\n" );
		
		if(WAV_FILE)
			{
			/* Volani funkce pro vytvoreni '.wav' souboru */
			MakeWav( receivedString, bytesRead, fileName );
			}
		printf( "\nChcete pokracovat? [ANO/NE]: " );
		if( !scanf( "%s", &doNext ) )
			{
			error = ErrorHandle( -101, "Rozhodnuti o pokracovani programu selhalo!" );
			break;
			}
		if( strstr( doNext, "NE\0" ) )
			break;

		printf( "\n########################## NOVY CYKLUS ###########################\n\n" );

		/* Vynulovani pameti */
		memset( stringToSend, NULL, INPUT_BUFFER_SIZE );
		memset( fileName, NULL, FILE_NAME_BUFFER_SIZE );
		memset( receivedString, NULL, RECEIVED_BUFFER_SIZE );
		}

	/* Dealokace a ukonceni programu */
	free( stringToSend );
	free( fileName );
	free( receivedString );
	CloseHandle( mainCom );
	return error;
	}
