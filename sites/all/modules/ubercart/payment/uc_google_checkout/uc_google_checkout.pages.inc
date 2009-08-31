<?php
// $Id: uc_google_checkout.pages.inc,v 1.1.2.4 2009/04/29 12:32:46 islandusurper Exp $

/**
 * @file
 * Google Checkout administration menu items.
 *
 */

function uc_google_checkout_callback() {
  if (variable_get('uc_google_checkout_mode', 'checkout') == 'checkout') {
    $merchant_id = variable_get('uc_google_checkout_merchant_id', '');
    $merchant_key = variable_get('uc_google_checkout_merchant_key', '');
  }
  else {
    $merchant_id = variable_get('uc_google_checkout_test_merchant_id', '');
    $merchant_key = variable_get('uc_google_checkout_test_merchant_key', '');
  }

  //watchdog('google', check_plain($_SERVER['PHP_AUTH_USER']), NULL, WATCHDOG_NOTICE);
  if ($_SERVER['PHP_AUTH_USER'] != $merchant_id) {
    watchdog('google', 'HTTP Authorization User does not match settings.', array(), WATCHDOG_ERROR);
  }
  //watchdog('google', check_plain($_SERVER['PHP_AUTH_PW']), NULL, WATCHDOG_NOTICE);
  if ($_SERVER['PHP_AUTH_PW'] != $merchant_key) {
    watchdog('google', 'HTTP Authorization Password does not match settings.', array(), WATCHDOG_ERROR);
  }
  $input = file_get_contents('php://input');
  //watchdog('google', print_r(check_plain($input), TRUE), NULL, WATCHDOG_NOTICE);
  $xml = new SimpleXMLElement($input);
  switch ($xml->getName()) {
    case 'merchant-calculation-callback':
      //uc_google_checkout_merchant_calculation($xml);
    break;
    case 'new-order-notification':
      uc_google_checkout_new_order($xml);
    break;
    case 'order-state-change-notification':
      uc_google_checkout_order_state_change($xml);
    break;
    case 'risk-information-notification':
      uc_google_checkout_accept_risk($xml);
    break;
    case 'charge-amount-notification':
      uc_google_checkout_charge_order($xml);
    break;
    case 'refund-amount-notification':
      uc_google_checkout_refund_order($xml);
    break;
    default:
      watchdog('google', 'Unknown notification document: @xml', array('@xml' => $input), WATCHDOG_ERROR);
      $output = uc_google_checkout_notification_acknowledgement($xml->{'serial-number'});
  }
}

