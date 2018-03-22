--TEST--
sap_connect() connectivity
--SKIPIF--
<?php
$config = include 'config.inc';

if (!$config) {
	print 'skip connection configuration not available';
}
?>
--FILE--
<?php
/* test throws SapConnectionException on invalid logon parameters */
try { $c = sap_connect(['ashost' => 'INVALID_HOST']); }
catch (SapConnectionException $e) {
	echo get_class($e), PHP_EOL;
}

$config = include 'config.inc';

/* test returns resource on successful connection */
$c = sap_connect($config);
var_dump($c);


?>
--EXPECTF--
SapConnectionException
resource(%d) of type (SAP Connection)