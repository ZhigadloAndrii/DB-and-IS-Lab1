#pragma once
#include <stdio.h>
#include <string.h>
#include "Structures.h"
#include "Store.h"

void readStore(struct Store* store)
{
	char name[16];
	int status;

	name[0] = '\0';

	printf("Enter store\'s name: ");
	scanf("%s", name);

	strcpy(store->name, name);
}

void readBooks(struct Book* book)
{
	int x;
	int price;

	char name[32];
	name[0] = '\0';

	printf("Enter book name: ");
	scanf("%s", name);
	strcpy(book->bookName , name);

	printf("Enter price: ");
	scanf("%d", &price);
	book->price = price;
}