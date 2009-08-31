<?php
// $Id: uc_stock.ca.inc,v 1.1.2.7 2009/07/21 14:51:22 islandusurper Exp $

/**
 * @file
 * This file contains all the Workflow-NG hooks that are neccesary for Workflow
 * integeration with the uc_stock module
 */

/******************************************************************************
 * Conditional Action Hooks                                                   *
 ******************************************************************************/

/**
 * Implementation of hook_ca_predicate().
 */
function uc_stock_ca_predicate() {
  $predicates['uc_stock_decrement_on_order'] = array(
    '#title' => t('Decrement stock upon order submission'),
    '#trigger' => 'uc_checkout_complete',
    '#class' => 'uc_stock',
    '#status' => 1,
    '#actions' => array(
      array(
        '#name' => 'uc_stock_action_decrement_stock',
        '#title' => t('Decrement stock of products in order'),
        '#argument_map' => array(
          'order' => 'order',
        ),
      ),
    ),
  );

  return $predicates;
}

/**
 * Implementation of hook_action().
 */
function uc_stock_ca_action() {
  $actions['uc_stock_action_decrement_stock'] = array(
    '#title' => t('Decrement stock of products on the order with tracking activated.'),
    '#callback' => 'uc_stock_action_decrement_stock',
    '#arguments' => array(
      'order' => array('#entity' => 'uc_order', '#title' => t('Order')),
    ),
    '#category' => t('Stock'),
  );

  return $actions;
}

/******************************************************************************
 * Conditional Action Callbacks and Forms                                     *
 ******************************************************************************/

/**
 * Decrease the stock of ordered products.
 */
function uc_stock_action_decrement_stock($order, $settings) {
  if (is_array($order->products)) {
    $stock_warnings = array();
    foreach ($order->products as $product) {
      if (($stock = uc_stock_level($product->model)) !== FALSE) {
        $stock_level = db_fetch_object(db_query("SELECT * FROM {uc_product_stock} WHERE sku = '%s'", $product->model));
        if ((($stock - $product->qty) <= $stock_level->threshold) && !in_array($product->model, array_keys($stock_warnings))) {
          $stock_level->stock -= $product->qty;
          $stock_warnings[$product->model] = $stock_level;
        }
        uc_stock_adjust($product->model, -$product->qty);
        uc_order_comment_save($order->order_id, 0, t('The stock level for %model_name has been decreased to !qty.', array('%model_name' => $product->model, '!qty' => ($stock - $product->qty))));
      }
    }
    if (!empty($stock_warnings) && variable_get('uc_stock_threshold_notification', FALSE)) {
      foreach ($stock_warnings as $model => $stock_level) {
        _uc_stock_send_mail($order, $stock_level);
      }
    }
  }
}
