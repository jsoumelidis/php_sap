# php_sap
PHP (>=5.3, 7) extension for calling SAP Remote Function Modules (SAP with Unicode)

Builds available for PHP version 5.3, 5.4, 5.5, 5.6 (x86, ts, nts) and PHP version 7.0 (x86, x64, ts, nts).
Not compiled (yet) on other platforms

You can use the provided Microsoft Visual Studio 2015 solution under the vs/ directory to compile the extension by yourself under Windows.
You just have to edit the appropriate settings in Property Manager for your system.

Read classes description in classes/ directory on how to use the extension within your PHP scripts.
Do not include those files as they exist only for documentation purposes. Real classes are already defined within the extension.

Then, read/test the scripts in examples/ directory

Thanks to piersharding (https://github.com/piersharding/php-sapnwrfc) for showing the way...
You can also advise his project page for instructions on setting up your enviroment for extension usage/compilation.
