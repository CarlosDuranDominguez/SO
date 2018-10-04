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
int copynFile(FILE * origin, FILE * destination, int nBytes)
{
	// Complete the function
	char c;
	int i = 0;
	while(i<nBytes && (c = getc(origin)) != EOF){
		if(fwrite(&c,1,1,destination)!=1){
		return -1;
		}
		i++;
	}
	if(c!=EOF)
		return i;
	else
		return -1;
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
char* loadstr(FILE * file)
{
	char c;
	int numBytes = 0;

	while( (c = getc(file))!='\0'&&c!=EOF){
			numBytes ++;
	}
	if(numBytes == 0||c == EOF){
		return NULL;
	}else{
		char* output;
		if((output = malloc(numBytes))==NULL)
			return NULL;
		if(fseek(file,-numBytes,SEEK_CUR)==-1||fread(output, 1, numBytes, file)!=numBytes)
			return NULL;

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
	// Leemos el numero de archivos
	if (fread(nFiles,4,1,tarFile) == 1){
		// Reservamos memoria para las cabeceras
		stHeaderEntry *a = (stHeaderEntry*)malloc((sizeof(char*)+sizeof(unsigned int))**nFiles);

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
					free(a);
					return NULL;
				}
			} else {
				free(a);
				return NULL;
			}
		}
		return a;
	}
	return NULL;
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
	// Abrimos el archivo tar.
	FILE* tarFile = fopen(tarName, "w");
	if(tarFile==NULL)
		return EXIT_FAILURE;
	// Usamos una variable auxilar para abrir los archivos.
	FILE* file;
	//Calculamos el tamanyo del header.
	int spaceHeader=4;
	for(int i = 0; i< nFiles; i++){
		int j = 0;
		while(*(fileNames[i]+j)!='\0'){
			spaceHeader++;
		}
		spaceHeader+=1+sizeof(unsigned int);
	}
	//Saltamos ese tamanyo.
	fseek(tarFile,spaceHeader,SEEK_CUR);
	//Reservamos memoria para los headers
	stHeaderEntry* headers = (stHeaderEntry*)malloc((sizeof(char*)+sizeof(unsigned int))*nFiles);
	//variable auxiliar para guardar los caracteres leidos
	char aux;
	//variable que calcula el tamanyo del archivo
	int size;
	//Recorremos todos los archivos a comprimir
	for(int i = 0; i<nFiles; i++){
		//abrimos
		file = fopen(fileNames[i],"r");
		//reseteamos el tamanyo
		size = 0;
		//leemos hasta que llegamos el fin del archivo
		do{
			//Si hay una mala lectura o escritura salimos de la subrutina con un error
			if(fread(&aux,1,1,file)!=1 || fwrite(&aux,1,1,tarFile)!=1){
				free(headers);
				return EXIT_FAILURE;
			}
			//Calculamos el tamanyo
			size++;
		}while(aux!=EOF);
		// determinamos la cabecera en memoria
		headers[i].name = fileNames[0];
		headers[i].size = size;
		//Si cierra con un error devolvemos error
		if(fclose(file)!=0) {
			free(headers);
			return EXIT_FAILURE;
		}
	}
	// Nos desplazamos el inicio del archivo y despues escribimos el numero de archivos, 
	// si ocurre un error, terminamos la ejecucion
	if(fseek(tarFile,0,SEEK_SET)!=0||fwrite(&nFiles,4,1,tarFile)!=1){
		free(headers);
		return EXIT_FAILURE;
	}

	//Escribimos las cabeceras
	for(int i = 0; i<nFiles; i++){
		//Escribimos el nombre
		int j = 0;
		do{
			fwrite(headers[i].name+j,1,1,tarFile);
			j++;
		}while(headers[i].name[j]!='\0');
		//Escribimos el tamanyo
		if(fwrite(&headers[i].size,sizeof(unsigned int),1,tarFile)!=1) {
			free(headers);
			return EXIT_FAILURE;
		}
	}
	// Liberamos la memoria de los headers
	free(headers);
	//Cerramos el archivo y en caso lo reflejamos en el return.
	if(fclose(tarFile)!=0)
			return EXIT_FAILURE;

	return EXIT_SUCCESS;
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
	FILE* tarfile;
	if((tarfile=fopen(tarName, "r"))!=NULL) {
		int nFiles;
		// Leemos el tamanyo del archivo y si ocurre un error terminamos la ejecucion.
		if(fread(&nFiles, 4, 1, tarfile)!=1){
			return EXIT_FAILURE;
		}
		//Leemos la cabecera y en caso de error terminamos la ejecucion
		stHeaderEntry *a = readHeader(tarfile, &nFiles); 
		if(a == NULL)
			return EXIT_FAILURE;
		// Leemos cada archivo senyalado por la cabecera y lo copiamos el fichero con nombre tarName.
		// SI ocurre un error terminamos el programa
		for(int i = 0; i < nFiles; i++){
			FILE* destinationFile;
			if((destinationFile= fopen(a[i].name, "w"))==NULL || copynFile(tarfile, destinationFile, a[i].size)==-1 || fclose(destinationFile)==EOF){
				return EXIT_FAILURE;
				free(a);
			}
		}
		free(a);
		//Cerramos el archivo y en caso lo reflejamos en el return.
		if(fclose(tarfile)==EOF)
			return EXIT_FAILURE;
		return EXIT_SUCCESS;
    }
		return EXIT_FAILURE;
}
