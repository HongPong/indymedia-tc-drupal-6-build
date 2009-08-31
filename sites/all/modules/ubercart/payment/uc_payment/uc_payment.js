// $Id: uc_payment.js,v 1.5.2.5 2009/02/24 15:35:51 islandusurper Exp $

// Arrays for order total preview data.
var li_titles = {};
var li_values = {};
var li_weight = {};
var li_summed = {};

// Timestamps for last time line items or payment details were updated.
var line_update = 0;
var payment_update = 0;

var do_payment_details = true;

if (Drupal.jsEnabled) {
  jQuery.extend(Drupal.settings, {
    ucShowProgressBar: false,
    ucDefaultPayment: '',
    ucOrderInitiate: false
  });

  $(document).ready(
    function() {

      // attach a progressbar if requested
      if (Drupal.settings.ucShowProgressBar) {
        show_progressBar('#line-items-div');
      }

      // initialize payment details
      if (Drupal.settings.ucDefaultPayment != '') {
        init_payment_details(Drupal.settings.ucDefaultPayment);
      }

      // disable the submission buttons and get payment details
      if (Drupal.settings.ucOrderInitiate) {
        add_order_save_hold();
        get_payment_details('admin/store/orders/' + $('#edit-order-id').val() + '/payment_details/' + $('#edit-payment-method').val());
      }
    }
  )
}

function show_progressBar(id) {
  var progress = new Drupal.progressBar('paymentProgress');
  progress.setProgress(-1, '');
  $(id).empty().append(progress.element);
}

/**
 * Sets a line item in the order total preview.
 */
function set_line_item(key, title, value, weight, summed, render) {
  var do_update = false;

  if (summed === undefined) {
    summed = 1;
  }
  // Check to see if we're actually changing anything and need to update.
  if (window.li_values[key] === undefined) {
    do_update = true;
  }
  else {
    if (li_titles[key] != title || li_values[key] != value || li_weight[key] != weight || li_summed[key] != summed) {
      do_update = true;
    }
  }

  if (do_update) {
    // Set the values passed in, overriding previous values for that key.
    if (key != "") {
      li_titles[key] = title;
      li_values[key] = value;
      li_weight[key] = weight;
      li_summed[key] = summed;
    }
    if (render == null || render) {
      render_line_items();
    }
  }
}

function render_line_items() {
  // Set the timestamp for this update.
  var this_update = new Date();

  // Set the global timestamp for the update.
  line_update = this_update.getTime();

  // Put all the existing line item data into a single array.
  var li_info = {};
  var cur_total = 0;
  $.each(li_titles,
    function(a, b) {
      li_info[a] = li_weight[a] + ';' + li_values[a] + ';' + li_titles[a] + ';' + li_summed[a];

      // Tally up the current order total for storage in a hidden item.
      if (li_titles[a] != '' && li_summed[a] == 1) {
        cur_total += li_values[a];
      }
    }
  );
  $('#edit-panes-payment-current-total').val(cur_total).click();

  $('#order-total-throbber').addClass('ubercart-throbber').html('&nbsp;&nbsp;&nbsp;&nbsp;');

  // Post the line item data to a URL and get it back formatted for display.
  $.post(Drupal.settings.basePath + '?q=cart/checkout/line_items', li_info,
    function(contents) {
      // Only display the changes if this was the last requested update.
      if (this_update.getTime() == line_update) {
        $('#line-items-div').empty().append(contents);
      }
    }
  );
}

function remove_line_item(key) {
  delete li_titles[key];
  delete li_values[key];
  delete li_weight[key];
  delete li_summed[key];
  render_line_items();
}

/**
 * Doesn't refresh the payment details if they've already been loaded.
 */
function init_payment_details(payment_method) {
  if (payment_update == 0) {
    get_payment_details('cart/checkout/payment_details/' + payment_method);
  }
}

/**
 * Display the payment details when a payment method radio button is clicked.
 */
function get_payment_details(path) {
  var progress = new Drupal.progressBar('paymentProgress');
  progress.setProgress(-1, '');
  $('#payment_details').empty().append(progress.element).removeClass('display-none');

  // Get the timestamp for the current update.
  var this_update = new Date();

  // Set the global timestamp for the update.
  payment_update = this_update.getTime();

  if ($('#edit-payment-details-data').length) {
    data = { 'payment-details-data' : $('#edit-payment-details-data').val() };
  }
  else {
    data = {};
  }
  // Make the post to get the details for the chosen payment method.
  $.post(Drupal.settings.basePath + '?q=' + path, data,
    function(details) {
      if (this_update.getTime() == payment_update) {
        // If the response was empty, throw up the default message.
        if (details == '') {
          $('#payment_details').empty().html(Drupal.settings.defPaymentMsg);
        }
        // Otherwise display the returned details.
        else {
          $('#payment_details').empty().append(details);
        }
      }

      // If on the order edit screen, clear out the order save hold.
      if (window.remove_order_save_hold) {
        remove_order_save_hold();
      }
    }
  );
}

/**
 * Pop-up an info box for the credit card CVV.
 */
function cvv_info_popup() {
  var popup = window.open(Drupal.settings.basePath + '?q=cart/checkout/credit/cvv_info', 'CVV_Info', 'toolbar=0,scrollbars=1,location=0,statusbar=0,menubar=0,resizable=1,width=480,height=460,left=282,top=122');
}

/**
 * Toggle the payment fields on and off on the receive check form.
 */
function receive_check_toggle(checked) {
  if (!checked) {
    $('#edit-amount').removeAttr('disabled').val('');
    $('#edit-comment').removeAttr('disabled').val('');
  }
  else {
    $('#edit-amount').attr('disabled', 'true').val('-');
    $('#edit-comment').attr('disabled', 'true').val('-');
  }
}

