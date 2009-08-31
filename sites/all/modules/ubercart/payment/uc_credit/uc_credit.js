// $Id: uc_credit.js,v 1.3.4.2 2008/11/07 21:13:23 islandusurper Exp $

if (Drupal.jsEnabled) {
  $(document).ready(
    function () {
      $('#cc_details_title').show(0);
      $('#cc_details').hide(0);
    }
  );
}

/**
 * Toggle credit card details on the order view screen.
 */
function toggle_card_details() {
  $('#cc_details').toggle();
  $('#cc_details_title').toggle();
}

