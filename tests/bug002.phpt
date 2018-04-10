--TEST--
SapException::get*() methods not increasing refcount
--FILE--
<?php
try { new Sap(['invalid' => 'invalid']); }
catch (SapConnectionException $e) {
	$key = $e->getMessageKey(); 
	debug_zval_dump($key); //+1 refcount for property, debug_zval_dump() adds +1 refcount while dumping (total refcount = 2)
	
	$other = $e->getMessageKey();
	debug_zval_dump($other);
}
?>
--EXPECTF--
string(21) "%s" refcount(2)
string(21) "%s" refcount(2)