<?php

class SapFunction
{
	/**
	 * Get function's name
	 *
	 * @return string
	 */
	public function getName() {}

	/**
	 * Invokes the function module on the backend and returns it's export parameters.
	 * In order to use this functionality, the SapFunction object must be fetched
	 * through a Sap object connected to a R/3 system.
     *
	 * The __invoke method makes the SapFunction object act as a 'callable' object, so
	 * $exports = $sapFunction->__invoke($imports) is the same as $exports = $sapFunction($imports).
	 *
	 * @param array $imports    import parameters
	 * @param bool|null $rtrim  right-trim string fields, if null the global .ini setting will be used
	 *
	 * @return array export/changing/table parameters for this function
	 *
     * @throws LogicException   if no connection available, or function's description has not been fetched
     *                          through a Sap object
	 * @throws SapException     RFC raised exception or other error occured
	 */
	public function __invoke(array $imports = null, $rtrim = null) {}

	/**
	 * Sets a parameter active/inactive.
     * If a parameter is set "inactive" it's data will not be transfered, neither on import nor on export.
	 * In case you want the default value applied for an import parameter (as defined in SE37)
	 * you should not set it's state or value at all, just leave it default.
	 * You could use this feature in case you are not interested in data of particular exporting tables for
     * reducing network traffic, speed up execution etc.
	 *
	 * @param string    $parameter  Parameter's name
	 * @param bool      $active     Activate/Deactivate parameter
	 *
	 * @return void
     *
     * @throws LogicException           if function's description has not been fetched through a Sap object
     * @throws UnexpectedValueException if parameter not exists in functions description
	 */
	public function setActive($parameter, $active = true) {}
	
	/**
	 * Retrieve all parameter names for this function module
	 *
	 * @return string[] Function's parameters as defined in SE37
	 */
	public function getParameters() {}
	
	/**
	 * Get the DDIC name of a TABLE or STRUCTURE parameter
	 * Returns false if parameter not found or is not a TABLE|STRUCTURE
	 *
	 * @param string $parameter Parameter's name
	 *
	 * @return string Parameter's DDIC name
     *
     * @throws LogicException           if function's description has not been fetched through a Sap object
     * @throws UnexpectedValueException if parameter not exists in functions description
     * @throws LogicException           if parameter is not of type STRUCTURE or TABLE
     * @throws SapException             other error occurred
	 */
	public function getTypeName($parameter) {}
}