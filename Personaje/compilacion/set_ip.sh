echo "Ingrese la IP del orquestador: "
read IP

for ARCHIVO in `ls *.conf`
do
	echo \orquestador=$IP:10000 >> $ARCHIVO
	echo Se actualizo el archivo $ARCHIVO
done	


