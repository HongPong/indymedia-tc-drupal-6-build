<?php
// $Id: image_captcha.admin.inc,v 1.21 2009/08/27 21:52:27 soxofaan Exp $

/**
 * @file
 * Functions for administration/settings interface.
 */

/**
 * function to get a list of available fonts
 */
function _image_captcha_available_fonts() {
  $available_fonts = array('BUILTIN' => t('Built-in font'));
  $fontsdirectories = array(
    drupal_get_path('module', 'image_captcha') .'/fonts',
    file_directory_path(),
  );
  foreach ($fontsdirectories as $fontsdirectory) {
    foreach (file_scan_directory($fontsdirectory, '\.[tT][tT][fF]$') as $filename => $font) {
      $available_fonts[$filename] = "{$font->basename} ($filename)";
    }
  }
  return $available_fonts;
}

/**
 * Configuration form for image_captcha
 */
function image_captcha_settings_form() {

  // Use javascript for some added usability on admin form.
  drupal_add_js(drupal_get_path('module', 'image_captcha') .'/image_captcha.js');

  $form = array();

  // First some error checking.
  $image_captcha_setup_errno = _image_captcha_check_setup();
  if ($image_captcha_setup_errno & IMAGE_CAPTCHA_ERROR_NO_GDLIB) {
    drupal_set_message(t(
      'The Image CAPTCHA module can not generate images because your PHP setup does not support it (no <a href="!gdlib">GD library</a>).',
      array('!gdlib' => 'http://php.net/manual/en/book.image.php')
    ), 'error');
    // It is no use to continue building the rest of the settings form.
    return $form;
  }

  $form['image_captcha_example'] = array(
    '#type' => 'fieldset',
    '#title' => t('Example'),
    '#description' => t('Presolved image CAPTCHA example, generated with the current settings.'),
  );
  $form['image_captcha_example']['image'] = array(
    '#type' => 'captcha',
    '#captcha_type' => 'image_captcha/Image',
    '#captcha_admin_mode' => TRUE,
  );

  // General code settings.
  $form['image_captcha_code_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Code settings'),
  );
  $form['image_captcha_code_settings']['image_captcha_image_allowed_chars'] = array(
    '#type' => 'textfield',
    '#title' => t('Characters to use in the code'),
    '#default_value' => variable_get('image_captcha_image_allowed_chars', IMAGE_CAPTCHA_ALLOWED_CHARACTERS),
  );
  $form['image_captcha_code_settings']['image_captcha_code_length'] = array(
    '#type' => 'select',
    '#title' => t('Code length'),
    '#options' => array(2 => 2, 3, 4, 5, 6, 7, 8, 9, 10),
    '#default_value' => (int) variable_get('image_captcha_code_length', 5),
    '#description' => t('The code length influences the size of the image. Note that larger values make the image generation more CPU intensive.'),
  );

  // font related stuff
  $form['image_captcha_font_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Font settings'),
  );
  $available_fonts = _image_captcha_available_fonts();
  list($default_font, $errno) = _image_captcha_get_font();
  $form['image_captcha_font_settings']['image_captcha_font'] = array(
    '#type' => 'select',
    '#title' => t('Font'),
    '#default_value' => $default_font,
    '#description' => t('The TrueType font (.ttf) to use for the text in the image CAPTCHA.'),
    '#options' => $available_fonts,
  );
  // add a prerender procedure for checking that a font should be set.
  $form['#pre_render'] = array('image_captcha_settings_form_pre_render');
  // font size
  if ($default_font != 'BUILTIN') {
    $form['image_captcha_font_settings']['image_captcha_font_size'] = array(
      '#type' => 'select',
      '#title' => t('Font size'),
      '#options' => array(
        9 => '9 pt - ' . t('tiny'),
        12 => '12 pt - ' . t('small'),
        18 => '18 pt',
        24 => '24 pt - ' . t('normal'),
        30 => '30 pt',
        36 => '36 pt - ' . t('large'),
        48 => '48 pt',
        64 => '64 pt - ' . t('extra large'),
      ),
      '#default_value' => (int) variable_get('image_captcha_font_size', 30),
      '#description' => t('The font size influences the size of the image. Note that larger values make the image generation more CPU intensive.'),
    );
  }
  // Disable some options when there is no TTF support
  if ($image_captcha_setup_errno & IMAGE_CAPTCHA_ERROR_NO_TTF_SUPPORT) {
    $form['image_captcha_font_settings']['image_captcha_font']['#disabled'] = TRUE;
    $form['image_captcha_font_settings']['image_captcha_font']['#default_value'] = 'BUILTIN';
  }

  // character spacing
  $form['image_captcha_font_settings']['image_captcha_character_spacing'] = array(
    '#type' => 'select',
    '#title' => t('Character spacing'),
    '#description' => t('Define the average spacing between characters. Note that larger values make the image generation more CPU intensive.'),
    '#default_value' => variable_get('image_captcha_character_spacing', '1.2'),
    '#options' => array(
      '0.75' => t('tight'),
      '1' => t('normal'),
      '1.2' => t('wide'),
      '1.5' => t('extra wide'),
    ),
  );

    // color settings
  $form['image_captcha_color_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Color settings'),
    '#description' => t('Configuration of the background and text colors in the image CAPTCHA.'),
  );
  $form['image_captcha_color_settings']['image_captcha_background_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Background color'),
    '#description' => t('Enter the hexadecimal code for the background color (e.g. #FFF or #FFCE90).'),
    '#default_value' => variable_get('image_captcha_background_color', '#ffffff'),
    '#maxlength' => 7,
    '#size' => 8,
  );
  $form['image_captcha_color_settings']['image_captcha_foreground_color'] = array(
    '#type' => 'textfield',
    '#title' => t('Text color'),
    '#description' => t('Enter the hexadecimal code for the text color (e.g. #000 or #004283).'),
    '#default_value' => variable_get('image_captcha_foreground_color', '#000000'),
    '#maxlength' => 7,
    '#size' => 8,
  );
  $form['image_captcha_color_settings']['image_captcha_foreground_color_randomness'] = array(
    '#type' => 'select',
    '#title' => t('Additional variation of text color'),
    '#options' => array(
      0 => t('none'),
      50 => t('small'),
      100 => t('moderate'),
      150 => t('high'),
      200 => t('very high'),
    ),
    '#default_value' => (int) variable_get('image_captcha_foreground_color_randomness', 100),
    '#description' => t('The different characters will have randomized colors in the specified range around the text color.'),
  );


  // distortion and noise settings
  $form['image_captcha_distortion_and_noise'] = array(
    '#type' => 'fieldset',
    '#title' => t('Distortion and noise'),
    '#description' => t('With these settings you can control the degree of obfuscation by distortion and added noise. Do not exaggerate the obfuscation and assure that the code in the image is reasonably readable. For example, do not combine high levels of distortion and noise.'),
  );
  // distortion
  $form['image_captcha_distortion_and_noise']['image_captcha_distortion_amplitude'] = array(
    '#type' => 'select',
    '#title' => t('Distortion level'),
    '#options' => array(
      0 => '0 - '. t('none'),
      1 => '1',
      2 => '2 - '. t('low'),
      3 => '3',
      4 => '4 - '. t('moderate'),
      5 => '5',
      6 => '6 - '. t('normal'),
      7 => '7',
      8 => '8 - '. t('high'),
      9 => '9',
      10 => '10 - '. t('severe'),
    ),
    '#default_value' => (int) variable_get('image_captcha_distortion_amplitude', 6),
    '#description' => t('Set the degree of wave distortion in the image.'),
  );
  $form['image_captcha_distortion_and_noise']['image_captcha_bilinear_interpolation'] = array(
    '#type' => 'checkbox',
    '#title' => t('Smooth distortion'),
    '#default_value' => variable_get('image_captcha_bilinear_interpolation', FALSE),
    '#description' => t('This option enables bilinear interpolation of the distortion which makes the image look smoother, but it is more CPU intensive.'),
  );
  // noise
  $form['image_captcha_distortion_and_noise']['image_captcha_dot_noise'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add salt and pepper noise'),
    '#default_value' => variable_get('image_captcha_dot_noise', 0),
    '#description' => t('This option adds randomly colored point noise.'),
  );
  $form['image_captcha_distortion_and_noise']['image_captcha_line_noise'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add line noise'),
    '#default_value' => variable_get('image_captcha_line_noise', 0),
    '#description' => t('This option enables lines randomly drawn on top of the text code.'),
  );
  $form['image_captcha_distortion_and_noise']['image_captcha_noise_level'] = array(
    '#type' => 'select',
    '#title' => t('Noise level'),
    '#options' => array(
      1 => '1 - '. t('low'),
      2 => '2',
      3 => '3 - '. t('medium'),
      4 => '4',
      5 => '5 - '. t('high'),
      7 => '7',
      10 => '10 - '. t('severe'),
    ),
    '#default_value' => (int) variable_get('image_captcha_noise_level', 5),
  );
  $form['#validate'] = array('image_captcha_settings_form_validate');
  return system_settings_form($form);
}

/**
 * Pre render function for image_captcha_settings_form for providing
 * extra info about TTF fonts.
 */
function image_captcha_settings_form_pre_render($form) {
  $errno = _image_captcha_check_setup();

  if ($errno & IMAGE_CAPTCHA_ERROR_NO_TTF_SUPPORT) {
    drupal_set_message(
      t('The Image CAPTCHA module can not use True Type fonts because your PHP setup does not support it. There is only a built-in bitmap font available.'),
      'warning'
    );
  }
  else if (count($form['image_captcha_font_settings']['image_captcha_font']['#options']) == 1) {
    drupal_set_message(t('The image CAPTCHA module works best with a TrueType font file (TTF) for generating the images, but because of licencing issues it is not allowed to package fonts with the module. A simple built-in bitmap font is provided as default instead. You can install TrueType fonts yourself by putting them in the fonts directory of the image CAPTCHA module (directory "%fontsdir") or by uploading them to your Drupal file system (directory "%filesdir") with for example the upload module.', array('%fontsdir' => drupal_get_path('module', 'image_captcha') .'/fonts', '%filesdir' => file_directory_path())), 'warning');
  }
  if ($form['image_captcha_font_settings']['image_captcha_font']['#default_value'] == 'BUILTIN') {
    drupal_set_message(t('The usage of the built-in bitmap font it is not recommended because of its small size and missing UTF-8 support.'), 'warning');
  }
  return $form;
}

/**
 * Validation function for image_captcha configuration form
 */
function image_captcha_settings_form_validate($form, &$form_state) {
  // Check image_captcha_image_allowed_chars for spaces.
  if (preg_match('/\s/', $form_state['values']['image_captcha_image_allowed_chars'])) {
    form_set_error('image_captcha_image_allowed_chars', t('The list of characters to use should not contain spaces.'));
  }
  // Check font.
  $font = $form_state['values']['image_captcha_font'];
  if ($font == '0') {
    form_set_error('image_captcha_font', t('You need to select a font'));
  }
  elseif ($font == 'BUILTIN') {
    // With the built in font, only latin2 characters should be used.
    if (preg_match('/[^a-zA-Z0-9]/', $form_state['values']['image_captcha_image_allowed_chars'])) {
      form_set_error('image_captcha_image_allowed_chars', t('The built-in font only supports Latin2 characters. Only use "a" to "z" and numbers.'));
    }
  }
  elseif ($font != 'BUILTIN' && (!is_file($font) || !is_readable($font))) {
    form_set_error('image_captcha_font', t('Font does not exist or is not readable.'));
  }

  // check color settings
  if (!preg_match('/^#([0-9a-fA-F]{3}){1,2}$/', $form_state['values']['image_captcha_background_color'])) {
    form_set_error('image_captcha_background_color', t('Background color is not a valid hexadecimal color value.'));
  }
  if (!preg_match('/^#([0-9a-fA-F]{3}){1,2}$/', $form_state['values']['image_captcha_foreground_color'])) {
    form_set_error('image_captcha_foreground_color', t('Text color is not a valid hexadecimal color value.'));
  }
}
