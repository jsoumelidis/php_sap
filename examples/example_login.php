<?php
$sapConfig = include __DIR__ . DIRECTORY_SEPARATOR . 'sap_config.php';

class SapRfcUserLogin extends SapFunction
{
	const SUCCESS = 0;
	const FAILURE = 1;
	
	protected $reason;
	
	public function authenticate($username, $password)
	{
		try {
			$this->__invoke(array(
				'BNAME' => $username,
				'PASSWORD' => $password,
			));
		}
		catch (SapException $se)
		{
			switch ($se->key)
			{
				/* Possible exception keys */
				case 'USER_LOCKED':
				case 'USER_NOT_ACTIVE':
				case 'PASSWORD_EXPIRED':
				case 'WRONG_PASSWORD':
				case 'NO_CHECK_FOR_THIS_USER':
				case 'PASSWORD_ATTEMPTS_LIMITED':
				case 'INTERNAL_ERROR':
				default:
					$this->reason = $se->key;
					return self::FAILURE;
			}
		}
		
		return self::SUCCESS;
	}
	
	public function getReason()
	{
		return $this->reason;
	}
}

if (isset($argc, $argv) && $argc > 2) {
	/* get username/password from command line arguments */
	$username = $argv[1];
	$password = $argv[2];
}
else {
	/* use Sap logon parameters to test this example */
	$username = $sapConfig['user'];
	$password = $sapConfig['passwd'];
}

try
{
	$sap = new Sap($sapConfig);
	
	$loginCheckFunction = $sap->fetchFunction('SUSR_LOGIN_CHECK_RFC', 'SapRfcUserLogin');
}
catch (SapException $se) {
	echo 'System not available (', $se->key, ')', PHP_EOL, $se->getMessage(), PHP_EOL;
	exit(1);
}

echo "Authenticating $username... ";

switch ($loginCheckFunction->authenticate($username, $password))
{
	case $loginCheckFunction::SUCCESS: echo 'SUCCESS!', PHP_EOL; break;
	default:
	{
		switch ($loginCheckFunction->getReason())
		{
			case 'PASSWORD_EXPIRED': /* You could use the SUSR_USER_CHANGE_PASSWORD_RFC function for password change here */
			default: echo 'FAILURE (', $loginCheckFunction->getReason(), ')', PHP_EOL;
		}
		
		break;
	}
}
