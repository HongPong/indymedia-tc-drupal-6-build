
********************************************************************
                     D R U P A L    M O D U L E
********************************************************************

Name: taxonomy_role
Author: der <dreed10 -at- gmail -dot- com>
Drupal: 5.x

********************************************************************
DESCRIPTION:

The purpose of this module is to provide a way to restrict which 
vocabularies show up on the node forms. This allows you to do 
things like setup vocabularies for administrator-only use or for
any other purpose where you want only certain roles to see or set
vocabulary terms on the nodes.

IMPORTANT! – Once this module installed, you MUST visit the 
access control admin page to set the access permissions for 
your existing vocabularies. If you don’t do this step then your 
vocabularies wont be visible to anyone but the original admin account.

********************************************************************
INSTALLATION:

1. Copy the module's directory and files to your site's modules directory.

2. Enable the taxonomy_role module in 'admin/build/modules'.

3. Set access permissions for you taxonomy vocabularies in admin/user/access.

4. Optional: Choose whether to hide or disable denied vocabularies in
   admin/settings/taxonomy_role.

********************************************************************
