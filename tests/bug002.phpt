--TEST--
SapException::get*() methods not increasing refcount
--FILE--
<?php
try { new Sap(['invalid' => 'invalid']); }
catch (SapConnectionException $e) {
	$key = $e->getMessageKey(); 
	debug_zval_dump($key); //+1 refcount for property, +1 refcount for var $key, debug_zval_dump() adds +1 refcount while dumping (total refcount = 3)
	
	$other = $e->getMessageKey();
	debug_zval_dump($other);
	
	$key = 1;
	debug_zval_dump($other);
}
?>
--EXPECTF--
string(21) "%s" refcount(3)
string(21) "%s" refcount(4)
string(21) "%s" refcount(3)