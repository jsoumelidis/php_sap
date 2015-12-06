<?php
$sapConfig = include __DIR__ . DIRECTORY_SEPARATOR . 'sap_config.php';

/* Call SAP Remote Functions using the old-fashion PHP style */
if ($connection = sap_connect($sapConfig))
{
	$result = sap_invoke_function('BAPI_USER_GET_DETAIL', $connection, array('USERNAME' => $sapConfig['user']));
	
	if ($result) {
		var_dump($result);
		exit(0);
	}
}

var_dump(sap_last_error());
exit(1);

