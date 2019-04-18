#include <stdio.h>
#include <math.h>
#include "../config.h"
#include "difficulty.h"

char* getFolder(){
	char* folder = calloc(256,1);
	snprintf(folder, 256, "~/.%s/", CURRENCY_NAME);
	return(folder);
}

uint64_t calcDifficultyForHeight(uint32_t height){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
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

uint64_t getDifficultyForTimestamp(uint32_t prevTimestamp, uint32_t height){
// Needed for TSA Difficulty (https://github.com/zawy12/difficulty-algorithms/issues/36)
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
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
	fseek(headerDB, (headerCnt-255)*BLOCKHEADER_SIZE, SEEK_SET);	
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
	timestamps[255] = prevTimestamp;
	return(diff(difficulties, timestamps, -1));
}

uint8_t* getTx(uint64_t txNumber){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	char* tx = (char*)malloc(20);
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

uint8_t* getBlockTx(uint32_t height){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	snprintf(fileName, 256, folder, "txDB");
	FILE* txDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "txNumDB");
	FILE* txNumDB = fopen(fileName, "r");
	free(fileName);

	uint64_t txCnt = 0;
	uint64_t realHeight[2] = 0;
	char* txNumber = (char*)malloc(8);
	while(fgets(txNumber, 8, txNumDB)) realHeight++;
	if(realHeight < height) return 0;
	fseek(txNumDB, (height-1)*8, SEEK_SET);
	fgets(txNumber, 8, txNumDB);
	for(uint8_t i=0;i<8;i++) realHeight[0] += txNumber[i] << (8*i);
	fseek(txNumDB, (height-1)*8, SEEK_SET);
	fgets(txNumber, 8, txNumDB);
	for(uint8_t i=0;i<8;i++) realHeight[1] += txNumber[i] << (8*i);
	uint64_t txCount = realHeight[1]-realHeight[0];
	char* txs = (char*)malloc(20*(txCount));
	for(uint64_t i=0;i<=txCount;i++) fgets(txs+20*i, 20, txDB);
	return(txs);
}

uint32_t* getPeerList(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint64_t* ip = (uint64_t*)malloc(8); //255.255.255.255:65536 - B.B.B.B:BB -> 6 Byte
	snprintf(fileName, 256, folder, "peerList");
	FILE* peerList = fopen(fileName, "r");
	free(fileName);

	uint32_t peerCount = 0;
	while(fgets(ip, 8, peerList)) peerCount++;
	fseek(txNumDB, 0, SEEK_SET)
	uint64_t* ips = (uint64_t*)malloc(peerCount*8);
	for(uint32_t i=0;i<peerCount;i++){
		fgets(ip, 8, peerList)
		for(uint8_t j=0;j<8;j++){
			ips[i] += ip[j] << (8*j);
		}
	}
	return(ips);
}

uint32_t* getUsernames(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint64_t* user = (uint64_t*)malloc(8); //255.255.255.255:65536 - B.B.B.B:BB -> 6 Byte
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "r");
	free(fileName);

	uint32_t userCount = 0;
	while(fgets(user, 8, userDB)) userCount++;
	fseek(userDB, 0, SEEK_SET)
	uint64_t* users = (uint64_t*)malloc(userCount*8);
	for(uint32_t i=0;i<userCount;i++){
		fgets(user, 8, userDB)
		for(uint8_t j=0;j<8;j++){
			users[i] += user[j] << (8*j);
		}
	}
	return(users);
}

uint32_t* getPubKeys(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint64_t* pubKey = (uint64_t*)malloc(48); //255.255.255.255:65536 - B.B.B.B:BB -> 6 Byte
	snprintf(fileName, 256, folder, "pubKeyDB");
	FILE* pubKeyDB = fopen(fileName, "r");
	free(fileName);

	uint32_t keyCount = 0;
	while(fgets(pubKey, 48, pubKeyDB)) keyCount++;
	fseek(pubKeyDB, 0, SEEK_SET)
	uint64_t* pubKeys = (uint64_t*)malloc(keyCount*48);
	for(uint32_t i=0;i<keyCount;i++){
		fgets(pubKey, 8, pubKeyDB)
		for(uint8_t j=0;j<8;j++){
			pubKeys[i] += pubKey[j] << (8*j);
		}
	}
	return(pubKeys);
}

