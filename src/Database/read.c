#include <stdio.h>
#include <math.h>
#include "../config.h"
#include "difficulty.h"

char folder = calloc(256,1);
snprintf(folder, 256, "~/.%s/", CURRENCY_NAME);

// Open globally needed files
/*
snprintf(buf, 256, folder, "txDB");
FILE* txDB = fopen(buf, "r");
snprintf(buf, 256, folder, "headerDB");
FILE* headerDB = fopen(buf, "r");
snprintf(buf, 256, folder, "peerList");
FILE* peerList = fopen(buf, "r");
snprintf(buf, 256, folder, "accountDB");
FILE* accountDB = fopen(buf, "r");
free(buf);
*/
uint64_t getDifficulty(uint32_t height){
	char* fileName = malloc(256,1);
	snprintf(fileName, 256, folder, "diffDB");
	FILE* diffDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "headerDB");
	FILE* headerDB = fopen(fileName, "r");
	free(fileName);

	uint32_t headerCnt = 0;
	uint32_t diffCnt = 0;
	uint32_t timestamps[256] = 0;
	uint64_t difficulties[256] = 0;
	uint32_t lastDiff = 0;
	char headerBuf[BLOCKHEADER_SIZE];
	char diffBuf[BLOCKHEADER_SIZE];
	while(fgets(headerBuf, BLOCKHEADER_SIZE, headerDB)) headerCnt++;
	while(fgets(diffBuf, 8, diffDB)) diffCnt++;
	if(headerCnt < 256 && diffCnt < 256) return 1;
	fseek(diffDB, (diffCnt-256)*8, SEEK_SET);	
	fseek(headerDB, (headerCnt-256)*BLOCKHEADER_SIZE, SEEK_SET);	
	for(uint16_t j=0;j<256;j++){
		fgets(diffBuf, 8, diffDB)
		fgets(headerBuf, BLOCKHEADER_SIZE, headerDB)
		for(uint8_t i=0;i<4;i++){
			timestamps[j] += headerBuf[i+j*BLOCKHEADER_SIZE] << (8*i);
		}
		for(uint8_t i=0;i<8;i++){
			difficulties[j] += diffBuf[i+j<<2] << (8*i);
		}
	}
	return(diff(difficulties, timestamps, -1));
}

uint8_t* getTx(uint64_t txNumber){
	char* tx = malloc(20,1);
	snprintf(fileName, 256, folder, "txDB");
	FILE* txDB = fopen(fileName, "r");
	free(fileName);

	uint64_t txCnt = 0;
	while(fgets(tx, 20, txDB)) txCnt++;
	if(txCnt < txNumber) return 0;
	fseek(txDB, txNumber*20, SEEK_SET);	
	fgets(tx, 20, txDB)
	return(tx);
}



