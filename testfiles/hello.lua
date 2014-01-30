#!./src/lucie
<?lucie
--var_dump(
--	urldecode("http://www.google.de?ich=bin+ganz+toll"),
--	urldecode("http://www.google.de?ich=bin%20ganz%20toll")
--);
var_dump( readini("testfiles/test.ini"));
io.stdout.write("Blub");
?>
