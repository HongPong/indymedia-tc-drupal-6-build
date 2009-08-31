<?php
// $Id: uc_product_handler_field_price.inc,v 1.1.2.5 2009/08/17 21:27:58 islandusurper Exp $

/**
 * @file
 * Views handler: Product price field.
 */

/**
 * Return a formatted price value to display in the View.
 */
class uc_product_handler_field_price extends views_handler_field {
  function render($values) {
    $context = array(
      'revision' => 'themed',
      'type' => 'product',
      'class' => array(
        'product',
        $this->field,
      ),
      'field' => $this->real_field,
      'subject' => array(
        'node' => $values,
      ),
    );
    $options = array('label' => FALSE);

    $table_alias_len = strlen($this->table_alias);
    foreach ($values as $key => $value) {
      if (substr($key, 0, $table_alias_len) == $this->table_alias) {
        $values->{substr($key, $table_alias_len + 1)} = $value;
      }
    }

    return uc_price($values->{$this->field_alias}, $context, $options);
  }
}
