#pragma once

/*
"Structures.h": master (supplier), product (detail), slave (supplement), indexer.
*/

struct Store
{
	int id;
	char name[16];
	long firstBookIdx;
	int bookCount;
};

struct Book
{
	int storeId;
	char bookName[32];
	int genreId;
	int price;
	int exists;
	long selfIdx;
	long nextIdx;
};

struct Indexer
{
	int id;	
	int address;
	int exists;
};