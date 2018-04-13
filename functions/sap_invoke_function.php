<?php

/**
 * Invokes a remote enabled function module on the backend and returns it's export parameters.
 *
 * @param string     $name    The remote function name to invoke
 * @param array|null $imports import parameters
 * @param bool|null  $rtrim   right-trim string fields, if null the global .ini setting will be used
 *
 * @return array export/changing/table parameters for this function
 *
 * @throws InvalidArgumentException if $connection is not a valid SAP Connection resource
 * @throws LogicException           if no connection available, or function's description has not been fetched
 *                                  through a Sap object
 * @throws SapException             RFC raised exception or other error occured
 */
function sap_invoke_function($name, $connection, array $imports = [], $rtrim = null) {}