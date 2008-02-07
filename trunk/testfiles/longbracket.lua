<?lucie
dofile("testfiles/includetest.lua");

io.write([=====[starting immediatly after the long bracket
new line

^^ free line, newline at the end
]=====]);

io.write([=====[
Just testing brackets inside long bracket stuff [[]]
]=====]);
io.write([=====[

Just testing brackets inside long bracket stuff ]]
]=====]);

blub("mal sehen");
