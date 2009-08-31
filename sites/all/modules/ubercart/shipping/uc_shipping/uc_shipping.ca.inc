<?php
// $Id: uc_shipping.ca.inc,v 1.1.2.1 2009/08/17 21:27:56 islandusurper Exp $

/**
 * @file
 * Conditional actions hooks for uc_shipping.module.
 */

function uc_shipping_ca_entity() {
  $entities['uc_shipment'] = array(
    '#title' => t('Ubercart shipment object'),
    '#type' => 'object',
    '#load' => 'uc_shipping_shipment_load',
    '#save' => 'uc_shipping_shipment_save',
  );

  return $entities;
}

function uc_shipping_ca_trigger() {
  $triggers['uc_shipment_save'] = array(
    '#title' => t('A shipment is saved'),
    '#category' => t('Fulfillment'),
    '#arguments' => array(
      'order' => array(
        '#entity' => 'uc_order',
        '#title' => t('Order'),
      ),
      'shipment' => array(
        '#entity' => 'uc_shipment',
        '#title' => t('Shipment'),
      ),
    ),
  );

  return $triggers;
}
