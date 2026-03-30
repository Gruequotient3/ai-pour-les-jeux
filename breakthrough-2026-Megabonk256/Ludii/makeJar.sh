rm -f MinMaxGSBSCLME.jar ubfmMegabonk.jar mctsMegabonk.jar
javac -cp Ludii-1.3.14.jar MinMaxGSBSCLME.java
javac -cp Ludii-1.3.14.jar ubfmMegabonk.java
javac -cp Ludii-1.3.14.jar mctsMegabonk.java
mv MinMaxGSBSCLME.class ubfmMegabonk.class mctsMegabonk.class bk/
jar cf MinMaxGSBSCLME.jar bk/MinMaxGSBSCLME.class
jar cf ubfmMegabonk.jar bk/ubfmMegabonk.class
jar cf mctsMegabonk.jar bk/mctsMegabonk.class
