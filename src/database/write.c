#include <stdio.h>
#include <stdint.h>
#include "read.h"

uint8_t prepareFundchangeForUsername(uint8_t* username, uint64_t funds){
	char*     folder   = getFolder();
	char*     fileName = (char*)malloc(256);
	uint64_t* user     = (uint64_t*)malloc(5);
	uint64_t* funds    = (uint64_t*)malloc(8);
	uint8_t*  funds_8  =  (uint8_t*)&funds;
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "wealthChangeDB");
	FILE* wealthDB = fopen(fileName, "w");
	free(fileName);
	free(folder);
	while(fgets(user, 5, userDB)){
		if(user[0] != username[0]
		|| user[1] != username[1]
		|| user[2] != username[2]
		|| user[3] != username[3]
		|| user[4] != username[4]){
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

uint8_t changeFundsForUsername(uint8_t* username, int64_t change){
	uint8_t out = prepareFundchangeForUsername(username, change + getFundchangeForUsername(username));
	if(!out) printf("[Error] Unable to change funds, username not found\n");
	return out; 
}

void setChangeToNull(){
	snprintf(fileName, 256, folder, "wealthChangeDB");
	FILE* wealthDB = fopen(fileName, "w");
	while(fprintf(wealthDB, "00000000", 8));
	fclose(wealthDB);
	return;
}

uint8_t addChangeToFunds(){
	char*     folder   = getFolder();
	char*     fileName = (char*)malloc(256);
	uint64_t* change   = (uint64_t*)malloc(8);
	uint64_t* funds    = (uint64_t*)malloc(8);
	snprintf(fileName, 256, folder, "wealthDB");
	FILE* wealthDB = fopen(fileName, "r+");
	snprintf(fileName, 256, folder, "wealthChangeDB");
	FILE* wealthChangeDB = fopen(fileName, "r");
	while(fgets(change, 8, wealthChangeDB)){
		fgets(funds, 8, wealthDB)

		// Check if funds would be negative
		if((*change&0x7fffffffffffffff) > *funds && !((*change)>>63)) 
			if()
				return 0;

		// Check funds would be out greater than the max
		if(((*change)>>63) && ((*change&0x7fffffffffffffff) + *funds)>>63)
			return 0;
		
	}
	fseek(wealthDB, 0, SEEK_SET);
	fseek(wealthChangeDB, 0, SEEK_SET);
	while(fgets(change, 8, wealthChangeDB)){
		fgets(funds, 8, wealthDB)
		if(((*change)>>63)) *funds += change;
		else *funds -= change;
		fseek(wealthDB, -8, SEEK_CUR)
		fprintf(wealthDB, "00000000", 8)
	}
	free(fileName);
	free(folder);
	fclose(wealthDB);
	fclose(wealthChangeDB);
	return 1;
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
	while(fgets(user, 5, userDB)){
		fseek(wealthDB,8,SEEK_CUR);
		fseek(pubKeyDB,8,SEEK_CUR);
	}
	fprintf(wealthDB, "00000000", 8);
	fprintf(pubKeyDB, pubKey, 48);
	fprintf(userDB, username, 5);
	fclose(userDB);
	fclose(wealthDB);
	fclose(pubKeyDB);
	return 1;
}

void addHeader(uint8_t* header){
	char* folder = getFolder();
	char* fileName = (char*)malloc(256);
	char* header = (char*)malloc(188);
	char* hash   = (char*)malloc(64);
	snprintf(fileName, 256, folder, "headerDB");
	FILE* headerDB = fopen(fileName, "a");
	free(fileName);
	free(folder);
	fprintf(headerDB, header, 188);
	fclose(headerDB);
}
