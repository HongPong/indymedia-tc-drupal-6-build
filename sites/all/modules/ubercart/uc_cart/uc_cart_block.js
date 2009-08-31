// $Id: uc_cart_block.js,v 1.8.2.4 2009/07/21 14:37:20 islandusurper Exp $

/**
 * @file
 * Adds effects and behaviors to the cart block.
 */

/**
 * Set the behavior to (un)collapse the cart block on a click
 */
Drupal.behaviors.ucCollapseBlock = function(context) {
  $('.cart-block-title-bar:not(.ucCollapseBlock-processed)', context).addClass('ucCollapseBlock-processed').click(
    function() {
      cart_block_toggle();
    }
  );
}

/**
 * Collapse the shopping cart block at page load.
 */
$(document).ready(
  function() {
    if (Drupal.settings.ucCollapsedBlock == true) {
      // Hide the contents.
      $('#cart-block-contents').hide();

      // Add the appropriate title bar class.
      $('.cart-block-title-bar').addClass('cart-block-toggle');

      // Add the appropriate arrow class.
      $('.cart-block-arrow').removeClass('arrow-down').addClass('arrow-up');
    }
  }
);

/**
 * Toggle the shopping cart block open and closed.
 */
function cart_block_toggle() {
  // Toggle the display of the cart contents table.
  $('#cart-block-contents').toggle();

  // Toggle the class of the cart block arrow.
  if ($('.cart-block-arrow').hasClass('arrow-up')) {
    $('.cart-block-arrow').removeClass('arrow-up').addClass('arrow-down');
  }
  else {
    $('.cart-block-arrow').removeClass('arrow-down').addClass('arrow-up');
  }
}
