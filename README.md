# php_sap
PHP (>=5.3, 7) extension for calling SAP Remote Function Modules (SAP with Unicode)

Builds available for PHP version 5.3, 5.4, 5.5, 5.6, 7.0 (both ts and nts) on windows (x86)
not tested with other versions
not compiled (yet) on other platforms

You can use the provided Microsoft Visual Studio 2015 solution under the vs/ directory to compile the extension by yourself under Windows.
You just have to edit the appropriate settings in Property Manager for the PHP version you are interested.

Read classes description in classes/ directory on how to use the extension within your PHP scripts.
Do not include those files as they exist for documentation purposes.
Real classes are already defined within the extension.

Then, read/test the scripts in examples/ directory

Thanks to piersharding (https://github.com/piersharding/php-sapnwrfc) for showing the way...
You can use the above page for setting up your enviroment for extension usage/compilation.
