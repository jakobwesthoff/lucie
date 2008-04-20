<?lucie
	
r = re.compile('@[A-Z][a-z]\\+@');
var_dump( r:exec("Test"), r:exec("test"), r:exec("T") );

?>
