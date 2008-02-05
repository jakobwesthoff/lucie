#!./src/lucie
var_dump(
	urldecode("http://www.google.de?ich=bin+ganz+toll"),
	urldecode("http://www.google.de?ich=bin%20ganz%20toll")
);
var_dump( readini("/home/jakob/dev/c/lucie/trunk/test.ini"));
