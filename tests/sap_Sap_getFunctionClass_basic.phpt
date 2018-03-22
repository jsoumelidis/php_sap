--TEST--
Sap::getFunctionClass() basic behavior
--FILE--
<?php
var_dump((new Sap())->getFunctionClass());
?>
--EXPECT--
string(11) "SapFunction"