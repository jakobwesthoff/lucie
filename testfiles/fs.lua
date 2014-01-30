<?lucie

filename = "/etc/passwd";
var_dump(
    file.exists( filename ),
	file.is_file( filename ),
	file.is_dir( filename ),
    file.is_link( filename ),
	file.is_readable( filename ),
	file.is_writable( filename ),
	file.is_executable( filename ),
    file.mtime( filename ),
    file.atime( filename ),
    file.ctime( filename ),
    file.owner( filename ),
    file.group( filename ),
	file.size( filename ),
	file.basename( filename ),
	file.dirname( filename )
);

?>
