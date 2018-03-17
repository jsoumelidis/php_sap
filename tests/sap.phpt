--TEST--
Sap tests
--FILE--
<?php
class RfcPing extends SapFunction
{
	private $arg;
	
	public function __construct($arg)
	{
		$this->arg = $arg;
	}
	
	public function getArg()
	{
		return $this->arg;
	}
}

$logon = include 'config.inc';

$sap = new Sap($logon);
echo 'Connected', PHP_EOL;

$result = $sap->call('RFC_PING');
echo gettype($result), PHP_EOL;
echo count($result), PHP_EOL;

$func = $sap->fetchFunction('RFC_PING');
echo gettype($func), PHP_EOL;
echo get_class($func), PHP_EOL;

$rfcPing = $sap->fetchFunction('RFC_PING', 'RfcPing', ['test']);
echo gettype($rfcPing), PHP_EOL;
echo get_class($rfcPing), PHP_EOL;
echo 'test' === $rfcPing->getArg() ? 'same' : 'not same', PHP_EOL;
?>
--EXPECT--
Connected
array
0
object
SapFunction
object
RfcPing
same