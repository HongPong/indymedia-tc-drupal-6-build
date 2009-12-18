<?php
//$Id: nodequeue_handler_field_links.inc,v 1.2 2008/10/20 03:48:00 ezrag Exp $
/**
 * Views field handler for the nodequeue queue links
 */
class nodequeue_handler_field_links extends views_handler_field_node_link {
  function render($values) {
    $nid = $values->{$this->aliases['nid']};
    return theme('links', nodequeue_link('node', node_load($nid)));
  }
}