#include <stdio.h>
#include "../config.h"

snprintf(folder, 256, "~/.%s/", CURRENCY_NAME);

// Open globally needed files
snprintf(buf, 256, folder, "txDB");
FILE* txDB = fopen(buf, "r");
snprintf(buf, 256, folder, "headerDB");
FILE* headerDB = fopen(buf, "r");
snprintf(buf, 256, folder, "peerList");
FILE* peerList = fopen(buf, "r");
snprintf(buf, 256, folder, "accountDB");
FILE* accountDB = fopen(buf, "r");
free(buf);

uint64_t getDifficulty(uint32_t height){
	char folder = calloc(256,1);
	char buf = malloc(256,1);
	snprintf(buf, 256, folder, "diffDB");
	FILE* diffDB = fopen(buf, "r");
	snprintf(buf, 256, folder, "headerDB");
	FILE* headerDB = fopen(buf, "r");

}



