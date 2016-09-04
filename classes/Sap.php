<?php
class Sap
{
	/**
	 * Valid logon parameter keys:
	 * client, user, passwd, lang, trace, ashost, sysnr,
	 * mshost, msserv, sysid, group,
	 * snc_qop, snc_myname, snc_partnername, snc_lib
	 * mysapsso2
	 *
	 * All values should be strings, otherwise they will be converted to strings
	 *
	 * @param array $logonParameters The R/3 logon parameters
	 */
	public function __construct(array $logonParameters) {}

	/**
	 * Fetches a function module's description from the R/3 backend
	 * and returns a ready to call SapFunction object.
	 * If argument #1 is a SapFunction object, function's description will be
	 * fetched into that object. Previous function description stored will be
	 * destroyed. Object must implement the getName() method
	 * that returns the corresponding Remote Function name as string.
	 * If argument #2 is provided, fetchFunction() will return a new object of $class
	 * using $ctor_args as constructor arguments.
	 *
	 * @param string|SapFunction $function
	 * @param string $class Class derived from SapFunction
	 * @param array $ctor_args Constructor arguments, used in conjuction with $class argument
	 *
	 * @return SapFunction
	 *
	 * @throws SapException function not found or other error occurs
	 */
	public function fetchFunction($function, $class = null, array $ctor_args = null) : SapFunction {}

	/**
	 * Instant call of a Remote Function Module.
	 *
	 * @param string $function Function's name
	 * @param array $args import/changing/tables parameters
	 *
	 * @return array Function's export/changing/tables parameters
	 
	 * @throws SapException function not found or other error occurs
	 */
	public function call(string $function, array $args) : array {}
	
	/**
	 * Sets the default SapFunction class for this Sap Object when fetching remote functions.
	 *
	 * @param string $class A class derived from SapFunction
	 *
	 * @return void
	 */
	public function setFunctionClass(string $class) : Sap {}
}