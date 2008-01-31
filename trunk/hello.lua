#!./src/lucie
io.write("Hello world!\n");
core.foobar(true, "blub", 23);
table = core.foobar2();
for k,v in ipairs(table) do
	io.write(k .. ": " .. v .. "\n");
end
foobar();
