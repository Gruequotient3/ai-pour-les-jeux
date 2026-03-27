rm -f MinMaxGSBSCLME.jar ubfmMegabonk.jar
javac -cp Ludii-1.3.14.jar MinMaxGSBSCLME.java
javac -cp Ludii-1.3.14.jar ubfmMegabonk.java
mv MinMaxGSBSCLME.class ubfmMegabonk.class bk/
jar cf MinMaxGSBSCLME.jar bk/MinMaxGSBSCLME.class
jar cf ubfmMegabonk.jar bk/ubfmMegabonk.class
