for ARCHIVO in `ls *.conf`
do
	rm $ARCHIVO
done

for ARCHIVO in Goomba.conf Hongo.conf Luigi.conf Mario.conf Tortuga.conf
do
	cp Esquema1/$ARCHIVO $ARCHIVO
done	

echo Se trajo el esquema 1
