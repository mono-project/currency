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

uint8_t* getBlockTx(uint32_t height){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	snprintf(fileName, 256, folder, "txDB");
	FILE* txDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "txSpaceDB");
	FILE* txSpaceDB = fopen(fileName, "r");
	free(fileName);
	free(folder);
	uint64_t txCnt = 0;
	uint64_t realHeight[2] = 0;
	char* txSpace = (char*)malloc(8);
	while(fgets(txSpace, 8, txSpaceDB)) realHeight++;
	if(realHeight < height) return 0;
	fseek(txSpaceDB, (height-1)*8, SEEK_SET);
	fgets(txSpace, 8, txSpaceDB);
	for(uint8_t i=0;i<8;i++) realHeight[0] += txSpace[i] << (8*i);
	fseek(txSpaceDB, (height-1)*8, SEEK_SET);
	fgets(txSpace, 8, txSpaceDB);
	for(uint8_t i=0;i<8;i++) realHeight[1] += txSpace[i] << (8*i);
	uint64_t txMem = realHeight[1]-realHeight[0];
	char* txs = (char*)malloc(1);
	uint64_t i=0;
	uint8_t currentSize = 0;
	while(i<=txMem){
		fgets(txs+i, 1, txDB);
		currentSize = txs[i]&0x7f;
		txs = (char*)realloc(txs, i+currentSize);
		fgets(txs+i, currentSize, txDB);
		i += currentSize;
	}
	free(txSpace);
	fclose(txDB);
	fclose(txSpaceDB);
	return(txs);
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
	uint64_t* funds = (uint64_t*)malloc(8);
	uint64_t  tmp = 0;
	uint64_t  out = 0;
	snprintf(fileName, 256, folder, "wealthChangeDB");
	FILE* wealthChangeDB = fopen(fileName, "r");
	free(fileName);
	free(folder);
	while(fgets(funds, 8, wealthChangeDB)){
		tmp = *funds;
		if(tmp>>63) out += tmp&0x7fffffffffffffff;
		else out -= tmp&0x7fffffffffffffff;
	}
	free(funds);
	fclose(wealthChangeDB);
	return out;
}

uint64_t* getPubKeys(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	uint64_t* pubKey = (uint64_t*)malloc(48);
	snprintf(fileName, 256, folder, "pubKeyDB");
	FILE* pubKeyDB = fopen(fileName, "r");
	free(fileName);
	free(folder);

	uint32_t keyCount = 0;
	while(fgets(pubKey, 48, pubKeyDB)) keyCount++;
	fseek(pubKeyDB, 0, SEEK_SET)
	uint64_t* pubKeys = (uint64_t*)malloc(keyCount*48);
	for(uint32_t i=0;i<keyCount;i++){
		fgets(pubKey, 5, pubKeyDB)
		for(uint8_t j=0;j<5;j++){
			pubKeys[i] += pubKey[j] << (5*j);
		}
	}
	free(pubKey);
	fclose(pubKeyDB);
	return(pubKeys);
}

uint8_t* getLastBlockHash(){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	char* header = (char*)malloc(188);
	char* hash   = (char*)malloc(64);
	uint64_t cnt = 0;
	snprintf(fileName, 256, folder, "headerDB");
	FILE* headerDB = fopen(fileName, "r");
	free(fileName);
	free(folder);
	while(fgets(header, 188, headerDB)) cnt++;
	if(!cnt) return 0;
	fseek(headerDB, (cnt-1)*188, SEEK_SET);
	fgets(header, 188, headerDB);
	blakenl(header, 188, hash);
	free(header);
	fclose(headerDB);
	return(hash);
}

