#!./src/lucie
var_dump({
		["test"] = "foobar",
		[1] = "baz",
		[2] = {
				["blub"] = "blab"
		},
		["zwölfdreizehn"] = {
			[0] = {
				["rekursion pur"] = "oder etwa nicht?",
				[1] = "und noch einen mehr ;)"
			}
		},
		[io.write] = 4
	});
var_dump( "string", 423, 23.5, io.write, nil );
