<?php
// $Id: uc_authorizenet.pages.inc,v 1.1.2.2 2009/04/28 21:03:14 islandusurper Exp $

/**
 * @file
 * Includes page callbacks for Authorize.Net's Silent POST feature and user
 *   specific recurring fee operation pages.
 */


// Receives a payment notification and handles it appropriately.
function uc_authorizenet_silent_post() {
  if (variable_get('uc_authnet_report_arb_post', FALSE)) {
    watchdog('uc_authorizenet', '!arbSilent POST received: <pre>@post</pre>', array('!arb' => (isset($_POST['x_subscription_id']) && isset($_POST['x_subscription_paynum'])) ? 'ARB ' : '', '@post' => print_r($_POST, TRUE)));
  }

  // Decrypt the Auth.Net API login data.
  $login_data = _uc_authorizenet_login_data();

  // TODO: Modify the MD5 hash to accommodate differences from AIM to ARB.

  // If we're receiving notification for an ARB payment...
  if (isset($_POST['x_subscription_id']) && isset($_POST['x_subscription_paynum'])) {
    // Compare our expected MD5 Hash against what was received.
    $md5 = strtoupper(md5($login_data['md5_hash'] . $_POST['x_trans_id'] . $_POST['x_amount']));

    // Post an error message if the MD5 hash does not validate.
    if ($_POST['x_MD5_Hash'] != $md5) {
      watchdog('uc_authorizenet', 'Invalid ARB payment notification received.', array(), WATCHDOG_ERROR);
    }
    else {
      // Otherwise, update the recurring fee information in our database.
      $fee = db_fetch_array(db_query("SELECT * FROM {uc_recurring_users} WHERE fee_handler = 'uc_authorizenet' AND data = '%s'", $_POST['x_subscription_id']));

      // Only process if the fee actually exists!
      if (!empty($fee)) {
        // Update the interval counters.
        $fee['remaining_intervals'] -= 1;
        $fee['charged_intervals'] += 1;

        // Set the next expected charge time.
        $fee['next_charge'] = strtotime('+'. $fee['regular_interval'], $fee['next_charge']);

        // Save the new values.
        uc_recurring_fee_save('user', $fee);

        // Log the ARB payment if enabled.
        if (variable_get('uc_authnet_report_arb_post', FALSE)) {
          watchdog('uc_authorizenet', 'ARB payment reported for order @order_id: <pre>@post</pre>', array('@order_id' => $fee['order_id'], '@post' => print_r($_POST, TRUE)));
        }

        // Let other modules act on the data.
        module_invoke_all('uc_arb_payment', $fee);
      }
    }
  }

  exit();
}

// Displays a form for customers to update their CC info.
function uc_authorizenet_arb_user_update_form($form_state, $user, $rfid) {
  $form = array();

  $fee = uc_recurring_fee_load('user', $rfid);

  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $user->uid,
  );
  $form['rfid'] = array(
    '#type' => 'value',
    '#value' => $rfid,
  );
  $form['description'] = array(
    '#value' => '<div>'. t('Recurring fee order ID: @order_id', array('@order_id' => $fee['order_id'])) .'</div>',
  );

  $form['cc_data'] = array(
    '#type' => 'fieldset',
    '#title' => t('Credit card details'),
    '#theme' => 'uc_payment_method_credit_form',
    '#tree' => TRUE,
  );
  $form['cc_data'] += uc_payment_method_credit_form(array(), $order);
  unset($form['cc_data']['cc_policy']);

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#suffix' => l(t('Cancel'), 'user/'. $user->uid),
  );

  return $form;
}

function uc_authorizenet_arb_user_update_form_submit($form, &$form_state) {
  $fee = uc_recurring_fee_load('user', $form_state['values']['rfid']);

  $updates = array(
    'payment' => array(
      'creditCard' => array(
        'cardNumber' => $form_state['values']['cc_data']['cc_number'],
        'expirationDate' => $form_state['values']['cc_data']['cc_exp_year'] .'-'. $form_state['values']['cc_data']['cc_exp_month'],
      ),
    ),
  );

  $result = uc_authorizenet_arb_update($fee['data'], $updates, $fee['order_id']);

  // If the update was successful...
  if ($result) {
    drupal_set_message(t('The payment details for that recurring fee have been updated.'));
  }
  else {
    drupal_set_message(t('An error has occurred while updating your payment details. Please try again and contact us if you are unable to perform the update.'), 'error');
  }

  $form_state['redirect'] = 'user/'. $form_state['values']['uid'];
}

// Displays a confirm form for customers to cancel their fees.
function uc_authorizenet_arb_user_cancel_form($form_state, $user, $rfid) {
  $form['uid'] = array(
    '#type' => 'value',
    '#value' => $user->uid,
  );
  $form['rfid'] = array(
    '#type' => 'value',
    '#value' => $rfid,
  );

  return confirm_form($form, t('Are you sure you wish to cancel this fee?'), 'user/'. $user->uid, t('This action cannot be undone and may result in the termination of subscription services.'), t('Confirm'), t('Cancel'));
}

function uc_authorizenet_arb_user_cancel_form_submit($form, &$form_state) {
  $fee = uc_recurring_fee_load('user', $form_state['values']['rfid']);

  $result = uc_authorizenet_arb_cancel($fee['data'], $fee['order_id'], $fee);

  // If the cancellation was successful...
  if ($result) {
    drupal_set_message(t('The recurring fee has been canceled.'));

    // Set the fee's recurring charges to 0.
    uc_recurring_fee_cancel($fee['rfid']);
  }
  else {
    drupal_set_message(t('An error has occurred. Please try again and contact us if the problem persists.'), 'error');
  }

  $form_state['redirect'] = 'user/'. $form_state['values']['uid'];
}

