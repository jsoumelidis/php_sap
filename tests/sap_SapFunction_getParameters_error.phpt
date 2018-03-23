--TEST--
SapFunction::getParameters() error behavior
--FILE--
<?php
try { (new SapFunction)->getParameters(); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}
?>
--EXPECT--
Function's description has not been fetched