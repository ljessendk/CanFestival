/*
This file is part of CanFestival, a library implementing CanOpen Stack.

Copyright (C): Francois Beaulier

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>

#include "gendcf.h"

#define COPEN_DEBUG_RECUP_DICO 0

/*
 * Displays the content of the concise DCF array dcfdata
 */
void dcf_data_display(uint8_t dcfdata[][DCF_MAX_SIZE])
{
	uint32_t var, NbEntries, Index, SubIdx, NbData;
	uint8_t *pDico;
    uint8_t NodeId;
	printf("Concise DCF data found in file :\n");
	for(NodeId = 1 ; NodeId < DCF_MAX_NODE_ID ; NodeId++) {
	    pDico = dcfdata[NodeId-1];
        NbEntries = pDico[0] | pDico[1]<<8 | pDico[2]<<16 | pDico[3]<<24;
        pDico += 4;
        printf("--- Node Id = %u : %u entries\n", NodeId, NbEntries);
        while(NbEntries--) {
            Index = pDico[0] | pDico[1]<<8;
            pDico += 2;
            SubIdx = pDico[0];
            pDico += 1;
            NbData = pDico[0] | pDico[1]<<8 | pDico[2]<<16 | pDico[3]<<24;
            pDico += 4;
		    printf("    Index=%X SubIdx=%X NbData=%X : ",Index,SubIdx,NbData);
		    for(var=0 ; var<NbData ; var++)
			    printf("%.2X ",pDico[var]);
            pDico += NbData;
		    printf("\n");
	    }
	    printf("\n");
    }
}

/* dcf_read_in_file
 ** Read in the file who's name is in filename all DCF entries 
 ** Entries are stored in dcfdata array
 ** Return:
 **     0 : no error
 **    -1 : can't open file 
 **    -2 : Node ID > DCF_MAX_NODE_ID in file
 **    -3 : too many lines for one entry
 **    -4 : out of bounds value 
 **    -5 : syntax error in file
 */
int dcf_read_in_file(char *fileName, uint8_t dcfdata[][DCF_MAX_SIZE])
{
	FILE *DicoFile;
	char ligne[65];
    char *pt;
	uint32_t NodeId, Data[4], var, ObjNbr=0, Offset=0;
	uint32_t Index, SubIdx, NbData;
	int ret, errsv, nb;
	uint8_t *pDico=NULL, *pObj=NULL;

	DicoFile = fopen(fileName,"r");
	errsv = errno;
	/* Si pas de fichier ce n'est pas une erreur : la config par defaut convient */ 
	if(DicoFile == NULL) {
		if( errsv == ENOENT )
			return 0;
		else
			return -1;
	}
	ret=0;
	while(1){
		pt = fgets(ligne,65,DicoFile);
        if(pt == NULL)
            break;
		if(feof(DicoFile)) {
#if COPEN_DEBUG_RECUP_DICO
			printf("Terminated\n");
#endif
			break;
		}
#if COPEN_DEBUG_RECUP_DICO
		printf("line : %x : %s \n",ligne[0],ligne);
        
#endif
		if((ligne[0]=='#')||(ligne[0]=='\n')||(ligne[0]==' ')||(ligne[0]==0x0a))
			continue;
		else if(ligne[0]=='[') {
			sscanf(ligne+1,"%u",&NodeId);
#if COPEN_DEBUG_RECUP_DICO
			printf("--- Node ID=%u \n",NodeId); 
#endif
			if((NodeId > DCF_MAX_NODE_ID) || (NodeId == 0)) {
                ret=-2;
                break;
            }
            ObjNbr = 0;
            Offset = 0;
            pDico = dcfdata[NodeId-1];
            pObj = pDico + 4;
		}
		else if(isxdigit(ligne[0]))	{
			nb = sscanf(ligne,"%X %X %X %X %X %X %X",&Index,&SubIdx,&NbData,&Data[0],&Data[1],&Data[2],&Data[3]);
            if(nb != 3+NbData) {
			    ret=-5;
                break;
            }
#if COPEN_DEBUG_RECUP_DICO
			printf("Index=%X SubIdx=%X NbData=%X : ",Index,SubIdx,NbData);
			for(var=0;var<NbData;var++)printf("%.2X ",Data[var]);
			printf("\n");
#endif
			/* si tous les paramètres récupérés sont bons, on ajoute un objet dans le tableau */
			if((Index <= 0xFFFF) && (SubIdx <= 0xFF) && (NbData <= 4)) {
                if(Offset > (DCF_MAX_SIZE-7-NbData)){
					ret=-3;
					break;
                }
                /* Load DCF in little endian */
				pObj[Offset++] = Index;
				pObj[Offset++] = Index>>8;
				pObj[Offset++] = SubIdx;
				pObj[Offset++] = NbData;
				pObj[Offset++] = NbData>>8;
				pObj[Offset++] = NbData>>16;
				pObj[Offset++] = NbData>>24;
				for(var=0 ; var < NbData ; var++)
					pObj[Offset++] = Data[var];
                ObjNbr++;
                /* Update number of entries at the begining of the DCF */
                pDico[0] = ObjNbr;
                pDico[1] = ObjNbr>>8;
                pDico[2] = ObjNbr>>16;
                pDico[3] = ObjNbr>>24;
			}
			else {
				ret=-4;
				break;
			}
		}
		else {
			ret=-5;
			break;
		}
	}
	fclose( DicoFile );
	return ret;
}

#if 0
int main(void)
{
    int ret;
    uint8_t TabConciseDCF[DCF_MAX_NODE_ID][DCF_MAX_SIZE] = {{0}};
    ret = dcf_read_in_file(DEVICE_DICT_NAME, TabConciseDCF);
    if(ret){
        printf("Erreur %d\n", ret);
        return 1;
    }
    dcf_data_display(TabConciseDCF);
    return 0;
}
#endif
