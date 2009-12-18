<?php
// $Id: hidden.display-pages.inc,v 1.4 2008/12/18 15:29:07 ekes Exp $

/**
 * @file
 * Hidden user pages to display hidden content.
 */

/**
 * Menu callback; a page with the hidden nodes list.
 *
 * @see theme_hidden_list_nodes()
 */
function hidden_list_nodes() {
  $query = 'SELECT n.nid AS nid, n.title AS title,'.
    ' r.title AS reason,'.
    ' u.uid AS uid, u.name AS name'.
    ' FROM {hidden_node} AS h'.
    ' LEFT JOIN {hidden_reasons} AS r ON h.rid=r.rid'.
    ' INNER JOIN ({users} AS u, {node} AS n) ON (n.uid=u.uid AND n.nid=h.nid)'.
    ' ORDER BY h.created DESC';
  $table  = _hidden_list_get($query, 'node');
  $output = theme('hidden_list_nodes', $table);
  drupal_set_html_head('<meta name="robots" content="noindex, nofollow" />');
  $output .= theme('pager', NULL, HIDDEN_PAGER_LIMIT);
  return $output;
}

/**
 * Menu callback; a page with the hidden comments list.
 *
 * @see theme_hidden_list_comments()
 */
function hidden_list_comments() {
  $query = 'SELECT c.cid AS cid, c.subject AS title,'.
          ' r.title AS reason,'.
          ' u.uid AS uid, u.name AS name'.
          ' FROM {hidden_comment} AS h'.
          ' LEFT JOIN {hidden_reasons} AS r ON h.rid=r.rid'.
          ' INNER JOIN ({users} AS u, {comments} AS c) ON (c.uid=u.uid AND c.cid=h.cid)'.
          ' ORDER BY h.created DESC';
  $table = _hidden_list_get($query, 'comment');
  $output = theme('hidden_list_comments', $table);
  drupal_set_html_head('<meta name="robots" content="noindex, nofollow" />');
  $output .= theme('pager', NULL, HIDDEN_PAGER_LIMIT);

  return $output;
}

/**
 * Formats table of hidden nodes.
 *
 * @param $table
 *  results from _hidden_list_get().
 * @return
 *  text html themed table.
 * @ingroup themeable.
 */
function theme_hidden_list_nodes($table) {
  return theme(
      'table',
      array(t('Title'), t('Author'), t('Reason')),
      $table
      );
}

/**
 * Format table of hidden comments.
 *
 * @param $table
 *  results from _hidden_list_get().
 * @return
 *  text html themed table.
 * @ingroup themable.
 */
function theme_hidden_list_comments($table) {
  return theme(
    'table',
    array(t('Title'), t('Author'), t('Reason')),
    $table
  );
}

/**
 * Menu callback; display a single hidden node or comment.
 *
 * @param $hidden
 *   object node or comment.
 * @return
 *   page content if is hidden.
 */
function hidden_view($type, $id) {
  if (! $hidden = _hidden_hidden_get($type, $id)) {
    drupal_not_found();
    return;
  }
  if ($type == 'comment') {
    $comment = _comment_load($id);
    $node = node_load($comment->nid);
    drupal_set_title(t('Hidden comment: @title', array('@title' => $comment->subject)));
    $links = hidden_link('comment', $comment);
    $output = theme_comment_view($comment, $node, $links);
  }
  elseif ($type == 'node') {
    $node = node_load($id);
    drupal_set_title(t('Hidden node: @title', array('@title' => $node->title)));
    $output = node_show($node, 0);
  }
  else {
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_view() called with invalid $hidden->type.');
    drupal_not_found();
    return;
  }
  return $output;
}
