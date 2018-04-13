<?php

/**
 * Creates a connection to a SAP backend.
 * For documentation regarding logon parameters see the demo sapnwrfc.ini
 * provided by your SAP Netweaver RFC SDK (available from SAP Marketplace).
 *
 * @param array $logonParameters
 *
 * @return resource
 *
 * @throws InvalidArgumentException If empty array provided
 * @throws SapConnectionException   For invalid logon parameters or other error occured
 */
function sap_connect(array $logonParameters) {}