#include <stdio.h>
#include <stdint.h>
#include "../config.h"
#include "../Crypto/blake2.h"

char* getFolder(){
	char* folder = calloc(256,1);
	snprintf(folder, 256, "~/.%s/", CURRENCY_NAME);
	return(folder);
}

uint8_t* getTx(uint64_t txNumber){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	char* tx = (char*)malloc(90);
	snprintf(fileName, 256, folder, "txDB");
	FILE* txDB = fopen(fileName, "r");
	free(fileName);
	free(folder);

	uint64_t txCnt = 0;
	while(fgets(tx, 90, txDB)) txCnt++;
	if(txCnt < txNumber) return 0;
	fseek(txDB, txNumber*90, SEEK_SET);	
	fgets(tx, 90, txDB)
	fclose(txDB);
	return(tx);
}

uint32_t* getPeerList(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint64_t* ip = (uint64_t*)malloc(8); //255.255.255.255:65536 - B.B.B.B:BB -> 6 Byte
	snprintf(fileName, 256, folder, "peerList");
	FILE* peerList = fopen(fileName, "r");
	free(fileName);
	free(folder);

	uint32_t peerCount = 0;
	while(fgets(ip, 8, peerList)) peerCount++;
	fseek(peerList, 0, SEEK_SET)
	uint64_t* ips = (uint64_t*)malloc(peerCount*8);
	for(uint32_t i=0;i<peerCount;i++){
		fgets(ip, 8, peerList)
		for(uint8_t j=0;j<8;j++){
			ips[i] += ip[j] << (8*j);
		}
	}
	free(ip);
	fclose(peerList);
	return(ips);
}

uint32_t* getUsernames(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint64_t* user = (uint64_t*)malloc(5);
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "r");
	free(fileName);
	free(folder);

	uint32_t userCount = 0;
	while(fgets(user, 5, userDB)) userCount++;
	fseek(userDB, 0, SEEK_SET)
	uint64_t* users = (uint64_t*)malloc(userCount*5);
	for(uint32_t i=0;i<userCount;i++){
		fgets(user, 5, userDB)
		for(uint8_t j=0;j<5;j++){
			users[i] += user[j] << (5*j);
		}
	}
	free(user);
	fclose(userDB);
	return(users);
}

uint64_t getFundsForUsername(uint8_t* username){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint64_t* user = (uint64_t*)malloc(5);
	uint64_t* funds = (uint64_t*)malloc(8);
	uint64_t  out = 0;
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "wealthDB");
	FILE* wealthDB = fopen(fileName, "r");
	free(fileName);
	free(folder);
	while(fgets(user, 5, userDB)){
		fgets(funds, 8, wealthDB);
		if(user[0] != username[0]
		|| user[1] != username[1]
		|| user[2] != username[2]
		|| user[3] != username[3]
		|| user[4] != username[4]) continue;
		fclose(userDB);
		fclose(wealthDB);
		out = *funds;
		free(funds);
		return(out);
	}
	free(user);
	free(funds);
	fclose(userDB);
	fclose(wealthDB);
	return 0;
}
uint64_t getFundchangeForUsername(uint8_t* username){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint64_t* user = (uint64_t*)malloc(5);
	uint64_t* funds = (uint64_t*)malloc(8);
	uint64_t  out = 0;
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "wealthChangeDB");
	FILE* wealthChangeDB = fopen(fileName, "r");
	free(fileName);
	free(folder);
	while(fgets(user, 5, userDB)){
		fgets(funds, 8, wealthChangeDB);
		if(user[0] != username[0]
		|| user[1] != username[1]
		|| user[2] != username[2]
		|| user[3] != username[3]
		|| user[4] != username[4]) continue;
		fclose(userDB);
		fclose(wealthChangeDB);
		out = *funds;
		free(funds);
		return(out);
	}
	free(user);
	free(funds);
	fclose(userDB);
	fclose(wealthChangeDB);
	return 0;
}

uint64_t getTotalFundchange(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	int64_t* funds = (int64_t*)malloc(8);
	int64_t  tmp = 0;
	uint64_t out = 0;
	snprintf(fileName, 256, folder, "wealthChangeDB");
	FILE* wealthChangeDB = fopen(fileName, "r");
	free(fileName);
	free(folder);
	while(fgets(funds, 8, wealthChangeDB)) tmp = *funds;
	free(funds);
	fclose(wealthChangeDB);
	return(tmp==0);
}

uint8_t* getPubKeyForUsername(uint8_t* username){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint8_t* pubKey = (uint8_t*)malloc(32);
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "pubKeyDB");
	FILE* pubKeyDB = fopen(fileName, "r");
	free(fileName);
	free(folder);
	while(fgets(user, 5, userDB)){
		if(!user) break;
		fgets(pubKey, 32, pubKeyDB);
		if(user[0] != username[0]
		|| user[1] != username[1]
		|| user[2] != username[2]
		|| user[3] != username[3]
		|| user[4] != username[4]) continue;
		fclose(userDB);
		fclose(pubKeyDB);
		return pubKey;
	}
	free(pubKey);
	fclose(userDB);
	fclose(pubKeyDB);
	return 0;
}

uint8_t* getLastStateHash(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	char* file   = (char*)malloc(1024);
	char* hash   = (char*)malloc(64);
	snprintf(fileName, 256, folder, "txDB");
	FILE* txDB = fopen(fileName, "r");
	free(fileName);
	free(folder);
	fgets(file, 1024, txDB); blakenl(file, 1024, hash);
	while(fgets(file, 1024, txDB)) blakesl(file, 1024, hash, 64, hash);
	free(file);
	fclose(txDB);
	return(hash);
}

