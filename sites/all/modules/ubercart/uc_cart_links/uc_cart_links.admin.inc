<?php
// $Id: uc_cart_links.admin.inc,v 1.1.2.4 2009/07/21 14:37:19 islandusurper Exp $

/**
 * @file
 * Cart links administration menu items.
 */

/**
 * Define a form to configure the cart links settings.
 *
 * @ingroup forms
 */
function uc_cart_links_settings_form() {
  $form = array();

  $form['instructions'] = array(
    '#value' => '<div>'. t('<a href="!url">View the help page</a> to learn how to create cart links.', array('!url' => url('admin/store/help/cart_links'))) .'</div>',
  );

  $form['uc_cart_links_add_show'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display the cart link product action when you add a product to your cart.'),
    '#default_value' => variable_get('uc_cart_links_add_show', FALSE),
  );
  $form['uc_cart_links_track'] = array(
    '#type' => 'checkbox',
    '#title' => t('Track clicks through cart links that specify tracking IDs.'),
    '#default_value' => variable_get('uc_cart_links_track', TRUE),
  );
  $form['uc_cart_links_messages'] = array(
    '#type' => 'textarea',
    '#title' => t('Cart links messages'),
    '#description' => t('Enter in messages available to the cart links API for display through a link. Separate messages with a line break.<br />Messages should have a numeric key and text value. Example: 1337|Message text.'),
    '#default_value' => variable_get('uc_cart_links_messages', ''),
  );
  $form['uc_cart_links_empty'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow cart links to empty customer carts.'),
    '#default_value' => variable_get('uc_cart_links_empty', TRUE),
  );
  $form['uc_cart_links_restrictions'] = array(
    '#type' => 'textarea',
    '#title' => t('Cart links restrictions'),
    '#description' => t('To restrict what cart links may be used on your site, enter valid cart links in this textbox.  Separate links with a line break. Leave blank to permit any cart link.'),
    '#default_value' => variable_get('uc_cart_links_restrictions', ''),
  );
  $form['uc_cart_links_invalid_page'] = array(
    '#type' => 'textfield',
    '#title' => t('Invalid link redirect page'),
    '#description' => t('Enter the URL to redirect to when an invalid cart link is used.'),
    '#default_value' => variable_get('uc_cart_links_invalid_page', ''),
    '#size' => 32,
    '#field_prefix' => url(NULL, array('absolute' => TRUE)) . (variable_get('clean_url', 0) ? '' : '?q='),
  );

  return system_settings_form($form);
}

/**
 * Display the cart links report.
 *
 * @return
 *   HTML output.
 */
function uc_cart_links_report() {
  $header = array(
    array('data' => t('ID'), 'field' => 'cart_link_id'),
    array('data' => t('Clicks'), 'field' => 'clicks'),
    array('data' => t('Last click'), 'field' => 'last_click', 'sort' => 'desc'),
  );

  $rows = array();
  $result = pager_query("SELECT * FROM {uc_cart_link_clicks}". tablesort_sql($header), 25, 1);
  while ($data = db_fetch_object($result)) {
    $rows[] = array(
      check_plain($data->cart_link_id),
      $data->clicks,
      format_date($data->last_click, 'short'),
    );
  }

  if (empty($rows)) {
    $rows[] = array(
      array('data' => t('No cart links have been tracked yet.'), 'colspan' => 3),
    );
  }

  return theme('table', $header, $rows) . theme('pager', array(), 25, 1);
}

/**
 * Learn how to create cart links for your products.
 *
 * @return
 *   Help text in HTML format.
 */
function uc_cart_links_creation_help() {
  $items = array(
    t('The cart link should be /cart/add/cart_link_content.'),
    t('Chain together as many actions as you want with dashes.'),
    t('Do not put any spaces or use dashes in any action arguments.'),
    t('Use the table below to learn about actions and their arguments.'),
    t('Arguments come directly after their action letters.'),
    t('Specify the redirection by adding ?destination=url where url is the page to go to.'),
  );
  $header = array(t('Action'), t('Description'), t('Argument'));
  $rows = array(
    array('i', t('Sets the ID of the cart link.'), t('A custom text ID for the link.')),
    array('e', t("Empties the customer's cart."), t('None.')),
    array('m', t('Displays a preset message to the customer.'), t('A message ID.')),
    array('p', t('Adds a product to the cart.'), t('A product string using the rules below...')),
  );
  $items2 = array(
    t("You must at least specify a node ID immediately after the 'p'."),
    t('Add additional specifications separated by underscores.'),
    t('Specify the quantity with q followed by the number to add.'),
    t('Specify attributes/options using a#o#, replacing # with the ID of the attribute and option.'),
    t('Turn off the add to cart message with m0.'),
  );

  $output = '<div>'. t('There is currently no user interface for creating cart links, but this section includes some basic instructions.<br />Cart links are simple to form using a few actions and arguments with the following rules:')
    .'<p>'. theme('item_list', $items) .'</p><p>'. theme('table', $header, $rows) .'</p><p>'. theme('item_list', $items2) .'</p><p>'. t('<b>Example:</b> /cart/add/e-p1_q5-imonday_special?destination=cart<br /><br />This example will empty the cart, add 5 of product 1 to the cart, track clicks with the ID "monday_special", and redirect the user to the cart. To use this on your site, simply create an HTML link to the URL you create:')
    .'</p><p>&lt;a href="http://www.example.com/cart/add/e-p1_q5-imonday_special?destination=cart">'. t('Link text.') .'&lt;/a></p></div>';

  return $output;
}
