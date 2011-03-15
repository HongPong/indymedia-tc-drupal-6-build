Description
===========

This module enables you to un-/hide nodes and comments.

The concept of 'hiding' has to be understood as something in between
 - being published and shown in all sorts of places and
 - being unpublished
it shall be visible if you go to another URI, e.g. hidden/, but
not when viewing nodes and comments in the normal way or by using
the search function.

The purpose is to help transparency of the 'open editing' process of an
'open publishing' site.

Features
========

 - Articles unpublished as hidden are available to users with 'access hidden'
     privileges from under the /hidden links.
 - Hidden articles are always displayed with the reason that they are hidden,
     and custom css class.
 - The reason for hiding can be selected from a list or/and typed in.
 - The list of reasons for hiding can be set from admin.
 - All pages with /hidden have robots meta tags to ask for no indexing.
 - Filters can be set to automatically hide posted nodes and comments.
 - Filters can be obvious reporting the reason to the user, or obfuscated by
     delay.
 - Notification of hidden articles can automatically be sent out by e-mail.
     This can be individually or in batches.
 - Users with 'report as hidden' privileges can report nodes and comments that
     they think should be hidden.
 - Admin pages for nodes and comments have additional tabs to look at hidden
     and reported content and complete mass operations on them.

Installation
============
After enabling the module, set access permissions at admin/user/access:
 - 'access hidden' permission to view hidden nodes and comments.
 - 'report for hiding' permission to be able to report nodes and comments
     that should be hidden by someone who has...
 - 'mark as hidden' permission to un-/mark nodes and comments as hidden
 - 'administer hidden' permission allows changes to filters and settings
      like what reports are made.
 - 'bypass filter' for users who do not have their content checked against
     filters for editorial violations.

You probably want the role that can 'mark as hidden' to be able to 'access
hidden' as well.

Visit /admin/settings/hidden to set what actions get logged and when and if
they get e-mailed. The e-mail address can be an editorial e-mail list. The
reports can be collected together and sent out periodically (requires cron) or
can be sent immediately an action happens.

Visit /admin/content/hidden to set what default reasons can be given for
hiding a node or comment. You can also set a short description that will be
displayed with the reason on a hidden item, this can include html <a> links to
editorial guidelines etc. Disabled reasons cannot be selected, but continue to
exist for any content already hidden using them.

Visit /admin/content/hidden/filters if you want to set up filters for words or
perl type regular expressions
(http://uk2.php.net/manual/en/reference.pcre.pattern.syntax.php) to match with
content that should be hidden pending moderation. If the filter is set to
happen immediately the user will be notified. If it is set to be delayed the
user will not be notified and the node or comment will only be automatically 
hidden after the delay has expired and a cron is run (or when a user hides it).
Each filter also has a reason associated with it. See usage notes about report
and hide forms for details about how this information is displayed.

Hidden content will be accessible to users with 'access hidden' permission on
pages starting /hidden. You may want to make a link to that page from
editorial guidelines or you can enable a navigation menu link to it on 
/admin/build/menu.

Additionally to the meta tags added to the /hidden pages created by the module
you may want to install a robots.txt file in the websites top directory
containing the lines:
  User-agent: *
  Disallow: /hidden/

Usage
======
Every node or comment has an extra link. 

Users with 'report for hiding' permission will have a link to report the post. The
following page allows them to explain why it should be hidden using one of the
default reasons set (see installation) and/or using free text notes.

Users with 'mark as hidden' permission will have a link to hide or unhide a
post. A hide link goes to a similar form as report for hiding. 

The forms have a public text box. This text, which can contain some HTML (as
Dupal 'Basic HTML' settings), will be displayed with the hidden post. 
There is also a private text box for users with 'mark as hidden' permission.
This will only be displayed to other users with 'mark as hidden' permission.

Mass Un-/Hiding for nodes:
 - Go to admin/content/node.
 - Tick the nodes you want to mass un-/hide.
 - Select Un-/Hide in the dropdown list of operations and click Update.
    The reason 'Policy Violation' will be applied automatically.

Mass Unhiding for nodes and comments:
 - Go to admin/content/node/list/hidden or admin/content/comment/list/hidden
 - Tick the nodes you want to mass unhide.
 - Click update

Reported nodes and comments:
 - Go to admin/content/node/list/hiddenreported or
    admin/content/comment/list/hiddenreported 
There you can mark content to be hidden for mass hiding of nodes above. You
can also mark them as 'seen' this lets other users who can 'mark as hidden'
that the reported content has been check and not hidden.

Notes
=====
 - The private notes section of the report, hide and filter forms, for users 
    with 'mark as hidden' privileges, can be used to leave messages about the
    content that will not be displayed or e-mailed out.
 - For filters regular expressions are slower than plain text searches, so if
    you are just looking for some text use plain text.
 - You don't need to make users full admin users to be able to deal with
    hidden posts.
 - Users who can administer nodes and comments will see hidden content with
    notes on pages other than /hidden ones.

Spam
====
You probably don't want to hide automated spam. This module is written to
integrate with the spam.module. Additional integration is on the to do wiki
page below.

Credits
=======
Sebastian Henschel <aotearoa <at> kodeaffe <dot> de>
  wrote original implementation of unpublishing hidden method (v1)
  for Aotearoa Indymedia Centre
ekes <ekes <at> aktivix <dot> org>
  rewritten and features implemented (v2)
  updated for Drupal 6 and testing framework
Testing: kameron, alster 

