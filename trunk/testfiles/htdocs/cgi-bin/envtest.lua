#!/home/jakob/dev/c/lucie/trunk/src/lucie
Content-type: text/html

<html>
	<head><title>Environment test</title></head>
	<body>
		<pre><?lucie var_dump( _SERVER, _GET, _POST, _HEADER ); ?></pre>
		<form name="test" action="envtest.lua?getval1=foobar&getval2=baz" method="post">
			<input type="text" name="input1" />
			<input type="text" name="input2" />
			<input type="submit" name="submit" value="Submit" />
		</form>
	</body>
</html>
