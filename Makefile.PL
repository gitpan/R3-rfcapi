use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

# select libs depending on UNIX platform
$ljbs='-lrfc';
$ljbs='-lrfc' if $^O eq 'aix';
$ljbs='-lrfc -lsocket -lnsl -ldl -lm -lw' if $^O eq 'solaris';
$ljbs='-lrfc -lsocket -lnsl -ldl -lm -lw' if $^O eq 'sunos';
$ljbs='-lrfc -lcl -ldld' if $^O eq 'hpux';
$ljbs='-lrfc -ldl -lm -lpthread' if $^O eq "linux";

if ($^O ne "MSWin32") # Makefile for UNIX
{
	if (-e '/usr/sap/rfcsdk')
	{
		WriteMakefile(
		'NAME'	=> 'R3::rfcapi',
		'VERSION_FROM' => 'rfcapi.pm', # finds $VERSION
		'LIBS'	=> [$ljbs],
		'LINKTYPE' => 'static',
		'DEFINE'	=> '',  
		'OBJECT' => 'rfcapi.o r3rfc.o r3conn.o ' .
			'r3func.o r3itab.o r3errh.o',
		'INC'	=> '-I/usr/sap/rfcsdk/include', 
		'XSOPT' => '-prototypes',
		);
	}
	else
	{
		print "RFCSDK not found at /usr/sap/rfcsdk.\n";
		print "Install RFCSDK or change Makefile.PL!\n";
	}
}
else # Makefile for Windows NT
{
	if (-e 'C:\usr\sap\rfcsdk')
	{
		WriteMakefile(
		'NAME'	=> 'R3::rfcapi',
		'VERSION_FROM' => 'rfcapi.pm', # finds $VERSION
		'DEFINE'	=> '',  
		'OBJECT' => 'rfcapi.o r3rfc.o r3conn.o r3func.o ' .
			'r3itab.o r3errh.o C:\usr\sap\rfcsdk\lib\librfc32.lib',
		'INC'	=> '/IC:\usr\sap\rfcsdk\include', 
		'XSOPT' => '-prototypes',
	);
	}
	else
	{
		print "RFCSDK not found at C:\\usr\\sap\\rfcsdk.\n";
		print "Install RFCSDK or change Makefile.PL!\n";
	}
}



