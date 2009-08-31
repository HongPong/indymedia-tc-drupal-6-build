// $Id: ca.js,v 1.1.2.3 2009/07/21 14:37:19 islandusurper Exp $

/**
 * @file
 *   Adds some helper JS to the conditional actions forms.
 */

/**
 * Add confirmation prompts to remove buttons.
 */
Drupal.behaviors.caRemoveConfirm = function(context) {
  $('.ca-remove-confirm:not(.caRemoveConfirm-processed)', context).addClass('caRemoveConfirm-processed').click(function() {
    return confirm(Drupal.t('Are you sure you want to remove this item?'));
  });
}

