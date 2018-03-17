--TEST--
SapFunction tests
--FILE--
<?php
$logon = include 'config.inc';
$sap = new Sap($logon);

$func = $sap->fetchFunction('RFC_PING');

echo $func->getName(), PHP_EOL;
echo (string)$func, PHP_EOL;

$result = $func();

echo gettype($result), PHP_EOL;
echo count($result), PHP_EOL;
?>
--EXPECT--
RFC_PING
RFC_PING
array
0