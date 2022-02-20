#pragma once
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Checks.h"
#include "Book.h"

#define STORES_IND "master.ind"
#define STORES_DATA "master.fl"
#define STORES_GARBAGE "master_garbage.txt"
#define INDEXER_SIZE sizeof(struct Indexer)
#define STORE_SIZE sizeof(struct Store)

#define BOOKS_DATA "games.fl"
#define BOOKS_GARBAGE "games_garbage.txt"
#define BOOK_SIZE sizeof(struct Book)

void noteDeletedStore(int id)
{
	FILE* garbageZone = fopen(STORES_GARBAGE, "rb");		

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	int* delIds = malloc(garbageCount * sizeof(int));		

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				
	}

	fclose(garbageZone);									
	garbageZone = fopen(STORES_GARBAGE, "wb");				
	fprintf(garbageZone, "%d", garbageCount + 1);			

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				
	}

	fprintf(garbageZone, " %d", id);						
	free(delIds);											
	fclose(garbageZone);									
}

void overwriteGarbageId(int garbageCount, FILE* garbageZone, struct Store* record)
{
	int* delIds = malloc(garbageCount * sizeof(int));		

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageZone, "%d", delIds + i);				
	}

	record->id = delIds[0];									

	fclose(garbageZone);									
	fopen(STORES_GARBAGE, "wb");							
	fprintf(garbageZone, "%d", garbageCount - 1);			

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageZone, " %d", delIds[i]);				
	}

	free(delIds);											
	fclose(garbageZone);									
}

int insertStore(struct Store record)
{
	FILE* indexTable = fopen(STORES_IND, "a+b");			
	FILE* database = fopen(STORES_DATA, "a+b");				
	FILE* garbageZone = fopen(STORES_GARBAGE, "rb");		
	struct Indexer indexer;
	int garbageCount;

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount)										
	{
		overwriteGarbageId(garbageCount, garbageZone, &record);

		fclose(indexTable);									
		fclose(database);									

		indexTable = fopen(STORES_IND, "r+b");				
		database = fopen(STORES_DATA, "r+b");				

		fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
		fread(&indexer, INDEXER_SIZE, 1, indexTable);
		fseek(database, indexer.address, SEEK_SET);			
		}
	else                                                    
	{
		long indexerSize = INDEXER_SIZE;

		fseek(indexTable, 0, SEEK_END);						

		if (ftell(indexTable))								
		{
			fseek(indexTable, -indexerSize, SEEK_END);		
			fread(&indexer, INDEXER_SIZE, 1, indexTable);	

			record.id = indexer.id + 1;						
		}
		else                                                
		{
			record.id = 1;									
		}
	}
	record.firstBookIdx = -1;
	record.bookCount = 0;

	fwrite(&record, STORE_SIZE, 1, database);				

	indexer.id = record.id;								
	indexer.address = (record.id - 1) * STORE_SIZE;		
	indexer.exists = 1;									

	printf("Your store\'s id is %d\n", record.id);

	fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET); 
	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			
	fclose(indexTable);										
	fclose(database);

	return 1;
}

int getStore(struct Store* master, int id, char* error)
{
	FILE* indexTable = fopen(STORES_IND, "rb");				
	FILE* database = fopen(STORES_DATA, "rb");				
	
	if (!checkFileExistence(indexTable, database, error))
	{
		return 0;
	}
	
	struct Indexer indexer;

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			

	if (!checkRecordExistence(indexer, error))
	{
		return 0;
	}

	fseek(database, indexer.address, SEEK_SET);				
	fread(master, sizeof(struct Store), 1, database);		
	fclose(indexTable);										
	fclose(database);

	return 1;
}

int updateStore(struct Store store, char* error)
{
	FILE* indexTable = fopen(STORES_IND, "r+b");			
	FILE* database = fopen(STORES_DATA, "r+b");				

	if (!checkFileExistence(indexTable, database, error))
	{	
		return 0;
	}

	struct Indexer indexer;
	int id = store.id;

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			

	if (!checkRecordExistence(indexer, error))
	{
		return 0;
	}

	fseek(database, indexer.address, SEEK_SET);				
	fwrite(&store, STORE_SIZE, 1, database);				
	fclose(indexTable);										
	fclose(database);

	return 1;
}

int deleteStore(int id, char* error)
{
	FILE* indexTable = fopen(STORES_IND, "r+b");			
															
	if (indexTable == NULL)
	{
		strcpy(error, "database files are not created yet");
		return 0;
	}

	if (!checkIndexExistence(indexTable, error, id))
	{
		return 0;
	}

	struct Store store;
	getStore(&store, id, error);

	struct Indexer indexer;

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			

	indexer.exists = 0;										

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);

	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			
	fclose(indexTable);										

	noteDeletedStore(id);									

	
	if (store.bookCount)								
	{
		FILE* gameDB = fopen(BOOKS_DATA, "r+b");
		struct Book book;

		fseek(gameDB, store.firstBookIdx, SEEK_SET);

		for (int i = 0; i < store.bookCount; i++)
		{
			fread(&book, BOOK_SIZE, 1, gameDB);
			fclose(gameDB);
			deleteSlave(store, book, book.genreId, error);
			
			gameDB = fopen(BOOKS_DATA, "r+b");
			fseek(gameDB, book.nextIdx, SEEK_SET);
		}

		fclose(gameDB);
	}
	return 1;
}