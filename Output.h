#pragma once

#include <stdio.h>
#include "Store.h"
#include "Structures.h"

void printStore(struct Store store)
{
	printf("Store\'s name: %s\n", store.name);
	printf("Store\'s books count: %d\n", store.bookCount);
}

void printBook(struct Book book, struct Store store)
{
	printf("Store: %s\n", store.name);
	printf("Name : %s\n", book.bookName);
	printf("Price: %d\n", book.price);
}