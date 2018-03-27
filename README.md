SAP Netweaver Remote Function Calls support for PHP
===================

This extension is aimed to provide developers a convenient way to invoke remote-enabled ABAP functions
and retrieve their results from PHP code.

A short example usage:

```php
	<?php
	$connection = new Sap($logonParametersArray);
	
	/** @var array $result */
	$result = $connection->call('MY_REMOTE_ENABLED_ABAP_FUNCTION', [
	    'I_VAR' => 5
	]);
	
	//or using the procedural way...
	$connection = sap_connect($logonParametersArray);
	
	/** @var array $result */
	$result = sap_invoke_function('MY_REMOTE_ENABLED_ABAP_FUNCTION', $connection, [
	    'I_VAR' => 5
	]);
```

For more examples visit the `examples` directory of this repository.

# Installation

Download and extract the SAP Netweaver RFC SDK package for your platform architecture
(available from SAP Marketplace).

### Windows

1. Make sure that .DLL files located in *path\to\nwrfcsdk\lib* can be discovered by PHP
	using one of the following methods:
	* Copy all .DLLs to your *%SystemDrive%\Windows* directory
	* Append *path\to\nwrfcsdk\lib* to your system's PATH environment variable
	* Copy all .DLLs in the same folder as your php executable
2. Install the appropriate Visual Studio version for your PHP version.
	* For PHP 5.6.x use VS2012 (VC11)
	* For PHP 7.0.x and 7.1.x use VS2015 (VC14)
	* For PHP 7.2.x use VS2017 (VC15)
	> Express or Community versions of Visual Studio are just fine
3. Download and extract the PHP devel package matching your PHP version from [here](https://windows.php.net/downloads/releases/)
	> Note that you should use the same platform (x86 or x64) and thread safety setting (TS or NTS)
	with your installed PHP.
4. Clone the appropriate branch of this repository for your PHP version
	(5.6-dev, 7.0-dev, 7.1-dev, 7.2-dev), or download a packaged release.
	```
	git clone https://github.com/jsoumelidis/php_sap.git X.X-dev
	```
5. On a VS20XX-xYY command line window *cd* to the cloned (or downloaded) php_sap directory
	(*XX* is the version and *YY* is the platform, like VS2015-x86), e.g.
	```
	cd C:\php_sap
	```
	> Note that you should use the same VS Command Line platform with your installed PHP (x86 or x64).  
6. Execute `Path\To\PHP\Devel\phpize`
7. Assuming you have extracted SAP Netweaver RFC SDK to *C:\nwrfcsdk* and PHP is installed
	in directory *C:\php\7.0-x64-nts* run the following command
	```
	configure --enable-sap --with-sap-nwrfcsdk="C:\nwrfcsdk" --with-prefix="C:\php\7.0-x64-nts"
	```
8. Run `nmake` to build the extension
9. Test the extension using `nmake test` and verify output
	> Testing against a SAP R/3 backend requires valid configuration. Edit the *config.inc* file
	located under *tests* directory of this repository and provide your system values.
	If you wish to use a *sapnwrfc.ini* file instead, place it in the root directory of the
	repository and use only the *dest* key in config.inc. For details on what system values to provide
	and/or how to create a sapnwrfc.ini file, check the demo .ini under */path/to/nwrfcsdk/demo*
	directory.
10. Use `nmake install` to copy the extension's dll file to your PHP installation's `ext` directory.
11. Edit your `php.ini` file to enable the extension adding `extension=php_sap.dll`
	(or just `extension=sap` for php7.2)
12. Verify extension is installed and loaded using
	 ```
	 C:\php\7.0-x64-nts\php -m
	 ```
	 You should be able to find the `sap` line in the output.

### Linux

1. Extract SAP Netweaver RFC SDK. This guide assumes the extraction path is `/usr/sap/nwrfcsdk`, but
	you can place the extracted nwrfcsdk folder wherever fits you best.
	> Note that the RFC libraries located in */path/to/nwrfcsdk/lib* must be cached by ldconfig.
	One way to do that is to create a *nwrfcsdk.conf* file in */etc/ld.so.conf.d/* directory
	that contain the path to the libraries and then refresh the cache by executing `sudo ldconfig`
2. Install PHP devel package matching your PHP version
3. Clone the appropriate branch of this repository for your PHP version
   	(5.6-dev, 7.0-dev, 7.1-dev, 7.2-dev), or download a packaged release.
   	```
   	git clone https://github.com/jsoumelidis/php_sap.git X.X-dev
   	```
4. *cd* to the cloned (or downloaded) php_sap directory:
   	```
   	cd /home/john/php_sap
   	```
5. Execute `phpize`
6. Run:
	```
	./configure --enable-sap --with-sap-nwrfcsdk=/usr/sap/nwrfcsdk
	```
7. Build the extension using `make`
8. Test with `make test` and verify output
	> Testing against a SAP R/3 backend requires valid configuration. Edit the *config.inc* file
	located under *tests* directory of this repository and provide your system values.
	If you wish to use a *sapnwrfc.ini* file instead, place it in the root directory of the
	repository and use only the *dest* key in config.inc. For details on what system values to provide
	and/or how to create a sapnwrfc.ini file, check the demo .ini under */path/to/nwrfcsdk/demo*
	directory.
9. Install the extension using `sudo make install`
10. Create a `sap.ini` file in your `/etc/php.d` directory with the following content:
	```
	extension=sap.so
	```
11. Verify extension is installed and loaded using `php -m`. Locate `sap` in the output


Usage
=====

Code examples demonstrating the usage of the extension is located under the *examples*
subdirectory of this repository. For a full documentation of the Classes exposed take a look 
in *classes* subdirectory of this repository.

Configuration
=============

*php_sap* extension exposes the following php.ini configuration:


```sap.rtrim_export_strings```: *boolean* Default: *Off*

Data containing characters (strings) are always transferred from the backend 
using the full length of their container (as defined in SE37). For example, a parameter of type 
CHAR(20) containing the word *"Hello"* will be transfered and exported as
*"Hello&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"*.
This setting enables/disables automatic right-trimming for those cases.
> This behavior can be overridden explicitly on every function invocation (see documentation).

```sap.sanwrfc_ini_dir```: *string* Default: *empty*

The directory (absolute or relative to current) to look for a sapnwrfc.ini file.
If empty, the current directory is used.

```sap.trace_dir```: *string* Default: *empty*

The directory (absolute or relative to current) to place RFC trace files.
If empty, the current directory is used.

IDEs
====

In order to provide autocomplete support for IDEs, you can require jsoumelidis/ext-sap in your
project's composer.json development dependencies
```
composer require --dev jsoumelidis/ext-sap X.X-dev
```
Replace X.X with your php version (e.g. 7.1-dev)