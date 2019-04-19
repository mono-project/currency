#include <stdio.h>
#include <math.h>
#include "../config.h"
#indlude "read.h"

uint8_t setFundsForUsername(uint8_t* username, uint64_t funds){
	uint8_t   good     = 1;
	char*     folder   = getFolder();
	char*     fileName = (char*)malloc(256);
	uint64_t* user     = (uint64_t*)malloc(8);
	uint64_t* funds    = (uint64_t*)malloc(8);
	uint8_t*  funds_8  =  (uint8_t*)&funds;
	snprintf(fileName, 256, folder, "userDB");
	FILE* userDB = fopen(fileName, "r");
	snprintf(fileName, 256, folder, "wealthDB");
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
		fclose(userDB);
		fclose(wealthDB);
		return 1;
	}
	fclose(userDB);
	fclose(wealthDB);
	return 0;
}

uint8_t changeFundsForUsername(uint8_t* username, uint64_t change, uint8_t mode){
	// Mode 1 = Add (+)
	// Mode 0 = Sub (-)

	uint64_t prevFunds = getFundsForUsername(username);
	uint8_t out = 0;
	if(mode){
		out = setFundsForUsername(username, prevFunds+change);
	} else {
		if(prevFunds-change > prevFunds){
			printf("[Error] more funds spent than available\n");
			return 0;
		}
		out = setFundsForUsername(username, prevFunds-change);
	}
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
