/*
// Copyright (c) 2021 Hewlett-Packard Development Company, L.P.
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

#define EV_MAX_LEN		4000
#define EV_NAME_MAX_LEN	32
#define EV_HEAD_LEN		(EV_NAME_MAX_LEN+2)
#define EV_DATA_MAX_LEN	(EV_MAX_LEN - EV_HEAD_LEN)
#define EV_FILE "/tmp/evs.dat"
#define EV_TMP	"/tmp/evs.tmp"

struct node {
	struct node *forward;
	struct node *backward;
	char name[32];
	long offset;
};

struct ev {
	uint16_t size;
	char name[32];
};

static struct node* new_node(void)
{
	struct node *n;

	n = (struct node *)malloc(sizeof(struct node));

	if(n == NULL) {
		printf("ev: new_entry(): malloc failed\n");
	}
	n->forward = NULL;
	n->backward = NULL;
	return n;
}

struct node *head = NULL;

void printEVs()
{
	struct node *n = head;
	int size = 0;
	printf("Printing EVs:\n");
	while(n != NULL)
	{
		printf("node: %s, offset: %lu\n", n->name, n->offset);
		n = n->forward;
		size++;
	}
	printf("Printed %d EVs\n", size);
	printf("Completed Printing EVs.\n");
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

	fp = fopen(EV_FILE, "a+");

	while(1) {
		rc = fread(&e, sizeof(struct ev), 1, fp);
		printf("Node: %d\n", i);
		if(rc<1) {
			printf("no more ev.\n");
			break;
		}
		n = new_node();
		if(head==NULL) {
			printf("Saving memory address of first node.\n");
			head = n; //saves memory address of first node
		}
		if(prev_node !=NULL)
			printf("Prev Node: %s\n", prev_node->name);
		else
			printf("Prev Node is NULL\n");
		insque(n, prev_node);

		strncpy(n->name, e.name, 32);
		printf("found instance: %s\n", n->name);
		n->offset = offset;
		printf("offset: %lu\n", n->offset);
		prev_node = n;
		offset += sizeof(struct ev);
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
	struct node *n;
	struct node *n_tmp;
	FILE *fp;

	n = head;
	while(n != NULL) {
		n_tmp = n;
		n = n->forward;

		remque(n_tmp);
		free(n_tmp);
	}
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
		printf("node(%d)= %s\n", i, n->name);
		if(strcmp(n->name, name)==0) {
			offset = n->offset;
			break;
		}
		n = n->forward;
		i++;
	}
	printf("offset found: %ld\n", offset);
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
		printf("getEVByname: ev(%s) not found\n", name);
		return -1;
	}

	fp = fopen(EV_FILE, "r");
	if(fp == NULL) {
		printf("getEVByName: evs.dat is not found\n");
		return -2;
	}

	fseek(fp, offset, SEEK_SET);
	rc = fread(&e, sizeof(struct ev), 1, fp);
	if(rc<1) {
		printf("getEVByname: fread: failed to get name & size\n");
		fclose(fp);
		return -3;
	}

	if(data_len < e.size) {
		printf("getEVByname: data size is too small\n");
		fclose(fp);
		return -4;
	}

	rc = fread(data, sizeof(char), e.size, fp);
	if(rc!=e.size) {
		printf("getEVByname: fread: failed to get ev data\n");
		fclose(fp);
		return -5;
	}

	fclose(fp);
	printf("Size of EV: %d\n", e.size);
	printf("EV Name: %s\n", e.name);
	printf("EV Data:\n");
	for(int i = 0; i < (sizeof(char) * e.size); ++i)
	{
		printf("Byte [%d]: 0x%x\n", i, data[i]);
	}
	printf("End EV Data\n");
	return e.size;
}

int getEVbyIndex(int index, char *data, int data_len)
{
	long offset = 0;
	struct ev e;
	FILE *fp;
	size_t rc;

	offset = getOffsetByIndex(index);
	if(offset<0) {
		printf("getEVByIndex: ev(index=%d) not found\n", index);
		return -1;
	}

	fp = fopen(EV_FILE, "r");
	if(fp == NULL) {
		printf("getEVByIndex: evs.dat is not found\n");
		return -2;
	}

	fseek(fp, offset, SEEK_SET);
	rc = fread(&e, sizeof(struct ev), 1, fp);
	if(rc<1) {
		printf("getEVByIndex: fread: failed to get name & size\n");
		fclose(fp);
		return -3;
	}

	if(data_len < e.size) {
		printf("getEVByIndex: data size is too small\n");
		fclose(fp);
		return -4;
	}

	rc = fread(data, sizeof(char), e.size, fp);
	if(rc!=e.size) {
		printf("getEVByIndex: fread: failed to get ev data\n");
		fclose(fp);
		return -5;
	}

	fclose(fp);
	return e.size;
}

int setEV(char *name, char *data, int data_len)
{
	long offset = 0;
	int read_size;
	struct ev e;
	struct node *n;
	struct node *n_last;
	FILE *fp;
	FILE *fp_tmp;
	size_t rc;
	char buf[EV_DATA_MAX_LEN];
	char new_ev = 1;

	//the evs.dat must exist 
	rename(EV_FILE, EV_TMP);
	fp_tmp = fopen(EV_TMP, "r");

	fp = fopen(EV_FILE, "w");
	if(fp == NULL) {
		printf("setEV: failed to create new evs.dat\n");
		return -1;
	}

	n = head;
	n_last = n;
	offset = 0;

	while(n!=NULL) {
		fseek(fp_tmp, n->offset, SEEK_SET); //move to next ev entry
		n->offset = ftell(fp); //update offset to the value of new evs file;
		if(strncmp(n->name, name, EV_NAME_MAX_LEN)) {
			//save the ev which is not matched
			read_size = fread(&e, sizeof(struct ev), 1, fp_tmp);	//name
			if(read_size<=0) {
				printf("EV Size is less than 0...\n");
				//break;
			}
			fwrite(&e, sizeof(struct ev), 1, fp);

			read_size = fread(buf, sizeof(char), e.size, fp_tmp);	//data
			if(read_size<=0) {
				printf("EV Size is less than 0...\n");
				//break;
			}
			fwrite(buf, sizeof(char), e.size, fp);
		}
		else {
			//replace ev and save ev to tmp file.
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
		n->offset = ftell(fp);
		strncpy(n->name, name, EV_NAME_MAX_LEN);
		if(n_last == NULL) {
			head = n;
		}
		else {
			insque(n, n_last);
		}

		e.size = data_len;
		strcpy(e.name, name);
		fwrite(&e, sizeof(struct ev), 1, fp);
		fwrite(data, sizeof(char), data_len, fp);
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
	size_t rc;
	char buf[EV_DATA_MAX_LEN];

	offset = getOffset(name);
	if(offset<0) {
		printf("delEV: ev(%s) not found\n", name);
		return 0;
	}

	//the evs.dat must exist 
	rename(EV_FILE, EV_TMP);
	fp_tmp = fopen(EV_TMP, "r");

	fp = fopen(EV_FILE, "w");
	if(fp == NULL) {
		printf("delEV: failed to create new evs.dat\n");
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
				printf("EV Size is less than 0...\n");
				//break;
			}
			fwrite(&e, sizeof(struct ev), 1, fp);

			read_size = fread(buf, sizeof(char), e.size, fp_tmp);	//data
			if(read_size<=0) {
				printf("EV Size is less than 0...\n");
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
