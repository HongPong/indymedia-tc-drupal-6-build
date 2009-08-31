<?php
// $Id: hidden.display-admin.inc,v 1.2 2008/12/12 19:23:35 ekes Exp $

/**
 * @file
 * Hidden administration of hidden content.
 */

/**
 * Menu callback; Admin list form of hidden comments.
 *
 * @ingroup forms
 * @see hidden_list_admin_comments_validate()
 * @see hidden_list_admin_comments_submit()
 * @see theme_hidden_list_admin_comments()
 */
function hidden_list_admin_comments(&$form_state) {
  $form = _hidden_list_admin_options();
  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      NULL,
      array('data' => t('title'), 'field' => 'title'),
      array('data' => t('author'), 'field' => 'name'),
      array('data' => t('reason'), 'field' => 'reason'),
      array('data' => t('hidden by'), 'field' => 'hider'),
      array('data' => t('hidden'), 'field' => 'date', 'sort' => 'desc'),
      NULL,
    )
  );

  $query = 'SELECT c.nid AS nid, c.cid AS cid, c.subject AS title, c.timestamp AS changed,'.
    ' h.created AS date, h.delay AS delay,'.
    ' r.title AS reason,'.
    ' u1.uid AS uid, u1.name AS name,'.
    ' u2.uid AS huid, u2.name AS hider'.
    ' FROM {hidden_comment} AS h'.
    ' LEFT JOIN {hidden_reasons} AS r ON h.rid=r.rid'.
    ' INNER JOIN ({comments} as c, {users} AS u1, {users} AS u2) ON (c.nid=h.nid AND c.cid=h.cid AND c.uid=u1.uid AND h.uid=u2.uid)';
  $query .= tablesort_sql($form['header']['#value']);
  $result = pager_query($query, HIDDEN_PAGER_LIMIT);

  // build a table listing the appropriate nodes
  while ($comment = db_fetch_object($result)) {
    $comments[$comment->cid] = '';
    $form['title'][$comment->cid] = array('#value' => l($comment->title, "node/$comment->nid", NULL, NULL, "comment-$comment->cid"));
    $form['name'][$comment->cid] = array('#value' =>  theme('username', $comment));
    $form['reason'][$comment->cid] = array('#value' => check_plain($comment->reason));
    $form['hider'][$comment->cid] = array('#value' => theme('username', (object)array('name' => $comment->hider, 'uid' => $comment->huid)));
    if ($comment->delay != 0) {
      if ($comment->delay < time() - 29) { // under half a minute for rounding in remaining section
        $form['date'][$comment->cid] = array('#value' => t('next cron run'));
      }
      else {
        $remaining = $comment->delay - time();
        $hours = floor($remaining / 3600);
        $remaining -= $hours * 3600;
        $minutes = round($remaining / 60);
        $time = ($hours!=0) ? $hours . format_plural($hours, ' hour ', ' hours ') : '';
        $time .= ($minutes!=0) ? $minutes . format_plural($minutes, ' minute', ' minutes') : '';
        $form['date'][$comment->cid] = array('#value' => t('after %time', array('%time' => $time)));
      }
    }
    else {
      $form['date'][$comment->cid] = array('#value' => format_date($comment->date, 'small'));
    }
    $form['operations'][$comment->cid] = array('#value' => l(t('Edit'), "comment/edit/$comment->cid", array('query' => 'destination=admin/content/comment/')));
  }

  $form['comments'] = array('#type' => 'checkboxes', '#options' => $comments);
  $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));

  return $form;
}

/**
 * Implentation of form API hook; validate hidden_list_admin_comments() form.
 */
function hidden_list_admin_comments_validate($form, &$form_state) {
  $form_state['values']['comments'] = array_diff($form_state['values']['comments'], array(0));
  if (count($form_state['values']['comments']) == 0) {
    form_set_error('', t('Please select one or more comments to perform the update on.'));
    return 'admin/content/comment/list/hidden';
  }
}

/**
 * Implementation of form API hook; submit hidden_list_admin_comments() form.
 */
function hidden_list_admin_comments_submit($form, &$form_state) {
  if ($form_state['values']['operation'] == 'UNHIDE') {
    $unhide = array();
    $error = array();
    foreach ($form_state['values']['comments'] as $cid => $value) {
      $cid = (int)$cid;
      if ($value) {
        if (_hidden_hidden_unhide('comment', $cid)) {
          $unhide[] = $cid;
        }
        else {
          $error[] = $cid;
        }
      }
    }
    if ($n = count($error)) {
      $msg = format_plural($n, 'Error unhiding comment.', 'Error unhiding comments.');
      _hidden_log(HIDDEN_LOG_ERROR, $msg, 'comment', $error);
      drupal_set_message($msg, 'error');
    }
    if ($n = count($unhide)) {
      $msg = format_plural($n, 'Unhidden comment.', 'Unhidden comments.');
      _hidden_log(HIDDEN_LOG_UNHIDE, $msg, 'comment', $unhide);
      drupal_set_message($msg);
    }
  }
  $form_state['redirect'] = 'admin/content/comment/list/hidden';
}

/**
 * Menu callback; admin list form of reported comments.
 *
 * @ingroup forms
 * @see hidden_list_admin_comments_reported_validate()
 * @see hidden_list_admin_comments_reported_submit()
 * @see theme_hidden_list_admin_comments_reported()
 */
function hidden_list_admin_comments_reported(&$form_state) {
  $form = _hidden_list_admin_reported_options();
  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      NULL,
      array('data' => t('title'), 'field' => 'title'),
      array('data' => t('author'), 'field' => 'name'),
      array('data' => t('reason'), 'field' => 'reason'),
      array('data' => t('reported by'), 'field' => 'reporter'),
      array('data' => t('reported on'), 'field' => 'date', 'sort' => 'desc'),
      array('data' => t('seen by'), 'field' => 'seen'),
      array('data' => t('status')),
    )
  );

  $query = 'SELECT c.nid AS nid, c.cid AS cid, c.subject AS title, c.timestamp AS changed, c.status AS published,'.
          ' hr.created AS date,'.
          ' r.title AS reason,'.
          ' u1.uid AS uid, u1.name AS name,'.
          ' u2.uid AS ruid, u2.name AS reporter,'.
          ' u3.uid AS suid, u3.name AS seen,'.
          ' h.rid AS hidden'.
          ' FROM {hidden_reported_comment} AS hr'.
          ' LEFT JOIN {hidden_reasons} AS r ON hr.rid=r.rid'.
          ' LEFT JOIN {users} AS u3 ON hr.suid=u3.uid'.
          ' LEFT JOIN {hidden_comment} AS h ON (hr.nid=h.nid AND hr.cid=h.cid)'.
          ' INNER JOIN ({comments} AS c, {users} AS u1, {users} AS u2) ON (c.nid=hr.nid AND c.cid=hr.cid AND c.uid=u1.uid AND hr.uid=u2.uid)';

  $query .= tablesort_sql($form['header']['#value']);
  $result = pager_query($query, HIDDEN_PAGER_LIMIT);

  // build a table listing the appropriate nodes
  while ($comment = db_fetch_object($result)) {
    if ($comment->published==COMMENT_PUBLISHED) {
      $status = t('published');
    }
    elseif ($comment->hidden != NULL) {
      $status = t('hidden');
    }
    else {
      $status = t('not published');
    }
    $comments[$comment->cid] = '';
    $form['title'][$comment->cid] = array('#value' => l($comment->title, "node/$comment->nid", NULL, NULL, "comment-$comment->cid"));
    $form['name'][$comment->cid] = array('#value' => theme('username', $comment));
    $form['reason'][$comment->cid] = array('#value' => check_plain($comment->reason));
    $form['reporter'][$comment->cid] = array('#value' => theme('username', (object)array('name' => $comment->reporter, 'uid' => $comment->ruid)));
    $form['date'][$comment->cid] = array('#value' => format_date($comment->date, 'small'));
    $form['seen'][$comment->cid] = array('#value' => ($comment->suid != 0) ? theme('username', (object)array('name' => $comment->seen, 'uid' => $comment->suid)) : '');
    $form['status'][$comment->cid] = array('#value' => $status);
  }

  $form['comments'] = array('#type' => 'checkboxes', '#options' => $comments);
  $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));

  return $form;
}

/**
 * Implentation of form API hook; validate hidden_list_admin_comments_reported() form.
 */
function hidden_list_admin_comments_reported_validate($form, &$form_state) {
  $form_state['values']['comments'] = array_diff($form_state['values']['comments'], array(0));
  if (count($form_state['values']['comments']) == 0) {
    form_set_error('', t('Please select one or more comments to perform the update on.'));
  }
}

/**
 * Implementation of form API hook; submit hidden_list_admin_comments_reported() form.
 */
function hidden_list_admin_comments_reported_submit($form, &$form_state) {
  global $user;
  switch ($form_state['values']['operation']) {
    case 'HIDE':
      $hide = array();
      $error = array();
      foreach ($form_state['values']['comments'] as $cid => $value) {
        $cid = (int)$cid;
        if ($value) {
          // could transfer nid with form, but is there an elegant way?
          $result = db_query('SELECT nid FROM {comments} WHERE cid=%d', $cid);
          $nid = db_result($result);
          if ($nid>0) {
            // would be nice to transfer reported reason(s) but there maybe more than one
            if (hidden_hidden_hide('comment', (int)$cid, $user->uid, 1)) {
              $hide[] = $cid;
            }
            else {
              $error[] = $cid;
            }
          }
        }
      }
      if ($n = count($error)) {
        $msg = format_plural($n, 'Error hiding reported comment', 'Error hiding reported comments');
        _hidden_log(HIDDEN_LOG_ERROR, $msg, 'comment', $error);
        drupal_set_message($msg, 'error');
      }
      if ($n = count($hide)) {
        $msg = format_plural($n, 'Hidden reported comment', 'Hidden reported comments');
        _hidden_log(HIDDEN_LOG_HIDE, $msg, 'comment', $hide, 1);
        drupal_set_message($msg);
      }
      break;
    case 'SPAM':
      if (! module_exists('spam')) {
        break;
      }
      $spam = array();
      foreach ($form_state['values']['comments'] as $cid => $value) {
        $cid = (int)$cid;
        if ($value) {
          $result = db_query('SELECT nid FROM {comments} WHERE cid=%d', $cid);
          $nid = db_result($result);
          if ($nid>0) {
            spam_spam_comment($cid);
            $spam[] = $cid;
          }
        }
      }
      if ($n = count($spam)) {
        $msg = format_plural($n, 'Marked reported comment as spam.', 'Marked reported comments as spam.');
        _hidden_log(HIDDEN_LOG_SPAM, $msg, 'comment', $cid);
        drupal_set_message($msg);
      }
    case 'SEEN':
      $seen = array();
      $error = array();
      foreach ($form_state['values']['comments'] as $cid => $value) {
        $cid = (int)$cid;
        if ($value) {
          $result = db_query('SELECT nid FROM {comments} WHERE cid=%d', $cid);
          $nid = db_result($result);
          if ($nid>0) {
            if (_hidden_reported_seen('comment', $cid, $user->uid)) {
              $seen[] = $cid;
            }
            else {
              $error[] = $cid;
            }
          }
        }
      }
      if ($n = count($error)) {
        $msg = format_plural($n, 'Error marking reported comment as seen', 'Error marking reported comments as seen');
        _hidden_log(HIDDEN_LOG_ERROR, $msg, 'comment', $error);
        drupal_set_message($msg, 'error');
      }
      if ($n = count($seen)) {
        $msg = format_plural($n, 'Reported comment marked as seen', 'Reported comments marked as seen');
        _hidden_log(HIDDEN_LOG_SEEN, $msg, 'comment', $seen);
        drupal_set_message($msg);
      }
      break;
    }
}

/**
 * Menu callback; Admin list form of hidden nodes.
 *
 * @ingroup forms
 * @see hidden_list_admin_nodes_validate()
 * @see hidden_list_admin_nodes_submit()
 * @see theme_hidden_list_admin_nodes()
 */
function hidden_list_admin_nodes(&$form_state) {
  $form = _hidden_list_admin_options();
  $form['header'] = array(
      '#type' => 'value',
      '#value' => array(
        NULL,
        array('data' => t('title'), 'field' => 'title'),
        array('data' => t('type'), 'field' => 'type'),
        array('data' => t('author'), 'field' => 'name'),
        array('data' => t('reason'), 'field' => 'reason'),
        array('data' => t('hidden by'), 'field' => 'hider'),
        array('data' => t('hidden'), 'field' => 'date', 'sort' => 'desc'),
        NULL,
        )
      );

  $query = 'SELECT n.nid AS nid, n.title AS title, n.type AS type, n.changed AS changed,'.
    ' h.created AS date, h.delay AS delay,'.
    ' r.title AS reason,'.
    ' u1.uid AS uid, u1.name AS name,'.
    ' u2.uid AS huid, u2.name AS hider'.
    ' FROM {hidden_node} AS h'.
    ' LEFT JOIN {hidden_reasons} AS r ON h.rid=r.rid'.
    ' INNER JOIN ({node} AS n, {users} AS u1, {users} AS u2) ON (n.nid=h.nid AND n.uid=u1.uid AND h.uid=u2.uid)';

  $query .= tablesort_sql($form['header']['#value']);
  $result = pager_query($query, HIDDEN_PAGER_LIMIT);

  // build a table listing the appropriate nodes
  while ($node = db_fetch_object($result)) {
    $nodes[$node->nid] = '';
    $form['title'][$node->nid] = array('#value' => l($node->title, "node/$node->nid") .' '. (node_last_viewed($node->nid) < $node->changed ? theme_mark() : ''));
    $form['type'][$node->nid] = array('#value' => node_get_types('name', $node));
    $form['name'][$node->nid] = array('#value' =>  theme('username', $node));
    $form['reason'][$node->nid] = array('#value' => check_plain($node->reason));
    $form['hider'][$node->nid] = array('#value' => theme('username', (object)array('name' => $node->hider, 'uid' => $node->huid)));
    if ($node->delay != 0) {
      if ($node->delay < time() - 29) { // under half a minute for rounding in remaining section
        $form['date'][$node->nid] = array('#value' => t('next cron run'));
      }
      else {
        $remaining = $node->delay - time();
        $hours = floor($remaining / 3600);
        $remaining -= $hours * 3600;
        $minutes = round($remaining / 60);
        $time = ($hours!=0) ? $hours . format_plural($hours, ' hour ', ' hours ') : '';
        $time .= ($minutes!=0) ? $minutes . format_plural($minutes, ' minute', ' minutes') : '';
        $form['date'][$node->nid] = array('#value' => t('after %time', array('%time' => $time)));
      }
    }
    else {
      $form['date'][$node->nid] = array('#value' => format_date($node->date, 'small'));
    }
    $form['operations'][$node->nid] = array('#value' => l(t('Edit'), "node/$node->nid/edit"));
  }

  $form['nodes'] = array('#type' => 'checkboxes', '#options' => $nodes);
  $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));

  return $form;
}

/**
 * Implentation of form API hook; validate hidden_list_admin_nodes() form.
 */
function hidden_list_admin_nodes_validate($form, &$form_state) {
  $form_state['values']['nodes'] = array_diff($form_state['values']['nodes'], array(0));
  if (count($form_state['values']['nodes']) == 0) {
    form_set_error('', t('Please select one or more nodes to perform the update on.'));
  }
}

/**
 * Implementation of form API hook; submit hidden_list_admin_nodes() form.
 */
function hidden_list_admin_nodes_submit($form, &$form_state) {
  $nids = array();
  foreach ($form_state['values']['nodes'] as $nid => $value) {
    if ($value) {
      $nids[] = (int)$nid;
    }
  }
  hidden_operations_unhide($nids);
}

/**
 * Menu callback; Admin list form of reported nodes.
 *
 * @ingroup forms
 * @see hidden_list_admin_nodes_reported_validate()
 * @see hidden_list_admin_nodes_reported_submit()
 * @see theme_hidden_list_admin_nodes_reported()
 */
function hidden_list_admin_nodes_reported(&$form_state) {
  $form = _hidden_list_admin_reported_options();
  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      NULL,
      array('data' => t('title'), 'field' => 'title'),
      array('data' => t('type'), 'field' => 'type'),
      array('data' => t('author'), 'field' => 'name'),
      array('data' => t('reason'), 'field' => 'reason'),
      array('data' => t('reported by'), 'field' => 'reporter'),
      array('data' => t('reported on'), 'field' => 'date', 'sort' => 'desc'),
      array('data' => t('seen by'), 'field' => 'seen'),
      array('data' => t('status')),
    )
  );

  $query = 'SELECT n.nid AS nid, n.title AS title, n.type AS type, n.changed AS changed, n.status AS published,'.
          ' hr.created AS date,'.
          ' r.title AS reason,'.
          ' u1.uid AS uid, u1.name AS name,'.
          ' u2.uid AS ruid, u2.name AS reporter,'.
          ' u3.uid AS suid, u3.name AS seen,'.
          ' h.rid AS hidden'.
          ' FROM {hidden_reported_node} AS hr'.
          ' LEFT JOIN {hidden_reasons} AS r ON hr.rid=r.rid'.
          ' LEFT JOIN {users} AS u3 ON hr.suid=u3.uid'.
          ' LEFT JOIN {hidden_node} AS h ON (hr.nid=h.nid)'.
          ' INNER JOIN ({node} AS n, {users} AS u1, {users} AS u2) ON (n.nid=hr.nid AND n.uid=u1.uid AND hr.uid=u2.uid)';

  $query .= tablesort_sql($form['header']['#value']);
  $result = pager_query($query, HIDDEN_PAGER_LIMIT);

  // build a table listing the appropriate nodes
  while ($node = db_fetch_object($result)) {
    if ($node->published) {
      $status = t('published');
    }
    elseif ($node->hidden != NULL) {
      $status = t('hidden');
    }
    else {
      $status = t('not published');
    }
    $nodes[$node->nid] = '';
    $form['title'][$node->nid] = array('#value' => l($node->title, "node/$node->nid") .' '. (node_last_viewed($node->nid) < $node->changed ? theme_mark() : ''));
    $form['type'][$node->nid] = array('#value' => node_get_types('name', $node));
    $form['name'][$node->nid] = array('#value' =>  theme('username', $node));
    $form['reason'][$node->nid] = array('#value' => check_plain($node->reason));
    $form['reporter'][$node->nid] = array('#value' => theme('username', (object)array('name' => $node->reporter, 'uid' => $node->ruid)));
    $form['date'][$node->nid] = array('#value' => format_date($node->date, 'small'));
    $form['seen'][$node->nid] = array('#value' => ($node->suid != 0) ? theme('username', (object)array('name' => $node->seen, 'uid' => $node->suid)) : '');
    $form['status'][$node->nid] = array('#value' => $status);
  }

  $form['nodes'] = array('#type' => 'checkboxes', '#options' => $nodes);
  $form['pager'] = array('#value' => theme('pager', NULL, 50, 0));

  return $form;
}

/**
 * Implentation of form API hook; validate hidden_list_admin_nodes_reported() form.
 */
function hidden_list_admin_nodes_reported_validate($form, &$form_state) {
  $form_state['values']['nodes'] = array_diff($form_state['values']['nodes'], array(0));
  if (count($form_state['values']['nodes']) == 0) {
    form_set_error('', t('Please select one or more nodes to perform the update on.'));
  }
}

/**
 * Implementation of form API hook; submit hidden_list_admin_nodes_reported() form.
 */
function hidden_list_admin_nodes_reported_submit($form, &$form_state) {
  global $user;
  switch ($form_state['values']['operation']) {
    case 'HIDE':
      $hide = array();
      $error = array();
      foreach ($form_state['values']['nodes'] as $nid => $value) {
        $nid = (int)$nid;
        if ($value) {
          // would be nice to transfer reported reason(s) but there maybe more than one
          if (hidden_hidden_hide('node', (int)$nid, $user->uid, 1)) {
            $hide[] = $nid;
          }
          else {
            $error[] = $nid;
          }
        }
      }
      if ($n = count($error)) {
        $msg = format_plural($n, 'Error hiding reported node.', 'Error hiding reported nodes.');
        _hidden_log(HIDDEN_LOG_ERROR, $msg, 'node', $error);
        _drupal_set_message($msg, 'error');
      }
      if ($n = count($hide)) {
        $msg = format_plural($n, 'Hidden reported node.', 'Hidden reported nodes.');
        _hidden_log(HIDDEN_LOG_HIDE, $msg, 'node', $hide, 1);
        drupal_set_message($msg);
      }
      break;
    case 'SPAM':
      if (! module_exists('spam')) {
        break;
      }
      $spam = array();
      foreach ($form_state['values']['nodes'] as $nid => $value) {
        $nid = (int)$nid;
        spam_spam_node($nid);
        $spam[] = $nid;
      }
      if ($n = count($spam)) {
        $msg = format_plural($n, 'Reported node marked as spam.', 'Reported nodes marked as spam.');
        _hidden_log(HIDDEN_LOG_SPAM, $msg, 'node', $spam);
        drupal_set_message($msg);
      }
    case 'SEEN':
      $error = array();
      $seen = array();
      foreach ($form_state['values']['nodes'] as $nid => $value) {
        $nid = (int)$nid;
        if ($value) {
          if (_hidden_reported_seen('node', (int)$nid, $user->uid)) {
            $seen[] = $nid;
          }
          else {
            $error[] = $nid;
          }
        }
      }
      if ($n = count($error)) {
        $msg = format_plural($n, 'Error marking reported node as seen.', 'Error marking reported nodes as seen.');
        _hidden_log(HIDDEN_LOG_ERROR, $msg, 'node', $error);
        drupal_set_message($msg, 'error');
      }
      if ($n = count($seen)) {
        $msg = format_plural($n, 'Marked reported node as seen.', 'Marked reported nodes as seen');
        _hidden_log(HIDDEN_LOG_SEEN, $msg, 'node', $seen);
        drupal_set_message($msg);
      }
      break;
    }
}
/**
 * Format admin hidden comment list form.
 *
 * @ingroup themeable.
 */
function theme_hidden_list_admin_comments($form) {
  $output = drupal_render($form['options']);

  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      $row[] = drupal_render($form['comments'][$key]);
      $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['name'][$key]);
      $row[] = drupal_render($form['reason'][$key]);
      $row[] = drupal_render($form['hider'][$key]);
      $row[] = drupal_render($form['date'][$key]);
      $row[] = drupal_render($form['operations'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('No comments available.'), 'colspan' => '6'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}

/**
 * Format admin reported node list form hidden_list_admin_comments_reported().
 *
 * @ingroup themeable.
 */
function theme_hidden_list_admin_comments_reported($form) {
  $output = drupal_render($form['options']);

  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      $row[] = drupal_render($form['comments'][$key]);
      $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['name'][$key]);
      $row[] = drupal_render($form['reason'][$key]);
      $row[] = drupal_render($form['reporter'][$key]);
      $row[] = drupal_render($form['date'][$key]);
      $row[] = drupal_render($form['seen'][$key]);
      $row[] = drupal_render($form['status'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('No comments available.'), 'colspan' => '8'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);
  return $output;
}

/**
 * Formats admin hidden node list form.
 *
 * @ingroup themeable.
 */
function theme_hidden_list_admin_nodes($form) {
  $output = drupal_render($form['options']);

  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      $row[] = drupal_render($form['nodes'][$key]);
      $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['type'][$key]);
      $row[] = drupal_render($form['name'][$key]);
      $row[] = drupal_render($form['reason'][$key]);
      $row[] = drupal_render($form['hider'][$key]);
      $row[] = drupal_render($form['date'][$key]);
      $row[] = drupal_render($form['operations'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('No nodes available.'), 'colspan' => '7'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);

  return $output;
}

/**
 * Formats admin reported node list form hidden_list_admin_nodes_reported().
 *
 * @ingroup themeable.
 */
function theme_hidden_list_admin_nodes_reported($form) {
  $output = drupal_render($form['options']);

  if (isset($form['title']) && is_array($form['title'])) {
    foreach (element_children($form['title']) as $key) {
      $row = array();
      $row[] = drupal_render($form['nodes'][$key]);
      $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['type'][$key]);
      $row[] = drupal_render($form['name'][$key]);
      $row[] = drupal_render($form['reason'][$key]);
      $row[] = drupal_render($form['reporter'][$key]);
      $row[] = drupal_render($form['date'][$key]);
      $row[] = drupal_render($form['seen'][$key]);
      $row[] = drupal_render($form['status'][$key]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('No nodes available.'), 'colspan' => '7'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);
  return $output;
}
