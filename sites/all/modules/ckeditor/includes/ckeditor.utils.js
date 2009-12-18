// $Id: ckeditor.utils.js,v 1.1 2009/12/04 20:36:57 wwalc Exp $
Drupal.ckeditor = (typeof(CKEDITOR) != 'undefined');

// this object will store teaser information
Drupal.ckeditorTeaser = {
  lookup: {},
  lookupSetup: false,
  cache: {}
};

Drupal.ckeditorToggle = function(textarea_id, TextTextarea, TextRTE, xss_check){
  if (!CKEDITOR.env.isCompatible) {
    return;
  }
  if (typeof(CKEDITOR.instances) != 'undefined' && typeof(CKEDITOR.instances[textarea_id]) != 'undefined') {
    Drupal.ckeditorOff(textarea_id);
    $('#switch_' + textarea_id).text(TextRTE);
  }
  else {
    Drupal.ckeditorOn(textarea_id);
    $('#switch_' + textarea_id).text(TextTextarea);
  }
};

/**
 * CKEditor starting function
 *
 * @param string textarea_id
 */
Drupal.ckeditorOn = function(textarea_id) {
  if ((typeof(Drupal.settings.ckeditor.load_timeout) == 'undefined') && (typeof(CKEDITOR.instances[textarea_id]) != 'undefined')) {
    return;
  }
  if (typeof(Drupal.settings.ckeditor.settings[textarea_id]) == 'undefined') {
    return;
  }
  if (!CKEDITOR.env.isCompatible) {
    return;
  }

  if (teaser = Drupal.ckeditorTeaserInfo(textarea_id)) {
    var tv = teaser.textarea.val();
    if (tv.length > 0) {
      $("#" + textarea_id).val(tv + '\n<!--break-->\n' + $("#" + textarea_id).val());
      teaser.textarea.val('');
    }
    teaser.buttonContainer.hide();
    teaser.textareaContainer.hide();
  }

  if (($("#" + textarea_id).val().length > 0) && ($("#" + textarea_id).attr('class').indexOf("filterxss1") != -1 || $("#" + textarea_id).attr('class').indexOf("filterxss2") != -1)) {
    $.post(Drupal.settings.basePath + 'index.php?q=ckeditor/xss', {
      text: $('#' + textarea_id).val(),
      'filters[]': Drupal.settings.ckeditor.settings[textarea_id].filters
    }, function(text){
      $("#" + textarea_id).val(text);
    });
  }

  $("#" + textarea_id).next(".grippie").css("display", "none");
  $("#" + textarea_id).addClass("ckeditor-processed");

  Drupal.settings.ckeditor.settings[textarea_id]['on'] = 
  {
    configLoaded  : function(ev)
    {
      ev.editor.addCss(ev.editor.config.extraCss);
    },
    instanceReady : function(ev)
    {
      var body = $(ev.editor.document.$.body);

      if (ev.editor.config.bodyClass)
        body.addClass(ev.editor.config.bodyClass);
      if (ev.editor.config.bodyId)
        body.attr('id', ev.editor.config.bodyId);
      if (typeof(Drupal.smileysAttach) != 'undefined')
        ev.editor.dataProcessor.writer.indentationChars = '    ';

      // IE8 - initial focus problem
      if (CKEDITOR.env.ie)
        ev.editor.insertHtml('');
    }
  };

  CKEDITOR.replace(textarea_id, Drupal.settings.ckeditor.settings[textarea_id]);
};

/**
 * CKEditor destroy function
 *
 * @param string textarea_id
 */
Drupal.ckeditorOff = function(textarea_id) {
  if (typeof(CKEDITOR.instances[textarea_id]) == 'undefined') {
    return;
  }
  if (!CKEDITOR.env.isCompatible) {
    return;
  }

  var data = CKEDITOR.instances[textarea_id].getData();
  CKEDITOR.instances[textarea_id].destroy();
  if (teaser = Drupal.ckeditorTeaserInfo(textarea_id)) {
    var brcode = /<!--break-->/;
    data = data.split(brcode);
    if (data.length > 1) {
      teaser.textareaContainer.show();
      teaser.textarea.attr('disabled', '');
      if (teaser.button.attr('value') != Drupal.t('Join summary')) {
        try {
          teaser.button.click();
        } 
        catch (e) {
          teaser.button.val(Drupal.t('Join summary'));
        }
      }
      teaser.textarea.val(data[0]);
      $("#" + textarea_id).val(data[1]);
    }
    else {
      $("#" + textarea_id).val(data[0]);
      teaser.textarea.attr('disabled', 'disabled');
      if (teaser.button.attr('value') != Drupal.t('Split summary at cursor')) {
        try {
          teaser.button.click();
        } 
        catch (e) {
          teaser.button.val(Drupal.t('Split summary at cursor'));
        }
      }
    }
    teaser.buttonContainer.show();
  }

  $("#" + textarea_id).next(".grippie").css("display", "block");
  $("#" + textarea_id).removeClass("ckeditor-processed");
};

/**
 * CKEditor popup mode function
 */
function ckeditorOpenPopup(jsID, textareaID, width){
  popupUrl = Drupal.settings.ckeditor.module_path + '/includes/ckeditor.popup.html?var=' + jsID + '&el=' + textareaID;
  
  var percentPos = width.indexOf('%');
  if (percentPos != -1) {
    width = width.substr(0, percentPos);
    width = width / 100 * screen.width;
  }
  window.open(popupUrl, null, 'width=' + width + ',toolbar=no,location=no,directories=no,status=no,menubar=no,scrollbars=no,resizable=1,dependent=yes');
  return false;
};

/**
 * Returns true if CKEDITOR.version >= version
 */
Drupal.ckeditorCompareVersion = function (version){
  var ckver = CKEDITOR.version;
  ckver = ckver.match(/(([\d]\.)+[\d]+)/i);
  version = version.match(/((\d+\.)+[\d]+)/i);
  ckver = ckver[0].split('.');
  version = version[0].split('.');
  for (var x in ckver) {
    if (ckver[x]<version[x]) {
      return false;
    }
  }
  return true;
};

/**
 * This function retrieves information about a possible teaser field associated
 * with the mentioned field.
 *
 * @param taid
 *            string HTML id of the main text area
 */
Drupal.ckeditorTeaserInfo = function(taid) {
  // if the result is cached, return it
  if (Drupal.ckeditorTeaser.cache[taid]) {
    return Drupal.ckeditorTeaser.cache[taid];
  }
  
  // build a lookup table
  if (!Drupal.ckeditorTeaser.lookupSetup) {
    Drupal.ckeditorTeaser.lookupSetup = true;
    for (var x in Drupal.settings.teaser) {
      Drupal.ckeditorTeaser.lookup[Drupal.settings.teaser[x]] = x;
    }
  }
  
  // find the elements
  if (Drupal.ckeditorTeaser.lookup[taid]) {
    var obj;
    if (window.opener) {
      obj = {
        textarea: window.opener.$('#' + Drupal.ckeditorTeaser.lookup[taid]),
        checkbox: window.opener.$('#' + Drupal.settings.teaserCheckbox[Drupal.ckeditorTeaser.lookup[taid]])
      };
    } else {
      obj = {
        textarea: $('#' + Drupal.ckeditorTeaser.lookup[taid]),
        checkbox: $('#' + Drupal.settings.teaserCheckbox[Drupal.ckeditorTeaser.lookup[taid]])
      };
    }
    
    obj.textareaContainer = obj.textarea.parent();
    obj.checkboxContainer = obj.checkbox.parent();
    
    obj.button = $('input.teaser-button', obj.checkbox.parents('div.teaser-checkbox').get(0));
    obj.buttonContainer = obj.button.parent();
    
    Drupal.ckeditorTeaser.cache[taid] = obj;
  }
  else {
    Drupal.ckeditorTeaser.cache[taid] = null;
  }
  
  return Drupal.ckeditorTeaser.cache[taid];
};

/**
 * IMCE support
 */
function ckeditor_fileUrl(file, win){
  var cfunc = win.location.href.split('&');

  for (var x in cfunc) {
    if (cfunc[x].match(/^CKEditorFuncNum=\d+$/)) {
      cfunc = cfunc[x].split('=');
      break;
    }
  }

  CKEDITOR.tools.callFunction(cfunc[1], file.url);
  win.close();
}

/**
 * Drupal behaviors
 */
Drupal.behaviors.ckeditor = function (context) {
  if ((typeof(CKEDITOR) == 'undefined') || !CKEDITOR.env.isCompatible) {
    return;
  }
  $('.ckeditor_links').show();
  // make sure the textarea behavior is run first, to get a correctly sized grippie
  // the textarea behavior requires the teaser behavior, so load that one as well
  if (Drupal.behaviors.teaser) {
    Drupal.behaviors.teaser(context);
  }
  if (Drupal.behaviors.textarea) {
    Drupal.behaviors.textarea(context);
  }

  $("textarea.ckeditor-mod:not(.ckeditor-processed)").each(function () {
    var ta_id=$(this).attr("id");
    if ((typeof(Drupal.settings.ckeditor.autostart) != 'undefined') && (typeof(Drupal.settings.ckeditor.autostart[ta_id]) != 'undefined')) {
      Drupal.ckeditorOn(ta_id);
    }
  });
};