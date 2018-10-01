#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
	// Complete the function

	char c;
	int i = 0;

	while(i<nBytes&&fread(&c,1,1,origin) == 1){
		if(fwrite(&c,1,1,destination)!=1){
		return -1;

		}
		i++;
	}


	return i;
}

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor 
 * 
 * The loadstr() function must allocate memory from the heap to store 
 * the contents of the string read from the FILE. 
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc()) 
 * 
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{
	char c;
	int numBytes = 0;

	while( c = getc(file)!='\0'){
			numBytes ++;

	}
	if(numBytes == 0){
		return NULL;
	}else{
		fseek(file,-numBytes,SEEK_CUR);
			char* output = malloc(numBytes);
			fread(output, 1, numBytes, file);
			// Complete the function
			return output;
	}

}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor 
 * nFiles: output parameter. Used to return the number 
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores 
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry* readHeader(FILE * tarFile, int *nFiles)
{

	if (fread(nFiles,4,1,tarFile) == 1){

	stHeaderEntry *a = (stHeaderEntry *)malloc(sizeof(stHeaderEntry)**nFiles);
	for(int i = 0; i<*nFiles; i++){
		char* cadena = loadstr(tarFile);

		if(cadena != NULL)
		{
			unsigned int n;
			if(fread(&n, sizeof(unsigned int), 1, tarFile)==1)
			{
				a[i] = (stHeaderEntry){cadena, n};
			}
			else {
				return NULL;
			}
		}else{return NULL;}
	}

	return a;
}

/** Creates a tarball archive 
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive. 
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as 
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof 
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size) 
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int createTar(int nFiles, char *fileNames[], char tarName[])
{
// Hay que comprobar que los archivos se abren bien y terminar la rutina
	FILE* file = fopen(tarName, 'w');
	if(fwrite(&nFiles,sizeof(int),1,file)==1){
		//Writting headers.
		for(int i = 0; i<nFiles; i++)
		{
			char* a = fileNames[i];
			do{
				if(fwrite(a, 1,1,file)!=1)
					return EXIT_FAILURE;
			}while((a++)*!='/0');

			if(fwrite(NULL,sizeof(unsigned int),1,file))
				return EXIT_FAILURE;
		}
		FILE * originFile;
		for(int i = 0; i<nFiles; i++)
		{
			originFile = fopen(fileNames[i], 'r');
			char c;
			unsigned int nBytes = 0;
			while(c=getc(originFile)!=EOF){
				if(fwrite(&c,1,1,file)==1)
					return EXIT_FAILURE;
				i++;
			}
		}
	}

	return EXIT_FAILURE;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the 
 * tarball's data section. By using information from the 
 * header --number of files and (file name, file size) pairs--, extract files 
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
	// Complete the function
	int nFiles;
	FILE* tarfile = fopen(tarName, 'r'); //abro el tarfile de la ruta
	stHeaderEntry *a = readHeader(tarfile, nFiles); //leo el header y relleno el array 'a'
	for(int i = 0; i < nFiles; i++){
		//aquí habría que leer el array 'a'
	}
	return EXIT_SUCCESS;
    }
    //Probablemente falta algo más porque habrá que usar la 'a' para algo, pero creo que no va por mal camino
	return EXIT_FAILURE;
}
