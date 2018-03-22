--TEST--
sap_invoke_function() error behavior
--FILE--
<?php
/* test accepts string as 1st argument */
try { $r = sap_invoke_function([]); }
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}

/* test accepts resource as 2nd argument */
try { $r = sap_invoke_function('DUMMY_RFC', []); }
catch (TypeError $e) {
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
catch (TypeError $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
Argument 1 passed to sap_invoke_function() must be of the type string, array given
Argument 2 passed to sap_invoke_function() must be of the type resource, array given
Argument 2 passed to sap_invoke_function() must be a resource of type SAP Connection
Argument 3 passed to sap_invoke_function() must be of the type array, string given