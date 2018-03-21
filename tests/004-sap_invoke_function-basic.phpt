--TEST--
sap_invoke_function basic behavior
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
$r = fopen("php://input", "r");
try { $r = sap_invoke_function('DUMMY_RFC', $r); }
catch (InvalidArgumentException $e) {
	echo $e->getMessage(), PHP_EOL;
}
finally {
	fclose($r);
}
?>
--EXPECT--
Argument 1 passed to sap_invoke_function() must be of the type string, array given
Argument 2 passed to sap_invoke_function() must be of the type resource, array given
Invalid connection. A resource of type SAP Connection is required