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

#include "sysrom.hpp"
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

//#include <sdbusplus>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>
#include <sdbusplus/message.hpp>
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/elog.hpp>


char const *mdrV2Service = "xyz.openbmc_project.Smbios.MDR_V2";
char const *mdrV2Interface = "xyz.openbmc_project.Smbios.MDR_V2";
char const *smbios_path = "/var/lib/smbios/smbios2";
char const* mdrV2Path = "/xyz/openbmc_project/Smbios/MDR_V2";
uint8_t mdrTypeII = 2;
uint8_t dirVer = 1;

int Rom_Response(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;

	respPkt->header.pkt_size = sizeof(struct ChifPktHeader);
	respPkt->header.sequence = recvPkt->header.sequence;
	respPkt->header.command = recvPkt->header.command;
	respPkt->header.service_id = ROM_SERVICE_ID;

	return respPkt->header.pkt_size;
}


int RomHandler(void *recv, int recv_len, void *resp, int resp_len)
{
	struct ChifPkt *recvPkt = (struct ChifPkt *)recv;
	struct ChifPkt *respPkt = (struct ChifPkt *)resp;
	struct SmbiosPktHeader *s_headerPkt;

	char const *temp_path = "/var/lib/smbios/smbios_temp";
	int total_smbios_recs; //total number of SMBIOS Records
	FILE *SMBIOS_ptr;

	memset(resp, 0, resp_len);
	printf("RomHandler: command:0x%08x\n", recvPkt->header.command);

	switch(recvPkt->header.command) {
		case 0x03:	//SMBIOS Begin Command
				//s_headerPkt = (struct SmbiosPktHeader *)recvPkt->msg; //create SMBIOS records buffer
				//total_smbios_recs = s_headerPkt->NumSmbiosRecs;		  //get total SMBIOS records from message

				printf("Begin receiving SMBIOS records\n");

				if (WriteSmbiosRecords(SMBIOS_ptr, (char *)temp_path, 0, -1))
				{
					return Rom_Response(recv, recv_len, resp, resp_len);
				}
				return -1;

		case 0x04:	//new SMBIOS Record
				printf("Receiving single SMBIOS record\n");
				if (WriteSmbiosRecords(SMBIOS_ptr, (char *)temp_path, (char *)recvPkt->msg, recvPkt->header.pkt_size))
				{
					return Rom_Response(recv, recv_len, resp, resp_len);
				}
				return -1;
		case 0x05:	//SMBIOS END
			if (WriteSmbiosRecords(SMBIOS_ptr, (char *)temp_path, 0, -2))
				{
					return Rom_Response(recv, recv_len, resp, resp_len);
				}
				return -1;
		case 0x06:
				return Rom_Response(recv, recv_len, resp, resp_len);
		case 0x0c:	//new SMBIOS Record blob
				printf("Receiving multiple SMBIOS records\n");
				if (WriteSmbiosRecords(SMBIOS_ptr, (char *)temp_path, (char *)recvPkt->msg, recvPkt->header.pkt_size))
				{
					return Rom_Response(recv, recv_len, resp, resp_len);
				}
				return -1;
			break;
		default:
			//other rom command doesn't need to response
			return -1;
	}
}

int WriteSmbiosRecords(FILE *fptr, char path[], char *buffer, int size)
{
	fptr = (size==-1) ? fptr = fopen(path, "wb+") : fopen(path, "ab+"); //If size is -1 (fresh records) overwrite file, if not append
	if (fptr != NULL)
	{
		const uint8_t MDRSMBIOSHdr_sz = sizeof(struct MDRSMBIOSHeader);
		if (size == -1) //Starting SMBIOS transaction
		{
			printf("Starting SMBIOS Download.\n");
			dirVer++;												//Increase DirVer by 1 so that SMBIOS MDR updates aswell
			uint8_t buff[MDRSMBIOSHdr_sz]; 			//create buffer
			memset(buff, 0, MDRSMBIOSHdr_sz);
			fwrite(buff, MDRSMBIOSHdr_sz, 1, fptr); 
			fclose(fptr);
		}
		else if (size == -2) //Ending SMBIOS transaction
		{
			struct MDRSMBIOSHeader* mdrHdr;
			uint8_t buff[MDRSMBIOSHdr_sz];
			mdrHdr = (MDRSMBIOSHeader*) &buff;
			mdrHdr->dirVer = dirVer;
    		mdrHdr->mdrType = mdrTypeII;
    		mdrHdr->timestamp = (uint32_t)time(NULL);
			fseek(fptr, 0, SEEK_END);		//Seek to end of file to get length
   			mdrHdr->dataSize = ftell(fptr) - (sizeof(uint8_t)*10);  //Save length of file - the header
			printf("Size of file: %d\n", mdrHdr->dataSize);
			fclose(fptr);
			fptr = fopen(path, "rb+");		 	 	//Move pointer back to start of file
			fwrite(mdrHdr, MDRSMBIOSHdr_sz, 1, fptr); //Save MDR Header to beginning of file
			fclose(fptr);
			printf("checking if smbios file exists.\n");
			struct stat finfo;
			if (stat(smbios_path, &finfo) == 0) //if smbios file already exists
			{ 
				if (remove(smbios_path) ==0)
				{
					printf("Removed old SMBIOS file.\n");
				}
				else{
					printf("Error removing old SMBIOS file %s\n", smbios_path);
					perror("Error removing old SMBIOS file");
					return -1;
				}
			}
			else{
				printf("No SMBIOS file to remove.\n");
			}
			if(rename(path, smbios_path) == 0)
			{
				printf("SMBIOS-MDR file successfully replaced.\n");
				syncSmbiosData();
			}
			else
			{
				perror("Error renaming file");
				printf("Failed to copy SMBIOS Records file. Exists at:\n\t%s\nBut was not copied to the proper location:\n\t%s\n", path, smbios_path);
				return -1;
			}
		}
		else //Write buffer into smbios records file
		{
			uint32_t NumRecs = (uint32_t)buffer[0];
			printf("Number of Records = %d/0x%X\n", NumRecs, NumRecs);
			uint16_t RecSz;
			uint32_t i = sizeof(uint32_t);
			while (i < size){
				RecSz = (uint16_t)buffer[i];
				printf("Record Size = %d/0x%X\n", RecSz, RecSz);
				i += sizeof(uint16_t);
				fwrite(&buffer[i], RecSz, 1, fptr);
				i += RecSz;
			}
			fclose(fptr);
		}
	}
	else
	{
		printf("Failed to create SMBIOS Records file. Path:%s\n", path);
		return -1;
	}
	return 1;
}

bool syncSmbiosData()
{
    bool status = false;
    sdbusplus::bus::bus bus = sdbusplus::bus::new_default_system();
    sdbusplus::message::message method =
        bus.new_method_call(mdrV2Service, mdrV2Path,
                            mdrV2Interface, "AgentSynchronizeData");

    try
    {
        sdbusplus::message::message reply = bus.call(method);
        reply.read(status);
    }
    catch (sdbusplus::exception_t& e)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Error Sync data with service",
            phosphor::logging::entry("ERROR=%s", e.what()),
            phosphor::logging::entry("SERVICE=%s", mdrV2Service),
            phosphor::logging::entry("PATH=%s", mdrV2Path));
        return false;
    }

    if (!status)
    {
        phosphor::logging::log<phosphor::logging::level::ERR>(
            "Sync data with service failure");
        return false;
    }

    return true;
}
