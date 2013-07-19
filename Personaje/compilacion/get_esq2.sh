for ARCHIVO in `ls *.conf`
do
	rm $ARCHIVO
done

for ARCHIVO in Goomba.conf Luigi.conf Mario.conf Tortuga.conf
do
	cp Esquema2/$ARCHIVO $ARCHIVO
done	

echo Se trajo el esquema 2
