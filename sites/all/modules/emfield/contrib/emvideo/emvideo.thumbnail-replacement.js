// $Id: emvideo.thumbnail-replacement.js,v 1.1.2.1 2009/02/26 03:35:54 aaron Exp $

/**
 *  @file
 *  This will use jQuery AJAX to replace a thumbnail with its video version.
 *
 *  Drupal.settings.emvideo.thumbs will contain an array with the following:
 *
 */

if (Drupal.jsEnabled) {
  $(document).ready(function(){
    if (Drupal.settings.emvideo.thumbnail_overlay) {
      $(".emvideo-thumbnail-replacement").prepend("<span></span>");
    }
    $('.emvideo-thumbnail-replacement').click(function() {
      // 'this' won't point to the element when it's inside the ajax closures,
      // so we reference it using a variable.
      var element = this;
      $.ajax({
        url: element.href,
        dataType: 'html',
        success: function (data) {
          if (data) {
            // Success.
            $(element).parent().html(data);
          }
          else {
            // Failure.
            alert('An unknown error occurred.\n'+ element.href);
          }
        },
        error: function (xmlhttp) {
          alert('An HTTP error '+ xmlhttp.status +' occurred.\n'+ element.href);
        }
      });
      return false;
    });
  });
}
