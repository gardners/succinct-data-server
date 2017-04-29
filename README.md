# succinct-data-server
Succinct Data Server

The files in files/www need to be visible from the webserver referenced by the
Succinct Data application for Android.

The other files need to be appropriately placed on the server in the home directory
of the user whom the server runs as.

A cronjob is then needed to poll TextMagic and then push the results to Magpi,
using the scripts provided in files/bin
