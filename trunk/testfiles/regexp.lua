<?lucie
	
r = re.compile('@^(([A-Z][a-z]+)*)(Bar.*)$@x');
var_dump( r:exec("TestFoobarBarbaz"), r:exec("test"), r:exec("T") );

?>
