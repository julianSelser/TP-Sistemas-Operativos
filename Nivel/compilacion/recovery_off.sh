for ARCHIVO in `ls *.conf`
do
	sed -i 's/Recovery=On/Recovery=Off/g' $ARCHIVO
	echo Se actualizo el archivo $ARCHIVO
done

echo Recovery OFF!


