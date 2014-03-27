/*
===========================================================================
    Copyright (C) 2010-2013  Ninja and TheKelm of the IceOps-Team

    This file is part of CoD4X17a-Server source code.

    CoD4X17a-Server source code is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    CoD4X17a-Server source code is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
===========================================================================
*/



//Non volatile config
//Changed settings will be written to / loaded from a file called nvconfig.cfg
//If a nonvolatile setting is changed it is saved to that file immediately
#include "q_shared.h"
#include "qcommon_io.h"
#include "qcommon_mem.h"
#include "filesystem.h"
#include "cmd.h"
#include "server.h"
#include "hl2rcon.h"
#include "sv_auth.h"

#include <string.h>

#define MAX_NVCONFIG_SIZE 2048*128

#define NV_ProcessBegin NV_LoadConfig
#define NV_ProcessEnd NV_WriteConfig

/*
================
NV_ParseConfigLine
================
*/

qboolean NV_ParseConfigLine(char* line, int linenumber){

    if(!Q_stricmp(Info_ValueForKey(line, "type") , "cmdMinPower")){

        if(!Cmd_InfoSetPower( line )){
            Com_DPrintf("Warning at line: %d\n", linenumber);
            return qtrue; //Don't rise errors here! This can happen for commands we add at a later stage
        }
        return qtrue;

    }else if(!Q_stricmp(Info_ValueForKey(line, "type") , "rconAdmin")){

        if(!HL2Rcon_InfoAddAdmin( line ))
        {
            Com_Printf("Error at line: %d\n", linenumber);
            return qfalse;
        }
        return qtrue;

    }else if(!Q_stricmp(Info_ValueForKey(line, "type") , "authAdmin")){

        if(!Auth_InfoAddAdmin( line ))
        {
            Com_Printf("Error at line: %d\n", linenumber);
            return qfalse;
        }
        return qtrue;

    }else if(!Q_stricmp(Info_ValueForKey(line, "type") , "admin")){
	Com_Printf("^1WARNING: ^7GUID based admin authorization has been disabled. Go to https://guidError.iceops.in/ for details.\n");
	return qfalse;
	/*
        if(!SV_RemoteCmdInfoAddAdmin( line ))
        {
            Com_Printf("Error at line: %d\n", linenumber);
            return qfalse;
        }
        return qtrue;
        */

    }else{
        Com_Printf("Error: unknown type (line: %d)\n", linenumber);
        return qfalse;
    }
}


/*
================
NV_LoadConfig
================
*/

void NV_LoadConfig(){

    int read, i;
    int error = 0;
    char buf[256];
    *buf = 0;
    fileHandle_t file;

    SV_RemoteCmdClearAdminList();
    HL2Rcon_ClearSourceRconAdminList();
    Auth_ClearAdminList();

    FS_SV_FOpenFileRead("nvconfig.dat", &file);
    if(!file){
        Com_DPrintf("Couldn't open nvconfig.dat for reading\n");
        return;
    }
    Com_Printf( "loading nvconfig.dat\n");

    i = 0;

    while(qtrue){
        read = FS_ReadLine(buf,sizeof(buf),file);
        if(read == 0){
            FS_FCloseFile(file);
            Com_Printf("Loaded nvconfig.dat %i errors\n", error);
            return;
        }
        if(read == -1){
            Com_Printf("Can not read from nvconfig.dat\n");
            FS_FCloseFile(file);
            return;
        }
        i++;//linecouter

        if(!*buf || *buf == '/' || *buf == '\n'){
            continue;
        }
        if(!NV_ParseConfigLine(buf, i)){
            error++;
        }
    }
}

/*
================
NV_WriteConfig
================
*/

void NV_WriteConfig(){

    char* buffer;

    buffer = Hunk_AllocateTempMemory(MAX_NVCONFIG_SIZE);
    if(!buffer){
        Com_Printf( "Error Updating NVConfig: Hunk_Alloc failed\n" );
        return;
    }
    Com_Memset(buffer,0,MAX_NVCONFIG_SIZE);
    Q_strcat(buffer,MAX_NVCONFIG_SIZE,"//Autogenerated non volatile config settings\n");

    Cmd_WritePowerConfig( buffer, MAX_NVCONFIG_SIZE );
    SV_RemoteCmdWriteAdminConfig( buffer, MAX_NVCONFIG_SIZE );
    HL2Rcon_WriteAdminConfig( buffer, MAX_NVCONFIG_SIZE );
    Auth_WriteAdminConfig( buffer, MAX_NVCONFIG_SIZE );

    FS_SV_WriteFile("nvconfig.dat", buffer, strlen(buffer));
    Hunk_FreeTempMemory( buffer );
    Com_DPrintf("NV-Config Updated\n");
}
