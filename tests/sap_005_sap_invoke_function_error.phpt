--TEST--
sap_invoke_function() error behavior
--FILE--
<?php
$config = include 'config.inc';

$rsrc = tmpfile();

/* test accepts string as 1st argument */
try { $r = sap_invoke_function([], $rsrc); }
catch (InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}

/* test accepts resource as 2nd argument */
try { $r = sap_invoke_function('DUMMY_RFC', []); }
catch (InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}

/* test accepts only resource of type "SAP Connection" as 2nd argument */
$rsrc = tmpfile();
try { $r = sap_invoke_function('DUMMY_RFC', $rsrc); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test accepts array as 3rd argument */
try { $r = sap_invoke_function('DUMMY_RFC', $rsrc, 'invalid'); }
catch (InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}

/* test accepts boolean as 4th argument */
try { $r = sap_invoke_function('DUMMY_RFC', $rsrc, [], []); }
catch (InvalidArgumentException $e) {
    echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
sap_invoke_function() expects parameter 1 to be string, array given
sap_invoke_function() expects parameter 2 to be resource, array given
sap_invoke_function() expects parameter 2 to be a resource of type SAP Connection
Argument 3 passed to sap_invoke_function() must be of the type array, string given
sap_invoke_function() expects parameter 4 to be boolean, array given