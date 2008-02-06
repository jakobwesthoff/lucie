#!/home/jakob/dev/c/lucie/trunk/src/lucie
print("Content-type: text/html\n\n");

print("<html>");
print("<head><title>Environment test</title></head>");
print("<body>");
print("<pre>");
var_dump( _SERVER, _GET, _POST, _HEADER );
print("</pre>");
print("<form name=\"test\" action=\"envtest.lua?getval1=foobar&getval2=baz\" method=\"post\">");
print("    <input type=\"text\" name=\"input1\" />");
print("    <input type=\"text\" name=\"input2\" />");
print("    <input type=\"submit\" name=\"submit\" value=\"Submit\" />");
print("</form>");
print("</body>");
print("</html>");
