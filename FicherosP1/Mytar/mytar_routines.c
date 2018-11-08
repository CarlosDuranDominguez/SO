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
	while(i<nBytes && !feof(origin)){
		fread(&c, 1, 1, origin);
		if(fwrite(&c,1,1,destination)!=1){
		return -1;
		}
		i++;
	}
	if(c!=EOF)
		return i;
	else{
		return -1;
	}
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
	numBytes++;
	if(numBytes == 0||c == EOF){
		return NULL;
	}else{
		char* output;
		if((output = malloc(numBytes))==NULL)
			return NULL;
		if(fseek(file,-numBytes,SEEK_CUR)==-1||fread(output, 1, numBytes, file)!=numBytes) {
			free(output);
			return NULL;
		}
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
	if (fread(nFiles,sizeof(int),1,tarFile) == 1){
		// Reservamos memoria para las cabeceras
		stHeaderEntry *a = (stHeaderEntry*)malloc((sizeof(stHeaderEntry))*(*nFiles));
		char** cadenas = (char**)malloc(sizeof(char*)*(*nFiles));
		for(int i = 0; i<*nFiles; i++){ 
			cadenas[i] = loadstr(tarFile);		
			if(cadenas[i] != NULL)
			{	
				unsigned int n;
				if(fread(&n, sizeof(unsigned int), 1, tarFile)==1)
				{					
					a[i] = (stHeaderEntry){cadenas[i], n};
				}
				else {
					free(cadenas);
					free(a);
					return NULL;
				}
			} else {
				free(cadenas);
				free(a);
				return NULL;
			}
		}
		free(cadenas);
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
	FILE* tarFile = fopen(tarName, "wb");
	if(tarFile==NULL)
		return EXIT_FAILURE;
	// Usamos una variable auxilar para abrir los archivos.
	FILE* file;
	//Calculamos el tamanyo del header.
	int spaceHeader=sizeof(int);
	for(int i = 0; i< nFiles; i++){
		int j = 0;
		while(*(fileNames[i]+j)!='\0'){
			spaceHeader++;
			j++;
		}
		spaceHeader+=sizeof(char)+sizeof(unsigned int);
	}
	//variable auxiliar para guardar los caracteres leidos
	char aux = nFiles;
	fwrite(&aux, sizeof(int), 1, tarFile);
	aux = 0;
	fwrite(&aux,1,spaceHeader-sizeof(int),tarFile);
	//Saltamos ese tamanyo.
	fseek(tarFile,(long)spaceHeader,SEEK_SET);
	//Reservamos memoria para los headers
	stHeaderEntry* headers = (stHeaderEntry*)malloc((sizeof(stHeaderEntry))*nFiles);

	//variable que calcula el tamanyo del archivo
	int size;
	//Recorremos todos los archivos a comprimir
	for(int i = 0; i<nFiles; i++){
		
		//abrimos
		file = fopen(fileNames[i],"rb");
		//reseteamos el tamanyo
		size = 0;
		//leemos hasta que llegamos el fin del archivo
		do{
			
			if(fread(&aux,1,1,file) != 1 || fwrite(&aux,1,1,tarFile) != 1) {
				

			} else {
				//Calculamos el tamanyo
				size++;	
			}	
		}while(!feof(file));
			
		
		// determinamos la cabecera en memoria
		headers[i].name = fileNames[i];
		headers[i].size = (unsigned int)size;
		//Si cierra con un error devolvemos error
		if(fclose(file)!=0) {
			fclose(tarFile);
			free(headers);
			return EXIT_FAILURE;
		}
		
	}
	// Nos desplazamos el inicio del archivo y despues escribimos el numero de archivos, 
	// si ocurre un error, terminamos la ejecucion
	if(fseek(tarFile,0,SEEK_SET)!=0 || fwrite(&nFiles,sizeof(int),1,tarFile)!=1){
		fclose(tarFile);
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
		fwrite(headers[i].name+j,1,1,tarFile);
		//Escribimos el tamanyo
		if(fwrite(&headers[i].size,sizeof(unsigned int),1,tarFile)!=1) {
			fclose(tarFile);
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

		//Leemos la cabecera y en caso de error terminamos la ejecucion
		stHeaderEntry *a = readHeader(tarfile, &nFiles); 
		
		
		if(a == NULL)
			return EXIT_FAILURE;
		// Leemos cada archivo senyalado por la cabecera y lo copiamos el fichero con nombre tarName.
		// SI ocurre un error terminamos el programa
		for(int i = 0; i < nFiles; i++){
			FILE* destinationFile;
			if((destinationFile= fopen(a[i].name, "wb"))==NULL || copynFile(tarfile, destinationFile, a[i].size)==-1 || fclose(destinationFile)==EOF){
				free(a);
				return EXIT_FAILURE;
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

/** Extract info files stored in a tarball archive
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
infoTar(char tarName[])
{
	FILE * file;
	file = fopen(tarName, "rb");
	int n;
	stHeaderEntry *headers = readHeader(file, &n);
	if (headers == NULL)
		return EXIT_FAILURE;
	for (int i = 0; i< n; i++){
		fprintf(stdout,"[%d] : file %s, size %d Bytes\n",i,headers[i].name,headers[i].size);
		free(headers[i].name);
	}
	fclose(file);
	free(headers);
	return EXIT_SUCCESS;
}

/** removeFileTar a tarball archive 
 *
 * nfiles: number of mtar files
 * filenames: array with the path names of the mtar files
 * tarname: name of the tarball archive
 * 
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE. 
 * (macros defined in stdlib.h).
 *
 */
int
removeFileTar(int nFiles, char *fileNames[], char tarName[])
{
	
	FILE* tarFileOrigin;
	FILE* tarFileDestiny;
	tarFileOrigin = fopen(tarName, "rb");
	tarFileDestiny = fopen(tarName, "rb+");
	int n , nErased = 0;
	unsigned int newN;
	
	stHeaderEntry *headers = readHeader(tarFileOrigin, &n);

	fseek(tarFileOrigin,sizeof(unsigned int), SEEK_SET);
	for(int i = 0; i< n; i++){
		for(int j = 0; j<nFiles; j++){
			if(strcmp(fileNames[j], headers[i].name)==0){
				nErased++;
			}
		}
		fseek(tarFileOrigin,strlen(headers[i].name)+1+sizeof(unsigned int), SEEK_CUR);
	}
	
	newN = n -nErased;
	stHeaderEntry* newHeaders = (stHeaderEntry*)malloc((sizeof(stHeaderEntry))*newN);
	int k =0;
	fwrite(&newN, sizeof(unsigned int), 1, tarFileDestiny);
	for(int i = 0; i< n; i++){
		int j = 0;
		while(j<nFiles && strcmp(fileNames[j], headers[i].name)!=0){
			j++;
		}
		if(j==nFiles){
			newHeaders[k].name = headers[i].name;
			newHeaders[k].size = headers[i].size;
			int h = 0;
			do{
				fwrite(newHeaders[k].name+h,1,1,tarFileDestiny);
				h++;
			}while(newHeaders[k].name[h]!='\0');
			fwrite(newHeaders[k].name+h,1,1,tarFileDestiny);
			fwrite(&newHeaders[k].size,sizeof(unsigned int),1,tarFileDestiny);
			k++;
		}

	}
	//Escribimos el cuerpo.
	
	k = 0;
	for(int i = 0; i<n; i++){
		if(strcmp(newHeaders[k].name, headers[i].name)!=0){
			fseek(tarFileOrigin,(int)headers[i].size, SEEK_CUR);
		} else {
			char* s;
			s = (char*)malloc(headers[i].size);
				fread(s,1,headers[i].size,tarFileOrigin);
				fwrite(s,1,headers[i].size, tarFileDestiny);

			k++;
		}	
	}
	
	for(int i = 0; i< n; i++)
		free(headers[i].name);
	free(headers);
	for(int i = 0; i< newN; i++)
		free(newHeaders[i].name);
	free(newHeaders);
	fclose(tarFileOrigin);
	fclose(tarFileDestiny);

	return EXIT_SUCCESS;
}
