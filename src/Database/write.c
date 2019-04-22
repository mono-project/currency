#include <stdio.h>
#include <stdint.h>
#include "read.h"

uint8_t prepareFundchangeForUsername(uint8_t* username, uint64_t funds){
	uint8_t   good     = 1;
	char*     folder   = getFolder();
	char*     fileName = (char*)malloc(256);
	uint64_t* user     = (uint64_t*)malloc(8);
	uint64_t* funds    = (uint64_t*)malloc(8);
	uint8_t*  funds_8  =  (uint8_t*)&funds;
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "wealthChangeDB");
	FILE* wealthDB = fopen(fileName, "w");
	free(fileName);
	free(folder);
	while(fgets(user, 8, userDB)){
		if(user[0] != username[0]
		|| user[1] != username[1]
		|| user[2] != username[2]
		|| user[3] != username[3]
		|| user[4] != username[4]
		|| user[5] != username[5]
		|| user[6] != username[6]
		|| user[7] != username[7]){
			fseek(wealthDB,8,SEEK_CUR);
			continue;
		}
		fprintf(wealthDB, funds_8, 8);
		free(user);
		free(funds);
		fclose(userDB);
		fclose(wealthDB);
		return 1;
	}
	free(user);
	free(funds);
	fclose(userDB);
	fclose(wealthDB);
	return 0;
}

uint8_t changeFundsForUsername(uint8_t* username, uint64_t change, uint8_t mode){
	// Mode 1 = Add (+)
	// Mode 0 = Sub (-)
	uint64_t prevChange = getFundchangeForUsername(username);
	uint64_t curChange = change&0x7fffffffffffffff;
	uint8_t curMode = (prevChange&0x8000000000000000)>>63;
	uint8_t out = 0;
	if(curMode){
		prevChange += curChange;
	}else{
		if(curChange > prevChange){
			prevChange = curChange - prevChange;
			curMode ^= 1;
		}else{
			prevChange -= curChange;
		}
	}	
	if(curChange&0x8000000000000000){
		printf("[Error] Unable to change funds, username not found\n");
		return 0;
	}
	curChange = (curMode<<63) | curChange;
	out = prepareFundchangeForUsername(username, curChange);
	if(!out) printf("[Error] Unable to change funds, username not found\n");
	return out;
}

uint8_t addUsername(uint8_t* username, uint8_t* pubKey){
	char*     folder   = getFolder();
	char*     fileName = (char*)malloc(256);
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "w+");
	snprintf(fileName, 256, folder, "wealthDB");
	FILE* wealthDB = fopen(fileName, "w");
	snprintf(fileName, 256, folder, "pubKeyDB");
	FILE* pubKeyDB = fopen(fileName, "w");
	if(!userDB || !wealthDB || !pubKeyDB) return 0;
	free(fileName);
	free(folder);
	while(fgets(user, 8, userDB)){
		fseek(wealthDB,8,SEEK_CUR);
		fseek(pubKeyDB,8,SEEK_CUR);
	}
	fprintf(userDB, username, 8);
	fprintf(wealthDB, "00000000", 8);
	fprintf(pubKeyDB, pubKey, 48);
	fclose(userDB);
	fclose(wealthDB);
	fclose(pubKeyDB);
	return 1;
}
