rm -f mcts_megabonk.jar
javac -cp Ludii-1.3.14.jar mcts_megabonk.java
mv mcts_megabonk.class bk/
jar cf mcts_megabonk.jar bk/mcts_megabonk.class
