ROBOT_DIR=$HOME/robot
ORIG_DIR=$ROBOT_DIR
DEST_DIR=$ROBOT_DIR/pics

PGM_FILE=$ORIG_DIR/saida.pgm 

NOW=`date +"%Y-%m-%d_%H-%M-%S"`
PNG_FILE=$DEST_DIR/$NOW.png

PREFIX="walle:"

echo $PREFIX Converting $PGM_FILE to $PNG_FILE...
pnmtopng $PGM_FILE > $PNG_FILE
echo Done.
 
echo $PREFIX Deleting $PGM_FILE...
# rm $PGM_FILE
echo Done.
