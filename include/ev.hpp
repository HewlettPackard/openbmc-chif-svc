/*
// Copyright (c) 2021-2025 Hewlett Packard Enterprise Development, LP
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

#ifndef __EV_H__
#define __EV_H__

#define EV_NAME_MAX_LEN	32
#define EV_FILE_MAX_SIZE (64 * 1024) //unit byte
#define EV_FILE_HEADER	 64 //unit byte
#define EV_MAX_LEN			4000
#define EV_HEAD_LEN 		(EV_NAME_MAX_LEN+2)
#define EV_DATA_MAX_LEN	(EV_MAX_LEN - EV_HEAD_LEN)

struct node {
        struct node *forward;
        struct node *backward;
        char name[EV_NAME_MAX_LEN];
        long offset;
};

struct ev {
        uint16_t size;
        char name[EV_NAME_MAX_LEN];
};

extern int EVError;

int initEV(void);
int getEVbyName(char *name, char *data, int data_len);
struct ev *getEVbyIndex(int index, char *data, int data_len);
int getNumOfAllEV(void);
int getSizeOfEVfile(void);
int setEV(char *name, char *data, int data_len);
int delEV(char *name);
int clearEV(void);
void printEVs(void);
#endif
