/* $Id: README.txt,v 1.1.4.4 2008/07/18 20:02:24 alexua Exp $ */

/**********************/
 Embedded Media Import
/**********************/

****PLEASE NOTE: EMIMPORT IS DEPRECATED IN DRUPAL 6 IN FAVOR OF FEEDAPI INTEGRATION********

Author: Aaron Winborn
Development Began 2007-10-04

Requires: Drupal 5, Embedded Media Field

Using this module will allow media sets (such as flickr photosets) to
be imported as individual nodes, including titles, bodies (using item
descriptions), and tags (which may be overridden). 

Currently supports: Flickr.

Go to /admin/content/emfield/import to change the settings for imports.
Enable the providers you wish to allow. Make sure to set the proper
access settings at /admin/user/access. When you have a content type
that allows importing, you will be able to import sets at /emimport.

WARNING: For now, it's best if you only use types with a single
Embedded Media Field. This hasn't been tested with multiple fields.
The author has tried to put stuff in place to allow this, but the
UI requirements were more than he could wrap his brain around at the 
time he was writing the module. However, if you do have multiple 
Embedded Media Fields, you can turn off support for an individual 
field (at that field's settings page), which will be safe.

BEST USE: A content type with a single media field, and at the least,
a title and body, and an optional vocabulary with free-tagging
capability. You may have other fields as well, so long as they are not
Embedded Media Fields.

TODO: Allow description to be inserted into a field other than body.
TODO: Allow sets to be imported into a single node (which has a field
allowing multiple values).
TODO: Allow support for other providers, such as YouTube.

Questions can be directed to winborn at advomatic dot com

