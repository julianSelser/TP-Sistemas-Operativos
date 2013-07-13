for ARCHIVO in `ls *.conf`
do
	sed -i 's/Recovery=Off/Recovery=On/g' $ARCHIVO
	echo Se actualizo el archivo $ARCHIVO
done

echo Recovery ON!


