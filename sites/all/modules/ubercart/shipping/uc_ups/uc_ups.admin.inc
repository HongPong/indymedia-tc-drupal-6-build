<?php
// $Id: uc_ups.admin.inc,v 1.1.2.10 2009/08/17 21:27:54 islandusurper Exp $

/**
 * @file
 * UPS administration menu items.
 */

/**
 * UPS Online Tool settings.
 *
 * Record UPS account information neccessary to use the service. Allow testing
 * or production mode. Configure which UPS services are quoted to customers.
 *
 * @ingroup forms
 * @see uc_admin_settings_validate()
 */
function uc_ups_admin_settings() {
  $form = array();

  $form['uc_ups_conditions'] = array(
    '#value' => '<p>'. l(t('Set the conditions that will return a UPS quote.'), CA_UI_PATH .'/uc_ups_get_quote/edit/conditions') .'</p>',
  );

  $form['uc_ups_access_license'] = array('#type' => 'textfield',
    '#title' => t('UPS OnLine Tools XML Access Key'),
    '#default_value' => variable_get('uc_ups_access_license', ''),
    '#required' => TRUE,
  );
  $form['uc_ups_shipper_number'] = array('#type' => 'textfield',
    '#title' => t('UPS Shipper #'),
    '#description' => t('The 6-character string identifying your UPS account as a shipper.'),
    '#default_value' => variable_get('uc_ups_shipper_number', ''),
    '#required' => TRUE,
  );
  $form['uc_ups_user_id'] = array('#type' => 'textfield',
    '#title' => t('UPS.com user ID'),
    '#default_value' => variable_get('uc_ups_user_id', ''),
    '#required' => TRUE,
  );
  $form['uc_ups_password'] = array('#type' => 'password',
    '#title' => t('Password'),
    '#default_value' => variable_get('uc_ups_password', ''),
  );
  $form['uc_ups_connection_address'] = array('#type' => 'select',
    '#title' => t('Mode'),
    '#description' => t('Quotes and shipments requested in Testing mode will not be picked up or charged to your account.'),
    '#options' => array('https://wwwcie.ups.com/ups.app/xml/' => t('Testing'),
      'https://www.ups.com/ups.app/xml/' => t('Production'),
    ),
    '#default_value' => variable_get('uc_ups_connection_address', 'https://wwwcie.ups.com/ups.app/xml/'),
  );
  $form['uc_ups_services'] = array('#type' => 'checkboxes',
    '#title' => t('UPS services'),
    '#default_value' => variable_get('uc_ups_services', _uc_ups_service_list()),
    '#options' => _uc_ups_service_list(),
    '#description' => t('Select the UPS services that are available to customers.'),
  );
  $form['uc_ups_pickup_type'] = array('#type' => 'select',
    '#title' => t('Pickup type'),
    '#options' => array(
      '01' => 'Daily Pickup',
      '03' => 'Customer Counter',
      '06' => 'One Time Pickup',
      '07' => 'On Call Air',
      '11' => 'Suggested Retail Rates',
      '19' => 'Letter Center',
      '20' => 'Air Service Center',
    ),
    '#default_value' => variable_get('uc_ups_pickup_type', '01'),
  );
  $form['uc_ups_classification'] = array('#type' => 'select',
    '#title' => t('UPS Customer classification'),
    '#options' => array(
      '01' => t('Wholesale'),
      '03' => t('Occasional'),
      '04' => t('Retail'),
    ),
    '#default_value' => variable_get('uc_ups_classification', '04'),
    '#description' => t('The kind of customer you are to UPS. For daily pickups the default is wholesale; for customer counter pickups the default is retail; for other pickups the default is occasional.'),
  );
  $form['uc_ups_negotiated_rates'] = array('#type' => 'radios',
    '#title' => t('Negotiated rates'),
    '#default_value' => variable_get('uc_ups_negotiated_rates', 0),
    '#options' => array(1 => t('Yes'), 0 => t('No')),
    '#description' => t('Is your UPS account receiving negotiated rates on shipments?'),
  );
  $form['uc_ups_residential_quotes'] = array('#type' => 'radios',
    '#title' => t('Assume UPS shipping quotes will be delivered to'),
    '#default_value' => variable_get('uc_ups_residential_quotes', 0),
    '#options' => array(
      0 => t('Business locations'),
      1 => t('Residential locations (extra fees)'),
    ),
  );
  $form['uc_ups_markup_type'] = array('#type' => 'select',
    '#title' => t('Markup type'),
    '#default_value' => variable_get('uc_ups_markup_type', 'percentage'),
    '#options' => array(
      'percentage' => t('Percentage (%)'),
      'multiplier' => t('Multiplier (×)'),
      'currency' => t('Addition (!currency)', array('!currency' => variable_get('uc_currency_sign', '$'))),
    ),
  );
  $form['uc_ups_markup'] = array('#type' => 'textfield',
    '#title' => t('Shipping rate markup'),
    '#default_value' => variable_get('uc_ups_markup', '0'),
    '#description' => t('Markup shipping rate quote by currency amount, percentage, or multiplier.'),
  );
  $form['uc_ups_all_in_one'] = array('#type' => 'radios',
    '#title' => t('Product packages'),
    '#default_value' => variable_get('uc_ups_all_in_one', 1),
    '#options' => array(
      0 => t('Each in its own package'),
      1 => t('All in one'),
    ),
    '#description' => t('Indicate whether each product is quoted as shipping separately or all in one package. Orders with one kind of product will still use the package quantity to determine the number of packages needed, however.'),
  );
  $form['uc_ups_unit_system'] = array('#type' => 'select',
    '#title' => t('System of measurement'),
    '#default_value' => variable_get('uc_ups_unit_system', variable_get('uc_length_unit', 'in')),
    '#options' => array(
      'in' => t('British'),
      'cm' => t('Metric'),
    ),
    '#description' => t('Choose the standard system of measurement for your country.'),
  );
  $form['uc_ups_insurance'] = array(
    '#type' => 'checkbox',
    '#title' => t('Package insurance'),
    '#default_value' => variable_get('uc_ups_insurance', TRUE),
    '#description' => t('When enabled, products are insured for their full value.'),
  );

  $form['#validate'][] = 'uc_ups_admin_settings_validate';

  return system_settings_form($form);
}

/**
 * Validation handler for uc_ups_admin_settings.
 *
 * Require password only if it hasn't been set.
 *
 * @see uc_ups_admin_settings()
 */
function uc_ups_admin_settings_validate($form, &$form_state) {
  $old_password = variable_get('uc_ups_password', '');
  if (!$form_state['values']['uc_ups_password']) {
    if ($old_password) {
      form_set_value($form['uc_ups_password'], $old_password, $form_state);
    }
    else {
      form_set_error('uc_ups_password', t('Password field is required.'));
    }
  }
}

/**
 * Last chance for user to review shipment.
 *
 * @ingroup forms
 * @see theme_uc_ups_confirm_shipment()
 * @see uc_ups_confirm_shipment_submit()
 */
function uc_ups_confirm_shipment($order) {
  $form = array();
  $form['digest'] = array('#type' => 'hidden', '#value' => $_SESSION['ups']['digest']);
  $form['submit'] = array('#type' => 'submit', '#value' => t('Request Pickup'));
  return $form;
}

/**
 * Display final shipment information for review.
 *
 * @ingroup themeable
 * @see uc_ups_confirm_shipment()
 */
function theme_uc_ups_confirm_shipment($form) {
  $output = '';

  $output .= '<div class="shipping-address"><b>'. t('Ship from:') .'</b><br />';
  $output .= uc_address_format(
    check_plain($_SESSION['ups']['origin']->first_name),
    check_plain($_SESSION['ups']['origin']->last_name),
    check_plain($_SESSION['ups']['origin']->company),
    check_plain($_SESSION['ups']['origin']->street1),
    check_plain($_SESSION['ups']['origin']->street2),
    check_plain($_SESSION['ups']['origin']->city),
    check_plain($_SESSION['ups']['origin']->zone),
    check_plain($_SESSION['ups']['origin']->postal_code),
    check_plain($_SESSION['ups']['origin']->country)
  );
  $output .= '<br />'. check_plain($_SESSION['ups']['origin']->email);
  $output .= '</div>';

  $output .= '<div class="shipping-address"><b>'. t('Ship to:') .'</b><br />';
  $output .= uc_address_format(
    check_plain($_SESSION['ups']['destination']->first_name),
    check_plain($_SESSION['ups']['destination']->last_name),
    check_plain($_SESSION['ups']['destination']->company),
    check_plain($_SESSION['ups']['destination']->street1),
    check_plain($_SESSION['ups']['destination']->street2),
    check_plain($_SESSION['ups']['destination']->city),
    check_plain($_SESSION['ups']['destination']->zone),
    check_plain($_SESSION['ups']['destination']->postal_code),
    check_plain($_SESSION['ups']['destination']->country)
  );
  $output .= '<br />'. check_plain($_SESSION['ups']['destination']->email);
  $output .= '</div>';
  $output .= '<div class="shipment-data">';
  $method = uc_ups_shipping_method();
  $output .= '<b>'. $method['ups']['quote']['accessorials'][$_SESSION['ups']['service']] .'</b><br />';
  $context = array(
    'revision' => 'themed',
    'type' => 'amount',
  );
  $output .= '<i>'. check_plain($_SESSION['ups']['rate']['type']) .'</i>: '. uc_price($_SESSION['ups']['rate']['amount'], $context) .' ('. check_plain($_SESSION['ups']['rate']['currency']) .')<br />';
  $ship_date = $_SESSION['ups']['ship_date'];
  $output .= 'Ship date: '. format_date(gmmktime(12, 0, 0, $ship_date['month'], $ship_date['day'], $ship_date['year']), 'custom', variable_get('uc_date_format_default', 'm/d/Y'));
  $exp_delivery = $_SESSION['ups']['expected_delivery'];
  $output .= '<br />Expected delivery: '. format_date(gmmktime(12, 0, 0, $exp_delivery['month'], $exp_delivery['day'], $exp_delivery['year']), 'custom', variable_get('uc_date_format_default', 'm/d/Y'));
  $output .= "</div>\n<br style=\"clear: both;\" />";
  $output .= drupal_render($form);
  return $output;
}

/**
 * Generate label and schedule pickup of the shipment.
 *
 * @see uc_ups_confirm_shipment()
 */
function uc_ups_confirm_shipment_submit($form, &$form_state) {
  // Request pickup using parameters in form.
  $order_id = $_SESSION['ups']['order_id'];
  $packages = array_keys($_SESSION['ups']['packages']);
  $request = uc_ups_request_pickup($form_state['values']['digest'], $order_id, $packages);
  $result = drupal_http_request(variable_get('uc_ups_connection_address', 'https://wwwcie.ups.com/ups.app/xml/') .'ShipAccept', array(), 'POST', $request);
  $response = new SimpleXMLElement($result->data);
  $code = (string)$response->Response->ResponseStatusCode;
  if ($code == 0) { // failed request
    $error = $response->Response->Error;
    $error_severity = (string)$error->ErrorSeverity;
    $error_code = (string)$error->ErrorCode;
    $error_description = (string)$error->ErrorDescription;
    drupal_set_message(t('(@severity error @code) @description', array('@severity' => $error_severity, '@code' => $error_code, '@description' => $error_description)), 'error');
    if ($error_severity == 'HardError') {
      $form_state['redirect'] = 'admin/store/orders/'. $order_id .'/shipments/ups/'. implode('/', $packages);
      return;
    }
  }

  $shipment = new stdClass();
  $shipment->order_id = $order_id;
  $shipment->origin = drupal_clone($_SESSION['ups']['origin']);
  $shipment->destination = drupal_clone($_SESSION['ups']['destination']);
  $shipment->packages = $_SESSION['ups']['packages'];
  $shipment->shipping_method = 'ups';
  $shipment->accessorials = $_SESSION['ups']['service'];
  $shipment->carrier = t('UPS');
  // if NegotiatedRates exist, quote based on those, otherwise, use TotalCharges
  if (isset($response->ShipmentResults->ShipmentCharges)) {
    $charge = $response->ShipmentResults->ShipmentCharges->TotalCharges;
    if (isset($response->ShipmentResults->NegotiatedRates)) {
      $charge = $response->ShipmentResults->NegotiatedRates->NetSummaryCharges->GrandTotal;
    }
  }
  $cost = (string)$charge->MonetaryValue;
  $shipment->cost = $cost;
  $shipment->tracking_number = (string)$response->ShipmentResults->ShipmentIdentificationNumber;
  $ship_date = $_SESSION['ups']['ship_date'];
  $shipment->ship_date = gmmktime(12, 0, 0, $ship_date['month'], $ship_date['day'], $ship_date['year']);
  $exp_delivery = $_SESSION['ups']['expected_delivery'];
  $shipment->expected_delivery = gmmktime(12, 0, 0, $exp_delivery['month'], $exp_delivery['day'], $exp_delivery['year']);

  foreach ($response->ShipmentResults->PackageResults as $package_results) {
    $package =& current($shipment->packages);
    $package->tracking_number = (string)$package_results->TrackingNumber;
    $label_image = (string)$package_results->LabelImage->GraphicImage;
    if (file_check_directory(file_create_path('ups_labels'), FILE_CREATE_DIRECTORY)) {
      $label_path = file_create_path('ups_labels') .'/label'. $package->tracking_number .'.gif';
      if ($label_file = fopen($label_path, 'wb')) {
        fwrite($label_file, base64_decode($label_image));
        fclose($label_file);
        $package->label_image = $label_path;
      }
      else {
        drupal_set_message(t('Could not open a file to save the label image.'), 'error');
      }
    }
    else {
      drupal_set_message(t('Could not find or create the directory "ups_labels" in the file system path.'), 'error');
    }
    unset($package);
    next($shipment->packages);
  }

  uc_shipping_shipment_save($shipment);

  unset($_SESSION['ups']);
  $form_state['redirect'] = 'admin/store/orders/'. $order_id .'/shipments';
}

/**
 * Display the shipping label for printing.
 *
 * Each argument is a component of the file path to the image.
 *
 * @ingroup themeable
 */
function theme_uc_ups_label_image() {
  $args = func_get_args();
  $image_path = implode('/', $args);
  print '<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 3.2//EN">
<html><head><title>
View/Print Label</title></head><style>
    .small-text {font-size: 80%;}
    .large-text {font-size: 115%;}
</style>
<body bgcolor="#FFFFFF">
<table border="0" cellpadding="0" cellspacing="0" width="600"><tr>
<td height="410" align="left" valign="top">
<b class="large-text">View/Print Label</b>
&nbsp;<br />
<ol class="small-text"> <li><b>Print the label:</b> &nbsp;
Select Print from the File menu in this browser window to print the label below.<br /><br /><li><b>
Fold the printed label at the dotted line.</b> &nbsp;
Place the label in a UPS Shipping Pouch. If you do not have a pouch, affix the folded label using clear plastic shipping tape over the entire label.<br /><br /><li><b>GETTING YOUR SHIPMENT TO UPS<br />
Customers without a Daily Pickup</b><ul><li>Ground, 3 Day Select, and Standard to Canada shipments must be dropped off at an authorized UPS location, or handed to a UPS driver. Pickup service is not available for these services. To find the nearest drop-off location, select the Drop-off icon from the UPS tool bar.<li>
Air shipments (including Worldwide Express and Expedited) can be picked up or dropped off. To schedule a pickup, or to find a drop-off location, select the Pickup or Drop-off icon from the UPS tool bar.  </ul> <br />
<b>Customers with a Daily Pickup</b><ul><li>
Your driver will pickup your shipment(s) as usual. </ul>
</ol></td></tr></table><table border="0" cellpadding="0" cellspacing="0" width="600">
<tr>
<td class="small-text" align="left" valign="top">
&nbsp;&nbsp;&nbsp;
FOLD HERE</td>
</tr>
<tr>
<td align="left" valign="top"><hr>
</td>
</tr>
</table>

<table>
<tr>
<td height="10">&nbsp;
</td>
</tr>
</table>

<table border="0" cellpadding="0" cellspacing="0" width="650" ><tr>
<td align="left" valign="top">
<img src="'. base_path() . $image_path .'" height="392" width="672">
</td>
</tr></table>
</body>
</html>';

  exit();
}


/**
 * Theme function to format the UPS service name and rate amount line-item shown
 *   to the customer.
 *
 * @param $service
 *   The UPS service name.
 * @ingroup themeable
 */
function theme_uc_ups_option_label($service) {
  // Start with logo as required by the UPS terms of service.
  $output = '<img class="ups-logo" src="'. url(drupal_get_path('module', 'uc_ups')) .'/uc_ups_logo.gif" alt="'. t('UPS Logo') .'" /> ';

  // Add the UPS service name.
  $output .= t('@service Rate', array('@service' => $service));

  return $output;
}
