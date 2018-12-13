#!/.bin/bash

askname () {
    echo Filename: 
    read filename
}
#Preguntamos por un fichero valido
askname
while [ ! -f $filename ]; do
    echo Error: $filename is an invalid file. 
    askname
done
#Si no existe la carpeta la creamos
if [ ! -d resultados ];
then
    mkdir resultados 
fi
#Obtenemos el nombre y el fichero de salida.
outfile=${filename##*/}
name=${outfile%.*}
outfile=resultados/$name.log
let i=0;
#comprobamos que no exista ya y si existe le cambiamos el nombre
while [ -e $outfile ]; do
    let ++i
    outfile=resultados/$name'('$i')'.log
done
#hacemos la grafica gantt
./sim $filename $outfile
cd ../gantt
./generate_gantt_chart ../sim/$outfile
cd ../sim