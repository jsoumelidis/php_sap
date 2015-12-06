<?php

class PartnerRepository extends SapFunction
{
	#My system specific function, don't bother
	const RFC_FUNCTION_NAME = 'Z_RFC_PARTNER_SELECT';
	
	public function getName()
	{
		return self::RFC_FUNCTION_NAME;
	}
	
	public function getPartners($vatnum)
	{
		$result = $this->__invoke(array(
			'I_PARS' => array(
				'STCD2' => $vatnum,
			),
		));
		
		return $result['ET_PARTNER_LIST'];
	}
}

$sapConfig = include __DIR__ . DIRECTORY_SEPARATOR . 'sap_config.php';

$sap = new Sap($sapConfig);

$partnerRepository = new PartnerRepository();

$sap->fetchFunction($partnerRepository);

$partners = $partnerRepository->getPartners('123456789');

print_r($partners);
	