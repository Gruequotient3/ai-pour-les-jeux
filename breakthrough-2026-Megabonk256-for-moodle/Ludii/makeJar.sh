rm -f ubfm_megabonk.jar
javac -cp Ludii-1.3.14.jar ubfm_megabonk.java
mv ubfm_megabonk.class bk/
jar cf ubfm_megabonk.jar bk/ubfm_megabonk.class
