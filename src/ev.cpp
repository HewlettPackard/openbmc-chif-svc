/*
// Copyright (c) 2021-2025 Hewlett-Packard Enterprise Development, LP
// 
// Hewlett-Packard and the Hewlett-Packard logo are trademarks of
// Hewlett-Packard Development Company, L.P. in the U.S. and/or other countries.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <search.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <fstream>
#include "ev.hpp"
#include "misc.hpp"

#define EV_MAX_LEN		4000
#define EV_NAME_MAX_LEN	32
#define EV_HEAD_LEN		(EV_NAME_MAX_LEN+2)
#define EV_DATA_MAX_LEN	(EV_MAX_LEN - EV_HEAD_LEN)
#define UEFI_EVS_STORE "/usr/share/uefi/uefievs.store"
#define EV_FILE "/home/root/evs.dat"
#define EV_TMP	"/home/root/evs.tmp"

struct ev e;

static struct node* new_node(void)
{
	struct node *n;

	n = (struct node *)malloc(sizeof(struct node));

	if(n == NULL) {
		dbPrintf("ev: new_entry(): malloc failed\n");
	}
	n->forward = NULL;
	n->backward = NULL;
	return n;
}

struct node *head = NULL;

void dumpNVRAM()
{
    // We need to open /dev/mtd
    // search for the nvram entry
    // and open it as an input
    //
    std::string buffer;

    FILE *fpi, *fpo;
    char file[65536];  // this size could be a problem. iLO 2.10 and later is 96KB. 
    char magic[2];
//    char name[32]; // max length is 32
    uint8_t nameSize;

    struct stat stat_buf;

    struct ev localEV;

    int rc = stat(UEFI_EVS_STORE, &stat_buf);

    dbPrintf("NVRAM Dump\n");

    if ( rc != 0 ) {
        dbPrintf("%s not found\n", UEFI_EVS_STORE);
        return;
    }

    // if we have a file which does exist we
    // keep it as it could have been updated by
    // previous boot

    rc = stat(EV_FILE, &stat_buf);

    if ( rc == 0 )
    {
        dbPrintf("%s found\n", EV_FILE);
	    return;
    }
 
    fpi =fopen(UEFI_EVS_STORE, "r");
    dbPrintf("EV: num read: %ld\n", fread( &file[0], 1, 65536 , fpi));
    fclose(fpi);

    fpo = fopen(EV_FILE, "w");


    dbPrintf("EV: Starting compare\n");

    for(int i = 0 ; i < 65535 ; i++ )
    {
        strncpy(&magic[0], &file[i], 2);
//        dbPrintf("EV: cmp string: %c%c \n", magic[0], magic[1]);
        if ( strncmp(&magic[0], "CQ", 2) == 0 ) {
            dbPrintf("EV: have match\n\n");
            nameSize = (uint8_t)(file[i-1]);
            for ( int j = 0 ; j < 32 ; j++ )
                localEV.name[j] = '\0';

            strncpy(&localEV.name[0], &file[i], nameSize);

			// The actual datasize is 16 bits of data
			memcpy(&localEV.size, &file[i] + nameSize, 2);
			dbPrintf("EV: Name: %s Size: %x \n", &localEV.name[0], localEV.size);

			// We have to write a localEV structure and
			fwrite(&localEV, sizeof(struct ev), 1, fpo);

			// the data to the output file
			fwrite(&file[i] + nameSize + 2, sizeof(char), localEV.size, fpo);
                i += nameSize;
        }
    }
    fclose(fpo);
    dbPrintf("EV: Exiting NVRAM dump");
}

void clearEVName(char *name)
{
    if (!name) {
        return;
    }
    for (int i = 0; i < 32; i++)
		name[i] = '\0';
}

void printEVs()
{
	struct node *n = head;
	int size = 0;
	while(n != NULL)
	{
		dbPrintf("EV: node: %s, offset: %lu\n", n->name, n->offset);
		n = n->forward;
		size++;
	}
}

int initEV(void)
{
	struct ev e;
	struct node *n;
	struct node *prev_node = NULL;
	FILE *fp;
	long offset = 0;
	size_t rc;
	int i = 0;

//    dumpNVRAM();  // stop using the default uefievs.store file
	fp = fopen(EV_FILE, "a+");

	while(1) {
		rc = fread(&e, sizeof(struct ev), 1, fp);
		if(rc<1) {
			break;
		}
		n = new_node();
        if (!n) {
            break;
        }
        if (head == NULL) {
			dbPrintf("EV: Saving memory address of first node.\n");
			head = n; //saves memory address of first node
		}
		if(prev_node !=NULL)
			dbPrintf("EV: Prev Node: %s\n", prev_node->name);
		else
			dbPrintf("EV: Prev Node is NULL\n");
		insque(n, prev_node);

		clearEVName(n->name);
		strncpy(n->name, e.name, 32);
		dbPrintf("EV: found instance: %s\n", n->name);
		n->offset = offset;
		dbPrintf("EV: offset: %lu\n", n->offset);
		prev_node = n;
		offset += ( sizeof(struct ev) );
		offset += e.size;

		fseek(fp, e.size, SEEK_CUR);
		i++;
	}
	fclose(fp);
	printEVs();
	return 0;
}

int clearEV(void)
{
	FILE *fp;

    head = NULL;
	fp = fopen(EV_FILE, "w");
	fsync(fileno(fp));
	fclose(fp);
	initEV();
	return 0;
}

static long getOffsetByIndex(int index)
{
	struct node *n;
	long offset = -1;
	int i = 0;

	n = head;
	while(n!=NULL) {
		if(i==index) {
			offset = n->offset;
			break;
		}
		n = n->forward;
		i++;
	}

	return offset;
}

static long getOffset(char *name)
{
	struct node *n;
	long offset = -1;
	int i = 0;

	n = head;
	while(n!=NULL) {
		// dbPrintf("EV: node(%d)= %s\n", i, n->name);
		if(strcmp(n->name, name)==0) {
			offset = n->offset;
			break;
		}
		n = n->forward;
		i++;
	}
	dbPrintf("EV: offset found: %ld\n", offset);
	return offset;
}

int getEVbyName(char *name, char *data, int data_len)
{
	long offset = 0;
	struct ev e;
	FILE *fp;
	size_t rc;

	offset = getOffset(name);
	if(offset<0) {
		dbPrintf("EV: getEVByname: ev(%s) not found\n", name);
		return -1;
	}

	fp = fopen(EV_FILE, "r");
	if(fp == NULL) {
		dbPrintf("EV: getEVByName: evs.dat is not found\n");
		return -2;
	}

	fseek(fp, offset, SEEK_SET);
	rc = fread(&e, sizeof(struct ev), 1, fp);
	if(rc<1) {
		dbPrintf("EV: getEVByname: fread: failed to get name & size\n");
		fclose(fp);
		return -3;
	}

	if(data_len < e.size) {
		dbPrintf("EV: getEVByname: data size is too small\n");
		fclose(fp);
		return -4;
	}

	rc = fread(data, sizeof(char), e.size, fp);
	if(rc!=e.size) {
		dbPrintf("EV: getEVByname: fread: failed to get ev data\n");
		fclose(fp);
		return -5;
	}

	fclose(fp);
	dbPrintf("EV: Size of EV: %d\n", e.size);
	dbPrintf("EV: Name: %s\n", e.name);
	dbPrintf("End EV Data\n");
	return e.size;
}

struct ev *getEVbyIndex(int index, char *data, int data_len)
{
	long offset = 0;
	FILE *fp;
	size_t rc;

	offset = getOffsetByIndex(index);

	if(offset<0) {
		dbPrintf("EV: getEVByIndex: ev(index=%d) not found\n", index);
		EVError = -1;
		return &e;
	}

	fp = fopen(EV_FILE, "r");
	if(fp == NULL) {
		dbPrintf("EV: getEVByIndex: evs.dat is not found\n");
		EVError = -2;
		return &e;
	}

	fseek(fp, offset, SEEK_SET);
	rc = fread(&e, sizeof(struct ev), 1, fp);
	if(rc<1) {
		dbPrintf("EV: getEVByIndex: fread: failed to get name & size\n");
		fclose(fp);
		EVError = -3;
		return &e;
	}

	if(data_len < e.size) {
		dbPrintf("EV: getEVByIndex: data size is too small\n");
		fclose(fp);
		EVError = -4;
		return &e;
	}

	rc = fread(data, sizeof(char), e.size, fp);
	if(rc!=e.size) {
		dbPrintf("EV: getEVByIndex: fread: failed to get ev data\n");
		fclose(fp);
		EVError = -5;
		return &e;
	}

	fclose(fp);
	EVError = e.size;
	return &e;
}

int setEV(char *name, char *data, int data_len)
{
//    long offset = 0;
	int read_size;
	struct ev e;
	struct node *n;
	struct node *n_last;
	FILE *fp;
	FILE *fp_tmp;
//    size_t rc;
	char buf[EV_DATA_MAX_LEN];
	char new_ev = 1;

	//the evs.dat must exist 
	rename(EV_FILE, EV_TMP);
	fp_tmp = fopen(EV_TMP, "r");

	fp = fopen(EV_FILE, "w");
	if(fp == NULL) {
        dbPrintf("setEV: failed to open evs.dat, retrying...\n");
        sleep(1);
        fp = fopen(EV_FILE, "w");
        if(fp == NULL) {
            dbPrintf("setEV: failed to create new evs.dat\n");
            return -1;
        }
	}

	n = head;
	n_last = n;
//	offset = 0;

	while(n!=NULL) {
		fseek(fp_tmp, n->offset, SEEK_SET); //move to next ev entry
		n->offset = ftell(fp); //update offset to the value of new evs file;
		if(strncmp(n->name, name, EV_NAME_MAX_LEN)) {
			//save the ev which is not matched
			read_size = fread(&e, sizeof(struct ev), 1, fp_tmp);	//name
			if(read_size<=0) {
				dbPrintf("EV Size is less than 0... %s %ld \n", n->name, n->offset);
				exit(0);
				//break;
			}
			fwrite(&e, sizeof(struct ev), 1, fp);

			if ( e.size > 0 )
			{
				read_size = fread(buf, sizeof(char), e.size, fp_tmp);	//data
				if(read_size<=0) {
					dbPrintf("EV Data is less than 0... %s %ld \n", n->name, n->offset);
				// break;
				}
				else
					fwrite(buf, sizeof(char), e.size, fp);
			}
		}
		else {
			//replace ev and save ev to tmp file.
			clearEVName(e.name);
			strncpy(e.name, name, EV_NAME_MAX_LEN);
			e.size = data_len;
			fwrite(&e, sizeof(struct ev), 1, fp);

			memcpy(buf, data, data_len);
			fwrite(buf, sizeof(char), data_len, fp);

			new_ev = 0; //clear the new_ev flag
		}
		n_last = n;
		n = n->forward;
	}

	if(new_ev) {
		n = new_node();
        if (n) {
            n->offset = ftell(fp);
            clearEVName(n->name);
            strncpy(n->name, name, EV_NAME_MAX_LEN);
            if(n_last == NULL) {
                head = n;
            }
            else {
                insque(n, n_last);
            }

            e.size = data_len;
            clearEVName(e.name);
            strcpy(e.name, name);
            fwrite(&e, sizeof(struct ev), 1, fp);
            fwrite(data, sizeof(char), data_len, fp);
        }
	}

	fsync(fileno(fp));
	fclose(fp);

	fclose(fp_tmp);
	remove(EV_TMP);

	return 0;
}

int delEV(char *name)
{
	long offset = 0;
	int read_size;
	struct ev e;
	struct node *n;
	struct node *n_tmp;
	FILE *fp;
	FILE *fp_tmp;
//	size_t rc;
	char buf[EV_DATA_MAX_LEN];

	offset = getOffset(name);
	if(offset<0) {
		dbPrintf("delEV: ev(%s) not found\n", name);
		return 0;
	}

	//the evs.dat must exist 
	rename(EV_FILE, EV_TMP);
	fp_tmp = fopen(EV_TMP, "r");

	fp = fopen(EV_FILE, "w");
	if(fp == NULL) {
		dbPrintf("delEV: failed to create new evs.dat\n");
		return -1;
	}
	
	n = head;
	offset = 0;

	while(n!=NULL) {
		fseek(fp_tmp, n->offset, SEEK_SET);
		n->offset = ftell(fp); //update offset to the value of new evs file;
		if(strcmp(n->name, name)) {
			//save ev which is not matched
			read_size = fread(&e, sizeof(struct ev), 1, fp_tmp);	//name
			if(read_size<=0) {
				dbPrintf("EV Size is less than 0...\n");
				//break;
			}
			fwrite(&e, sizeof(struct ev), 1, fp);

			read_size = fread(buf, sizeof(char), e.size, fp_tmp);	//data
			if(read_size<=0) {
				dbPrintf("EV Size is less than 0...\n");
				//break;
			}
			fwrite(buf, sizeof(char), e.size, fp);
		}
		else {
			//skip ev
			n_tmp = n;
		}
		n = n->forward;
	}

	remque(n_tmp); //remove del ev from head

	fsync(fileno(fp));
	fclose(fp);

	fclose(fp_tmp);
	remove(EV_TMP);

	return 0;
}

int getNumOfAllEV(void)
{
	struct node *n;
	int i = 0;

	n = head;
	while(n!=NULL) {
		i++;
		n = n->forward;
	}

	return i;
}

int getSizeOfEVfile(void)
{
	struct stat buf;
	int fd;

	fd = open(EV_FILE, O_RDONLY);
	if(fd<0) {
		return 0;
	}

	if(fstat(fd, &buf)<0) {
		return 0;
	}

	return buf.st_size;
}
