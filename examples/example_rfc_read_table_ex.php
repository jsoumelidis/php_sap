<?php
/* Our "custom" object */
class T100Row {}

/*
 * SapRfcReadTable is a special SapFunction within the php_sap extension.
 */
class MyRfcReadTable extends SapRfcReadTable
{
	public function __construct($arg)
	{
		echo 'Doing some construct staff with ', $arg, PHP_EOL;
	}
	
	protected function mapToObjects($rows)
	{
		$arr = array();
		
		foreach ($rows as $row)
		{
			$obj = new T100Row;
			
			foreach ($row as $field => $value) {
				$obj->$field = $value;
			}
			
			$arr[] = $obj;
		}
		
		return $arr;
	}
	
	public function select($fields, $table, $where = array(), $rowcount = 0, $offset = 0)
	{
		return $this->mapToObjects(
			parent::select($fields, $table, $where, $rowcount, $offset)
		);
	}
}

$sapConfig = include __DIR__ . DIRECTORY_SEPARATOR . 'sap_config.php';

try
{
	$sap = new Sap($sapConfig);
	
	/*
	 * Option 1: Fetch RFC_READ_TABLE function into a previously
	 * instatiated instance of 'MyRfcReadTable' class
	 */
	
	#$myRfcReadTable = new MyRfcReadTable('Argument #1');
	#$sap->fetchFunction($myRfcReadTable);
	
	
	/*
	 * Option 2: Fetch RFC_READ_TABLE function, returning a new instance
	 * of 'MyRfcReadTable' class with constructor arguments
	 */
	$myRfcReadTable = $sap->fetchFunction('RFC_READ_TABLE', 'MyRfcReadTable', array('Argument #1'));
	
	/* Reads all fields for the first 5 rows of table 'T100' with field SPRSL = 'D' */
	$data = $myRfcReadTable->select('*', 'T100', array('SPRSL' => 'D'), 5);
}
catch (SapException $se)
{
	echo 'SapException ::', $se->key, ':: ', $se->getMessage(), PHP_EOL;
	exit(1);
}

print_r($data);