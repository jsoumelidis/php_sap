--TEST--
SapFunction::getName() error behavior
--FILE--
<?php

try { (new SapFunction)->getName(); }
catch (LogicException $e) {
	echo $e->getMessage(), PHP_EOL;
}

?>
--EXPECT--
Function's description has not been fetched