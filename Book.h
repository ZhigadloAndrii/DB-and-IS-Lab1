#pragma once
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include "Structures.h"
#include "Checks.h"
#include "Store.h"

#define BOOKS_DATA "games.fl"
#define BOOKS_GARBAGE "games_garbage.txt"
#define BOOK_SIZE sizeof(struct Book)


void reopenDatabase(FILE* database)
{
	fclose(database);
	database = fopen(BOOKS_DATA, "r+b");
}

void linkAddresses(FILE* database, struct Store store, struct Book book)
{
	reopenDatabase(database);								

	struct Book previous;

	fseek(database, store.firstBookIdx, SEEK_SET);

	for (int i = 0; i < store.bookCount; i++)		    
	{
		fread(&previous, BOOK_SIZE, 1, database);
		fseek(database, previous.nextIdx, SEEK_SET);
	}

	previous.nextIdx = book.selfIdx;				
	fwrite(&previous, BOOK_SIZE, 1, database);		
}

void relinkAddresses(FILE* database, struct Book previous, struct Book book, struct Store* store)
{
	if (book.selfIdx == store->firstBookIdx)		
	{
		if (book.selfIdx == book.nextIdx)			
		{
			store->firstBookIdx = -1;					
		}
		else                                            
		{
			store->firstBookIdx = book.nextIdx;  
		}
	}
	else                                         
	{
		if (book.selfIdx == book.nextIdx)		
		{
			previous.nextIdx = previous.selfIdx; 
		}
		else                                     
		{
			previous.nextIdx = book.nextIdx;	
		}

		fseek(database, previous.selfIdx, SEEK_SET);
		fwrite(&previous, BOOK_SIZE, 1, database);	
	}
}

void noteDeletedBook(long address)
{
	FILE* garbageZone = fopen(BOOKS_GARBAGE, "rb");		

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	long* delAddresses = malloc(garbageCount * sizeof(long)); 

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%ld", delAddresses + i);
	}

	fclose(garbageZone);							
	garbageZone = fopen(BOOKS_GARBAGE, "wb");				
	fprintf(garbageZone, "%ld", garbageCount + 1);			

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %ld", delAddresses[i]);		
	}

	fprintf(garbageZone, " %d", address);					
	free(delAddresses);										
	fclose(garbageZone);									
}

void overwriteGarbageAddress(int garbageCount, FILE* garbageZone, struct Book* record)
{
	long* delIds = malloc(garbageCount * sizeof(long));		

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				
	}

	record->selfIdx = delIds[0];						
	record->nextIdx = delIds[0];

	fclose(garbageZone);									
	fopen(BOOKS_GARBAGE, "wb");							    
	fprintf(garbageZone, "%d", garbageCount - 1);			

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				
	}

	free(delIds);											
	fclose(garbageZone);									
}

int insertBook(struct Store store, struct Book book, char* error)
{
	book.exists = 1;

	FILE* database = fopen(BOOKS_DATA, "a+b");
	FILE* garbageZone = fopen(BOOKS_GARBAGE, "rb");

	int garbageCount;											

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount)											
	{
		overwriteGarbageAddress(garbageCount, garbageZone, &book);
		reopenDatabase(database);								
		fseek(database, book.selfIdx, SEEK_SET);			
	}
	else                                                    
	{
		fseek(database, 0, SEEK_END);

		int dbSize = ftell(database);

		book.selfIdx = dbSize;
		book.nextIdx = dbSize;
	}

	fwrite(&book, BOOK_SIZE, 1, database);					

	if (!store.bookCount)								    
	{
		store.firstBookIdx = book.selfIdx;
	}
	else                                                       
	{
		linkAddresses(database, store, book);
	}

	fclose(database);										

	store.bookCount++;										
	updateStore(store, error);								

	return 1;
}

int getSlave(struct Store store, struct Book* book, int productId, char* error)
{
	if (!store.bookCount)									
	{
		strcpy(error, "This store has no books yet");
		return 0;
	}

	FILE* database = fopen(BOOKS_DATA, "rb");


	fseek(database, store.firstBookIdx, SEEK_SET);		
	fread(book, BOOK_SIZE, 1, database);

	for (int i = 0; i < store.bookCount; i++)			
	{
		if (book->genreId == productId)					
		{
			fclose(database);
			return 1;
		}

		fseek(database, book->nextIdx, SEEK_SET);
		fread(book, BOOK_SIZE, 1, database);
	}
	
	strcpy(error, "No such book in database");			
	fclose(database);
	return 0;
}

int updateSlave(struct Book book, int productId)
{
	FILE* database = fopen(BOOKS_DATA, "r+b");

	fseek(database, book.selfIdx, SEEK_SET);
	fwrite(&book, BOOK_SIZE, 1, database);
	fclose(database);
	
	return 1;
}

int deleteSlave(struct Store store, struct Book book, int productId, char* error)
{
	FILE* database = fopen(BOOKS_DATA, "r+b");
	struct Book previous;

	fseek(database, store.firstBookIdx, SEEK_SET);

	do		                                                    
	{															
		fread(&previous, BOOK_SIZE, 1, database);
		fseek(database, previous.nextIdx, SEEK_SET);
	}
	while (previous.nextIdx != book.selfIdx && book.selfIdx != store.firstBookIdx);

	relinkAddresses(database, previous, book, &store);
	noteDeletedBook(book.selfIdx);						

	book.exists = 0;											

	fseek(database, book.selfIdx, SEEK_SET);				
	fwrite(&book, BOOK_SIZE, 1, database);					
	fclose(database);

	store.bookCount--;										
	updateStore(store, error);

}