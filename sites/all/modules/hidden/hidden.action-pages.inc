<?php

/**
 * @file
 * Hidden action pages for hiding and unhiding content.
 */

/**
 * Menu callback; form to hide a node or comment.
 *
 * @param $form_state
 * @param $type
 *   string 'node' or 'comment'
 * @param $id
 *   int nid or cid
 * @return
 *   form.
 * @ingroup forms
 * @see hidden_hide_validate()
 * @see hidden_hide_submit()
 */
function hidden_hide(&$form_state, $type, $id) {
  global $user;
  $type = (string) $type;
  $id = (int) $id;

  $hidden = (object) _hidden_hide_form_hidden_get($type, $id);
  if (! $item = _hidden_hide_form_item_get($type, $id)) {
    drupal_goto();
  }

  drupal_set_title(t('Hide @title', array('@title' => $item->title)));
  $form = _hidden_hide_form($user, $hidden);
  $form['#content_type'] = $item->type;
  $form['#content_id'] = $item->id;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Hide'),
  );

  return $form;
}

/** 
 * Form fields for hide form.
 *
 * Separated to allow embedding into the filter form as well to create the reason for
 * filter hiding.
 */
function _hidden_hide_form($user, $hidden = StdClass) {
  $form = array();

  if (user_access('administer hidden')) {
    $form['user'] = array(
      '#type' => 'textfield',
      '#title' => t('Hidden by'),
      '#maxlength' => 60,
      '#autocomplete_path' => 'user/autocomplete',
      '#default_value' => isset($hidden->name) ? check_plain($hidden->name) : check_plain($user->name),
    );
  }
  else {
    $form['user'] = array(
      '#type' => 'textfield',
      '#title' => t('Hidden by'),
      '#value' => check_plain($user->name),
      '#disabled' => TRUE,
    );
  }

  // @todo change these to rid, publicnote and privatenote as they are stored in the db
  //   alos in hidden_report_form()
  $reasons = _hidden_reasons_get_options();
  $form['reason'] = array(
    '#type' => 'radios',
    '#title' => t('Preset Reason'),
    '#default_value' => isset($hidden->rid) ? $hidden->rid : 0,
    '#options' => $reasons,
    '#description' => t('Preset reason why this item shall be hidden.'),
  );

  $form['publictext'] = array(
    '#type' => (variable_get('hidden_public_notes', TRUE)) ? 'textarea' : 'hidden',
    '#title' => t('Your Public notes'),
    '#default_value' => isset($hidden->publicnote) ? $hidden->publicnote : '',
    '#rows' => 5,
    '#maxlength' => 128,
    '#description' => t("Optional - type your reason why this item shall be hidden."),
  );

  $form['privatetext'] = array(
    '#type' => 'textarea',
    '#title' => t('Your Private notes'),
    '#default_value' => isset($hidden->privatenote) ? $hidden->privatenote : '',
    '#rows' => 5,
    '#maxlength' => 128,
    '#description' => t("Optional - notes that can only be seen by other users who can (un)hide content."),
  );

  return $form;
}
/**
 * Menu callback; form to report a node or comment.
 *
 * @param $form_state
 * @param $type
 *   string 'node' or 'comment'
 * @param $id
 *   int nid or cid
 *
 * @return
 *   form.
 * @ingroup forms
 * @see hidden_report_submit()
 */
function hidden_report(&$form_state, $type, $id) {
  if (! user_access('report for hiding')) {
    drupal_access_denied();
  }
  global $user;
  $type = (string) $type;
  $id = (int) $id;

  _hidden_hide_form_hidden_get($type, $id);
  if (! $item = _hidden_hide_form_item_get($type, $id)) {
    drupal_goto();
  }

  drupal_set_title(t('Report @title', array('@title' => $item->title)));

  $form = _hidden_hide_form($user);
  unset($form['privatetext']);
  $form['#content_type'] = $item->type;
  $form['#content_id'] = $item->id;

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Report'),
  );

  return $form;
}

/**
 * Retrieves hidden if it exists or handles error and goto.
 *
 * @param string $type 'node' or 'comment'.
 * @param int $id nid or cid
 * @return array hidden or FALSE
 */
function _hidden_hide_form_hidden_get($type, $id) {
  if (! _hidden_check_param($type, $id)) {
    // @todo this is now called from hidden_hide and hidden_report - seperate
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_hide() called with invalid arg.');
    drupal_goto('hidden');
  }
  if ($hidden = _hidden_hidden_get($type, $id)) {
    if ($hidden->delay == 0) {
      // already fully hidden
      drupal_set_message(t('Item is already hidden.'), 'error');
      drupal_goto(hidden_path($type, $id, 'view'));
    }
    // else it is hidden but it's on delay, so not yet actually hidden
  }

  return $hidden;
}

/**
 * Retrieves node or comment.
 *
 * @todo this is generic MOVE
 *
 * Moves comment->subject to title to make consistent handling of both nodes and comments.
 * Calling functions are only using the title, but for abstraction using _load commands -
 * eg. should also work with node comments then.
 *
 * @param string $type 'node' or 'comment'.
 * @param int $id nid or cid
 * @return object node or comment.
 */
function _hidden_hide_form_item_get($type, $id) {
  if ($type == 'comment') {
    if (! $item = _comment_load($id)) {
      // @todo error
      return FALSE;
    }
    $item->title = $item->subject;
  }
  else {
    if (! $item = node_load($id)) {
      // @todo error
      return FALSE;
    }
    $item->cid = 0;
  }

  $item->type = $type;
  $item->id = $id;

  return $item;
}

/**
 * Implementation of form API hook; hide content submit hidden_hide().
 */
function hidden_hide_validate($form, &$form_state) {
  if (user_access('administer hidden')) {
    // Validate the "hidden by" field.
    if (!empty($form_state['values']['user']) && !($user = user_load(array('name' => $form_state['values']['user'])))) {
      form_set_error('user', t('The username %name does not exist.', array('%name' => $form_state['values']['user'])));
    }
  }
  // @todo check nid/cid/type?
  //   probably not _needed_
  //   and note hidden_admin_filter_form_validate() uses this too
}

/**
 * Implementation of form API hook; hide or report content submit hidden_hide().
 */
function hidden_hide_submit($form, &$form_state) {
  if (! user_access('mark as hidden')) {
      drupal_access_denied();
  }

  $type = (string) $form['#content_type'];
  $id = (int) $form['#content_id'];
  if (! _hidden_check_param($type, $id)) {
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_hide_submit() passed invalid parameters.');
    drupal_goto('hidden');
  }

  if (user_access('administer hidden')) {
    $user = user_load(array('name' => $form_state['values']['user']));
    $uid = $user->uid;
  }
  else {
    global $user;
    $uid = $user->uid;
  }

  $rid = (int) $form_state['values']['reason'];
  $rid = hidden_reason_check($rid) ? $rid : 0;
  $public = (string) $form_state['values']['publictext'];
  $private = (string) $form_state['values']['privatetext'];

  $t_args = array('%type' => $type);
  if (hidden_hidden_hide($type, $id, $uid, $rid, $public, $private)) {
    $msg = t('Hidden %type.', $t_args);
    _hidden_log(HIDDEN_LOG_HIDE, $msg, $type, $id, $rid, $public);
    drupal_set_message($msg);
  }
  else {
    $msg = t('Error hiding %type.', $t_args);
    _hidden_log(HIDDEN_LOG_ERROR, $msg, $type, $id, $rid, $public);
    drupal_set_message($msg, 'error');
  }
  /**
   * The redirect challenge.
   *
   * What do users want/expect to see next?
   * What are they allowed to see?
   */
  if (! user_access('administer content')) {
    if (isset($_REQUEST['destination'])) {
      unset($_REQUEST['destination']);
    }
  }
  $form_state['redirect'] = 'hidden/'. $type .'/'. $id;
}

/**
 * Implementation of form API hook; hide or report content submit hidden_hide().
 */
function hidden_report_submit($form, &$form_state) {
  if (! user_access('report for hiding')) {
    drupal_access_denied();
  }

  $type = (string) $form['#content_type'];
  $id = (int) $form['#content_id'];
  if (! _hidden_check_param($type, $id)) {
    _hidden_log(HIDDEN_LOG_DEBUG, 'hidden_report_submit() passed invalid parameters.');
    drupal_goto('hidden');
  }

  global $user;
  $uid = $user->uid;

  $rid = (int) $form_state['values']['reason'];
  $rid = hidden_reason_check($rid) ? $rid : 0;
  $public = (string) $form_state['values']['publictext'];

  $t_args = array('%type' => $type);
  if (_hidden_reported_hide($type, $id, $uid, $rid, $public)) {
    $msg = t('Reported %type.', $t_args);
    _hidden_log(HIDDEN_LOG_REPORTED, $msg, $type, $id, $rid, $public);
    drupal_set_message($msg);
  }
  else {
    $msg = t('Error reporting %type.', $t_args);
    _hidden_log(HIDDEN_LOG_ERROR, $msg, $type, $id, $rid, $public);
    drupal_set_message($msg, 'error');
  }
}

/**
 * Menu callback; unhide a node or comment.
 *
 * @param $hidden
 *   hidden details object see hidden_hidden_check()
 */
function hidden_unhide($type, $id) {
  $t_array = array('%type' => $type);
  if (_hidden_hidden_unhide($type, $id)) {
    $msg =  t('Hidden %type unhidden', $t_array);
    _hidden_log(HIDDEN_LOG_UNHIDE, $msg, $type, $id);
    drupal_set_message($msg);
    if ($type == 'node') {
      drupal_goto('node/'. $id);
    }
    else {
      $nid = _hidden_comment_nid($id);
      drupal_goto('node/'. $nid, NULL, 'comment-'. $id);
    }
  }
  else {
    $msg = t('Error unhiding %type.', $t_array);
    _hidden_log(HIDDEN_LOG_ERROR, $msg, $type, $id);
    drupal_set_message($msg, 'error');
    drupal_goto();
  }

}
